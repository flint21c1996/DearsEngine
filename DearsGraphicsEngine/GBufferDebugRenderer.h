#pragma once

#include <array>
#include <d3d11.h>
#include <wrl.h>

class GBuffer;
using Microsoft::WRL::ComPtr;

// 실제 G-Buffer를 변경하지 않고 디버그 표시용 이미지만 생성하는 전체 화면 패스이다.
// Material 채널 분리와 Position 복원처럼 ImGui 기본 Image 셰이더로 할 수 없는 변환을 담당한다.
class GBufferDebugRenderer
{
public:
	enum class View : size_t
	{
		Metallic = 0,
		Roughness,
		AO,
		LinearDepth,
		WorldPosition,
	};
	static constexpr UINT ViewCount = 5;

	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, GBuffer* gBuffer);

	// ImGui가 SRV 포인터를 DrawData에 저장하기 전에 디버그 텍스처 크기를 확정한다.
	// 프레임 도중 리소스를 재생성하면 ImGui가 해제된 SRV를 참조할 수 있다.
	bool Resize(UINT width, UINT height);
	void Render();
	ID3D11ShaderResourceView* GetView(View view) const;

private:
	void ReleaseTargets();

	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_context = nullptr;
	GBuffer* m_gBuffer = nullptr;
	UINT m_width = 0;
	UINT m_height = 0;
	ComPtr<ID3D11VertexShader> m_vertexShader;
	ComPtr<ID3D11PixelShader> m_pixelShader;
	ComPtr<ID3D11InputLayout> m_unusedInputLayout;
	std::array<ComPtr<ID3D11Texture2D>, ViewCount> m_textures;
	std::array<ComPtr<ID3D11RenderTargetView>, ViewCount> m_renderTargetViews;
	std::array<ComPtr<ID3D11ShaderResourceView>, ViewCount> m_shaderResourceViews;
};
