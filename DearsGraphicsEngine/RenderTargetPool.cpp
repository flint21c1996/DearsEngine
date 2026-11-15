#include "RenderTargetPool.h"

namespace
{
DXGI_FORMAT ToDxgiFormat(RenderTargetFormat format)
{
	switch (format)
	{
	case RenderTargetFormat::Rgba16Float:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case RenderTargetFormat::Rgba8Unorm:
	default:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
}
}

void RenderTargetPool::Initialize(ID3D11Device* device, UINT defaultWidth, UINT defaultHeight)
{
	m_device = device;
	m_defaultWidth = defaultWidth;
	m_defaultHeight = defaultHeight;
}

RenderTargetHandle RenderTargetPool::Create(const RenderTargetDesc& desc)
{
	if (!m_device || desc.name.empty())
	{
		return RenderTargetHandle();
	}

	// 패스 구성 함수가 여러 번 불려도 같은 타깃이 중복 생성되지 않게 한다.
	const RenderTargetHandle existing = Find(desc.name);
	if (existing.IsValid())
	{
		return existing;
	}

	Entry entry;
	entry.description = desc;
	entry.name = desc.name;
	entry.width = desc.width != 0 ? desc.width : m_defaultWidth;
	entry.height = desc.height != 0 ? desc.height : m_defaultHeight;
	if (entry.width == 0 || entry.height == 0)
	{
		return RenderTargetHandle();
	}

	// 출력(RTV)으로 쓴 뒤 다른 패스나 ImGui가 입력(SRV)으로 읽을 수 있어야 하므로 두 용도를 모두 연다.
	D3D11_TEXTURE2D_DESC colorDesc{};
	colorDesc.Width = entry.width;
	colorDesc.Height = entry.height;
	colorDesc.MipLevels = 1;
	colorDesc.ArraySize = 1;
	colorDesc.Format = ToDxgiFormat(desc.format);
	colorDesc.SampleDesc.Count = 1;
	colorDesc.Usage = D3D11_USAGE_DEFAULT;
	colorDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	if (FAILED(m_device->CreateTexture2D(&colorDesc, nullptr, entry.colorTexture.GetAddressOf())) ||
		FAILED(m_device->CreateRenderTargetView(
			entry.colorTexture.Get(), nullptr, entry.renderTargetView.GetAddressOf())) ||
		FAILED(m_device->CreateShaderResourceView(
			entry.colorTexture.Get(), nullptr, entry.shaderResourceView.GetAddressOf())))
	{
		return RenderTargetHandle();
	}

	if (desc.withDepth)
	{
		// 전용 깊이는 이 타깃에 그릴 때만 쓰고 셰이더에서 읽지 않으므로 DSV 전용 포맷으로 만든다.
		// 외곽선 패스가 스텐실을 쓰기 때문에 메인 깊이와 같은 D24S8을 사용한다.
		D3D11_TEXTURE2D_DESC depthDesc{};
		depthDesc.Width = entry.width;
		depthDesc.Height = entry.height;
		depthDesc.MipLevels = 1;
		depthDesc.ArraySize = 1;
		depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthDesc.SampleDesc.Count = 1;
		depthDesc.Usage = D3D11_USAGE_DEFAULT;
		depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		if (FAILED(m_device->CreateTexture2D(&depthDesc, nullptr, entry.depthTexture.GetAddressOf())) ||
			FAILED(m_device->CreateDepthStencilView(
				entry.depthTexture.Get(), nullptr, entry.depthStencilView.GetAddressOf())))
		{
			return RenderTargetHandle();
		}
	}

	entry.viewport.TopLeftX = 0.0f;
	entry.viewport.TopLeftY = 0.0f;
	entry.viewport.Width = static_cast<float>(entry.width);
	entry.viewport.Height = static_cast<float>(entry.height);
	entry.viewport.MinDepth = 0.0f;
	entry.viewport.MaxDepth = 1.0f;

	m_entries.push_back(std::move(entry));

	RenderTargetHandle handle;
	handle.id = static_cast<unsigned int>(m_entries.size());
	return handle;
}

RenderTargetHandle RenderTargetPool::Find(const std::string& name) const
{
	RenderTargetHandle handle;
	for (size_t index = 0; index < m_entries.size(); ++index)
	{
		if (m_entries[index].name == name)
		{
			handle.id = static_cast<unsigned int>(index + 1);
			break;
		}
	}
	return handle;
}

const RenderTargetPool::Entry* RenderTargetPool::Get(RenderTargetHandle handle) const
{
	if (!handle.IsValid() || handle.id > m_entries.size())
	{
		return nullptr;
	}
	return &m_entries[handle.id - 1];
}

bool RenderTargetPool::Resize(RenderTargetHandle handle, UINT width, UINT height)
{
	const Entry* current = Get(handle);
	if (!current || !width || !height || width > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION ||
		height > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION) return false;
	if (current->width == width && current->height == height) return true;
	RenderTargetPool replacement;
	replacement.Initialize(m_device, width, height);
	RenderTargetDesc desc = current->description;
	desc.width = width; desc.height = height;
	if (!replacement.Create(desc).IsValid()) return false;
	replacement.m_entries.front().description = current->description;
	m_entries[handle.id - 1] = std::move(replacement.m_entries.front());
	return true;
}

bool RenderTargetPool::ResizeDefaultTargets(UINT width, UINT height)
{
	if (!m_device || !width || !height || width > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION ||
		height > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION) return false;
	if (width == m_defaultWidth && height == m_defaultHeight) return true;
	// Stage all replacements so allocation failure preserves the old collection.
	RenderTargetPool staged = *this;
	for (size_t i = 0; i < m_entries.size(); ++i)
	{
		const auto& desc = m_entries[i].description;
		if (!staged.Resize({static_cast<unsigned int>(i + 1)},
			desc.width ? desc.width : width, desc.height ? desc.height : height)) return false;
	}
	m_entries = std::move(staged.m_entries);
	m_defaultWidth = width; m_defaultHeight = height;
	return true;
}
