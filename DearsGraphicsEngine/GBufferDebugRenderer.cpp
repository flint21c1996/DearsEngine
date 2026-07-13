#include "GBufferDebugRenderer.h"

#include <vector>

#include "GBuffer.h"
#include "RendererHelper.h"

bool GBufferDebugRenderer::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, GBuffer* gBuffer)
{
	if (!device || !context || !gBuffer)
	{
		return false;
	}

	m_device = device;
	m_context = context;
	m_gBuffer = gBuffer;

	ComPtr<ID3D11Device> devicePtr = device;
	const std::vector<D3D11_INPUT_ELEMENT_DESC> noInputElements;
	RendererHelper::CreateVertexShaderAndInputLayout(
		devicePtr,
		"../DearsGraphicsEngine/Shader/GBufferDebugVertexShader.hlsl",
		noInputElements,
		m_vertexShader,
		m_unusedInputLayout);
	RendererHelper::CreatePixelShader(
		devicePtr,
		"../DearsGraphicsEngine/Shader/GBufferDebugPixelShader.hlsl",
		m_pixelShader);
	return Resize(gBuffer->GetWidth(), gBuffer->GetHeight());
}

void GBufferDebugRenderer::Render()
{
	if (!m_context || !m_gBuffer || !m_vertexShader || !m_pixelShader)
	{
		return;
	}

	// Render()는 ImGui가 SRV 포인터를 이미 보관한 프레임 중간에 호출된다.
	// 따라서 여기서는 절대 리소스를 재생성하지 않고, 크기가 다르면 해당 프레임만 건너뛴다.
	// 실제 Resize는 SetRenderViewportWidth()처럼 프레임 렌더 전에 호출되는 경로가 담당한다.
	if (m_width != m_gBuffer->GetWidth() || m_height != m_gBuffer->GetHeight())
	{
		return;
	}

	std::array<ID3D11RenderTargetView*, ViewCount> targets{};
	for (size_t index = 0; index < targets.size(); ++index)
	{
		targets[index] = m_renderTargetViews[index].Get();
	}

	// 직전 프레임 ImGui가 이 SRV들을 사용했을 수 있으므로 RTV로 묶기 전에 낮은 PS 슬롯을 비운다.
	std::array<ID3D11ShaderResourceView*, 8> nullResources{};
	m_context->PSSetShaderResources(0, static_cast<UINT>(nullResources.size()), nullResources.data());
	m_context->OMSetRenderTargets(ViewCount, targets.data(), nullptr);
	m_context->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	m_context->OMSetDepthStencilState(nullptr, 0);
	m_context->IASetInputLayout(nullptr);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	m_context->Draw(3, 0);
}

ID3D11ShaderResourceView* GBufferDebugRenderer::GetView(View view) const
{
	const size_t index = static_cast<size_t>(view);
	return index < m_shaderResourceViews.size() ? m_shaderResourceViews[index].Get() : nullptr;
}

bool GBufferDebugRenderer::Resize(UINT width, UINT height)
{
	if (!m_device || width == 0 || height == 0)
	{
		return false;
	}
	if (m_width == width && m_height == height && m_textures[0])
	{
		return true;
	}

	ReleaseTargets();
	m_width = width;
	m_height = height;

	for (size_t index = 0; index < m_textures.size(); ++index)
	{
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		if (FAILED(m_device->CreateTexture2D(&desc, nullptr, m_textures[index].GetAddressOf())) ||
			FAILED(m_device->CreateRenderTargetView(m_textures[index].Get(), nullptr, m_renderTargetViews[index].GetAddressOf())) ||
			FAILED(m_device->CreateShaderResourceView(m_textures[index].Get(), nullptr, m_shaderResourceViews[index].GetAddressOf())))
		{
			return false;
		}
	}
	return true;
}

void GBufferDebugRenderer::ReleaseTargets()
{
	for (auto& value : m_textures) value.Reset();
	for (auto& value : m_renderTargetViews) value.Reset();
	for (auto& value : m_shaderResourceViews) value.Reset();
}
