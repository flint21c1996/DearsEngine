#pragma once

#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

// 그림자 맵의 Depth를 직교/원근 투영 방식에 맞춰 사람이 확인하기 쉬운 색상으로 변환한다.
// 실제 그림자 맵은 전혀 수정하지 않으며, 에디터에 표시할 RGBA 텍스처만 별도로 만든다.
class ShadowMapDebugRenderer
{
public:
	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
	void Render(ID3D11ShaderResourceView* shadowMap, UINT lightIndex, float nearPlane, float farPlane, bool isPerspective);
	void RenderPointCube(ID3D11ShaderResourceView* shadowCube, UINT lightIndex, float nearPlane, float farPlane);
	ID3D11ShaderResourceView* GetShaderResourceView() const { return m_shaderResourceView.Get(); }

private:
	struct DebugConstants
	{
		float nearPlane = 0.1f;
		float farPlane = 100.0f;
		float isPerspective = 1.0f;
		UINT lightIndex = 0;
	};
	void RenderInternal(
		ID3D11ShaderResourceView* shadowMap,
		ID3D11PixelShader* pixelShader,
		UINT lightIndex,
		float nearPlane,
		float farPlane,
		bool isPerspective);

	ComPtr<ID3D11DeviceContext> m_context;
	ComPtr<ID3D11VertexShader> m_vertexShader;
	ComPtr<ID3D11PixelShader> m_pixelShader;
	ComPtr<ID3D11PixelShader> m_pointCubePixelShader;
	ComPtr<ID3D11InputLayout> m_unusedInputLayout;
	ComPtr<ID3D11Buffer> m_constantBuffer;
	ComPtr<ID3D11Texture2D> m_texture;
	ComPtr<ID3D11RenderTargetView> m_renderTargetView;
	ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
};
