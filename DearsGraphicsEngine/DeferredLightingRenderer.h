#pragma once

#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

// Geometry Pass가 만든 G-Buffer를 읽어 최종 Scene Color를 만드는 렌더러이다.
//
// 이 렌더러는 씬의 ModelBuffer나 RenderObject를 전혀 순회하지 않는다.
// 화면의 모든 표면 정보가 이미 G-Buffer에 있으므로, Vertex Buffer 없는
// Fullscreen Triangle 하나를 그려 Pixel Shader를 화면 크기만큼 실행한다.
//
// GameEngine은 "Lighting Pass를 실행한다"는 사실만 알고,
// 실제 DX11 Shader/Depth State/Draw 호출은 그래픽스 계층 안에 숨긴다.
class DeferredLightingRenderer
{
public:
	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
	void Render();

private:
	ID3D11DeviceContext* m_context = nullptr;
	ComPtr<ID3D11VertexShader> m_vertexShader;
	ComPtr<ID3D11PixelShader> m_pixelShader;
	ComPtr<ID3D11InputLayout> m_unusedInputLayout;
	ComPtr<ID3D11DepthStencilState> m_depthDisabledState;
};
