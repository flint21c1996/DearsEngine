#include "GBuffer.h"

#include <algorithm>

namespace
{
constexpr UINT DepthShaderResourceOffset = GBuffer::TargetCount;
}

bool GBuffer::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, UINT width, UINT height)
{
	if (!device || !context)
	{
		return false;
	}

	m_device = device;
	m_context = context;
	return Resize(width, height);
}

bool GBuffer::Resize(UINT width, UINT height)
{
	if (!m_device || width == 0 || height == 0)
	{
		return false;
	}

	if (m_width == width && m_height == height && m_colorTextures[0])
	{
		return true;
	}

	ReleaseResources();
	m_width = width;
	m_height = height;

	// Albedo는 일반 색상 범위이므로 8-bit UNORM이면 충분하다.
	// Normal은 -1~1 방향값의 작은 차이가 조명에 크게 보이므로 16-bit float를 사용한다.
	// Material은 Metallic/Roughness/AO처럼 0~1 값들이므로 8-bit UNORM으로 묶는다.
	return CreateColorTarget(static_cast<size_t>(Target::Albedo), DXGI_FORMAT_R8G8B8A8_UNORM) &&
		CreateColorTarget(static_cast<size_t>(Target::Normal), DXGI_FORMAT_R16G16B16A16_FLOAT) &&
		CreateColorTarget(static_cast<size_t>(Target::Material), DXGI_FORMAT_R8G8B8A8_UNORM) &&
		CreateDepthTarget();
}

void GBuffer::BindForGeometryPass()
{
	if (!m_context || !m_depthStencilView)
	{
		return;
	}

	// 전 프레임 Lighting Pass에서 SRV로 읽었던 바인딩을 먼저 해제한다.
	// DX11은 충돌 시 경고와 함께 일부 바인딩을 강제로 NULL로 만들 수 있으므로,
	// 패스 경계에서 의도를 명확히 표현하는 편이 디버깅하기 쉽다.
	UnbindShaderResources(LightingShaderSlot);

	std::array<ID3D11RenderTargetView*, TargetCount> renderTargets{};
	for (size_t index = 0; index < renderTargets.size(); ++index)
	{
		renderTargets[index] = m_renderTargetViews[index].Get();
	}

	m_context->OMSetRenderTargets(TargetCount, renderTargets.data(), m_depthStencilView.Get());

	const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	for (const ComPtr<ID3D11RenderTargetView>& renderTarget : m_renderTargetViews)
	{
		m_context->ClearRenderTargetView(renderTarget.Get(), clearColor);
	}
	m_context->ClearDepthStencilView(
		m_depthStencilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);
}

void GBuffer::BindForLightingPass(UINT firstShaderSlot)
{
	if (!m_context)
	{
		return;
	}

	// RTV/DSV 쓰기를 끝낸 뒤에만 같은 텍스처를 SRV로 읽을 수 있다.
	// 이 함수가 출력 대상을 해제하므로 Lighting Pass의 실제 출력 대상(백 버퍼)은
	// 이 함수가 끝난 다음에 Renderer가 바인딩해야 한다.
	m_context->OMSetRenderTargets(0, nullptr, nullptr);

	std::array<ID3D11ShaderResourceView*, TargetCount + 1> resources{};
	for (size_t index = 0; index < m_shaderResourceViews.size(); ++index)
	{
		resources[index] = m_shaderResourceViews[index].Get();
	}
	resources[DepthShaderResourceOffset] = m_depthShaderResourceView.Get();

	m_context->PSSetShaderResources(firstShaderSlot, static_cast<UINT>(resources.size()), resources.data());
}

void GBuffer::UnbindShaderResources(UINT firstShaderSlot)
{
	if (!m_context)
	{
		return;
	}

	std::array<ID3D11ShaderResourceView*, TargetCount + 1> nullResources{};
	m_context->PSSetShaderResources(
		firstShaderSlot,
		static_cast<UINT>(nullResources.size()),
		nullResources.data());
}

ID3D11ShaderResourceView* GBuffer::GetShaderResourceView(Target target) const
{
	const size_t index = static_cast<size_t>(target);
	return index < m_shaderResourceViews.size() ? m_shaderResourceViews[index].Get() : nullptr;
}

bool GBuffer::CreateColorTarget(size_t index, DXGI_FORMAT format)
{
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = m_width;
	textureDesc.Height = m_height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	if (FAILED(m_device->CreateTexture2D(&textureDesc, nullptr, m_colorTextures[index].GetAddressOf())))
	{
		return false;
	}
	if (FAILED(m_device->CreateRenderTargetView(
		m_colorTextures[index].Get(), nullptr, m_renderTargetViews[index].GetAddressOf())))
	{
		return false;
	}
	return SUCCEEDED(m_device->CreateShaderResourceView(
		m_colorTextures[index].Get(), nullptr, m_shaderResourceViews[index].GetAddressOf()));
}

bool GBuffer::CreateDepthTarget()
{
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = m_width;
	textureDesc.Height = m_height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	if (FAILED(m_device->CreateTexture2D(&textureDesc, nullptr, m_depthTexture.GetAddressOf())))
	{
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc{};
	depthViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	if (FAILED(m_device->CreateDepthStencilView(
		m_depthTexture.Get(), &depthViewDesc, m_depthStencilView.GetAddressOf())))
	{
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc{};
	resourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	resourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resourceViewDesc.Texture2D.MipLevels = 1;
	return SUCCEEDED(m_device->CreateShaderResourceView(
		m_depthTexture.Get(), &resourceViewDesc, m_depthShaderResourceView.GetAddressOf()));
}

void GBuffer::ReleaseResources()
{
	for (ComPtr<ID3D11Texture2D>& texture : m_colorTextures)
	{
		texture.Reset();
	}
	for (ComPtr<ID3D11RenderTargetView>& view : m_renderTargetViews)
	{
		view.Reset();
	}
	for (ComPtr<ID3D11ShaderResourceView>& view : m_shaderResourceViews)
	{
		view.Reset();
	}

	m_depthTexture.Reset();
	m_depthStencilView.Reset();
	m_depthShaderResourceView.Reset();
}
