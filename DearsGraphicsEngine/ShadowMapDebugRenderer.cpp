#include "ShadowMapDebugRenderer.h"

#include <algorithm>
#include <vector>

#include "RendererHelper.h"

bool ShadowMapDebugRenderer::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
	if (!device || !context)
	{
		return false;
	}

	m_context = context;
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
		"../DearsGraphicsEngine/Shader/ShadowMapDebugPixelShader.hlsl",
		m_pixelShader);

	// 미리보기는 UI에서 16:9로 보여주므로 렌더 타깃도 같은 비율로 만든다.
	// 원본 Shadow Map 해상도와 분리해 두면 디버그 UI 때문에 큰 텍스처를 한 장 더 만들지 않아도 된다.
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = 640;
	textureDesc.Height = 360;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	if (FAILED(device->CreateTexture2D(&textureDesc, nullptr, m_texture.GetAddressOf())) ||
		FAILED(device->CreateRenderTargetView(m_texture.Get(), nullptr, m_renderTargetView.GetAddressOf())) ||
		FAILED(device->CreateShaderResourceView(m_texture.Get(), nullptr, m_shaderResourceView.GetAddressOf())))
	{
		return false;
	}
	const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_context->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);

	DebugConstants constants;
	m_constantBuffer = RendererHelper::CreateConstantBuffer(devicePtr, constants);
	return m_vertexShader && m_pixelShader && m_constantBuffer;
}

void ShadowMapDebugRenderer::Render(
	ID3D11ShaderResourceView* shadowMap,
	float nearPlane,
	float farPlane,
	bool isPerspective)
{
	if (!m_context || !shadowMap || !m_renderTargetView || !m_vertexShader || !m_pixelShader)
	{
		return;
	}

	DebugConstants constants;
	constants.nearPlane = (std::max)(nearPlane, 0.001f);
	constants.farPlane = (std::max)(farPlane, constants.nearPlane + 0.001f);
	constants.isPerspective = isPerspective ? 1.0f : 0.0f;
	RendererHelper::UpdateBuffer(m_context, constants, m_constantBuffer);

	// 그림자 패스에서 DSV 쓰기가 끝난 뒤 같은 리소스를 SRV로 읽어 디버그 색상으로 변환한다.
	// 출력은 별도 RGBA 텍스처이므로 실제 그림자 판정에 사용하는 Depth 값에는 영향이 없다.
	UINT previousViewportCount = 1;
	D3D11_VIEWPORT previousViewport{};
	m_context->RSGetViewports(&previousViewportCount, &previousViewport);
	D3D11_VIEWPORT previewViewport{};
	previewViewport.Width = 640.0f;
	previewViewport.Height = 360.0f;
	previewViewport.MaxDepth = 1.0f;
	m_context->RSSetViewports(1, &previewViewport);

	ID3D11RenderTargetView* target = m_renderTargetView.Get();
	m_context->OMSetRenderTargets(1, &target, nullptr);
	m_context->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	m_context->OMSetDepthStencilState(nullptr, 0);
	m_context->IASetInputLayout(nullptr);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	m_context->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
	m_context->PSSetShaderResources(0, 1, &shadowMap);
	m_context->Draw(3, 0);

	// 다음 패스가 그림자 맵을 다른 슬롯에 다시 바인딩할 수 있도록 임시 t0 연결을 해제한다.
	ID3D11ShaderResourceView* nullResource = nullptr;
	m_context->PSSetShaderResources(0, 1, &nullResource);
	if (previousViewportCount > 0)
	{
		m_context->RSSetViewports(1, &previousViewport);
	}
}
