#include "DeferredLightingRenderer.h"

#include <vector>

#include "RendererHelper.h"

bool DeferredLightingRenderer::Initialize(
	ID3D11Device* device,
	ID3D11DeviceContext* context)
{
	if (!device || !context)
	{
		return false;
	}

	m_context = context;
	ComPtr<ID3D11Device> devicePtr = device;
	const std::vector<D3D11_INPUT_ELEMENT_DESC> noInputElements;

	// Fullscreen Triangle은 SV_VertexID로 정점 위치를 생성하므로 Input Layout이 필요 없다.
	// 기존 G-Buffer 디버그 뷰와 같은 Vertex Shader를 재사용한다.
	RendererHelper::CreateVertexShaderAndInputLayout(
		devicePtr,
		"../DearsGraphicsEngine/Shader/GBufferDebugVertexShader.hlsl",
		noInputElements,
		m_vertexShader,
		m_unusedInputLayout);
	RendererHelper::CreatePixelShader(
		devicePtr,
		"../DearsGraphicsEngine/Shader/DeferredLightingPixelShader.hlsl",
		m_pixelShader);

	// Lighting Pass는 G-Buffer의 각 픽셀을 화면에 옮기는 단계이므로
	// 새 Geometry의 깊이를 검사하거나 Depth Buffer에 값을 쓸 필요가 없다.
	D3D11_DEPTH_STENCIL_DESC depthDesc{};
	depthDesc.DepthEnable = FALSE;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	depthDesc.StencilEnable = FALSE;
	if (FAILED(device->CreateDepthStencilState(
		&depthDesc,
		m_depthDisabledState.GetAddressOf())))
	{
		return false;
	}

	return m_vertexShader && m_pixelShader;
}

void DeferredLightingRenderer::Render()
{
	if (!m_context || !m_vertexShader || !m_pixelShader || !m_depthDisabledState)
	{
		return;
	}

	// Geometry/Hull/Domain Shader가 이전 Draw에서 남아 있으면 Fullscreen Pass에 필요 없는
	// 단계가 실행될 수 있으므로 사용하지 않는 Shader Stage를 명시적으로 비운다.
	m_context->HSSetShader(nullptr, nullptr, 0);
	m_context->DSSetShader(nullptr, nullptr, 0);
	m_context->GSSetShader(nullptr, nullptr, 0);

	// Vertex Buffer와 Index Buffer 없이 SV_VertexID 0, 1, 2로 큰 삼각형 하나를 만든다.
	// 사각형 두 장보다 정점이 적고 화면 중앙의 대각선 경계도 생기지 않는다.
	m_context->IASetInputLayout(nullptr);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	// Deferred 결과는 불투명 Scene Color이므로 이전 색과 블렌딩하지 않는다.
	// Pixel Shader의 discard가 실행된 배경 픽셀만 기존 Back Buffer 색을 유지한다.
	m_context->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	m_context->OMSetDepthStencilState(m_depthDisabledState.Get(), 0);
	m_context->Draw(3, 0);
}
