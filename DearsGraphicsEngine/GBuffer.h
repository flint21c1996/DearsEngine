#pragma once

#include <array>
#include <d3d11.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

// 디퍼드 렌더링의 Geometry Pass가 기록할 화면 공간 데이터를 소유한다.
//
// 하나의 픽셀에 최종 색상만 쓰는 포워드 렌더링과 달리, 디퍼드 렌더링은
// 표면의 색상/노멀/재질 값을 서로 다른 렌더 타깃에 나눠 저장한다.
// Lighting Pass는 이 텍스처들을 SRV로 읽어 화면 전체의 조명을 계산한다.
//
// DX11 리소스는 이 클래스 안에만 두었다. GameEngine은 G-Buffer의 구체적인
// DX11 타입을 알 필요 없이 Geometry/Lighting 패스만 요청해야 하기 때문이다.
class GBuffer
{
public:
	// MRT에 바인딩하는 순서와 셰이더의 SV_Target 번호가 일치해야 한다.
	// enum 값을 배열 인덱스로 사용하면 C++과 HLSL의 슬롯 대응을 한 곳에서 관리할 수 있다.
	enum class Target : size_t
	{
		Albedo = 0,   // 표면의 기본색(RGB)과 여분 채널(A)
		Normal,       // 월드 공간 노멀(XYZ). 정밀도를 위해 16-bit float 사용
		Material,     // Metallic, Roughness, AO 등 PBR 스칼라 값
	};

	// Windows SDK의 일부 헤더/매크로와 Count 같은 짧은 이름이 충돌할 수 있으므로
	// 배열 크기는 의미가 분명한 고정 상수로 둔다.
	static constexpr UINT TargetCount = 3;
	// 기존 PBR 텍스처와 shadow map 슬롯을 건드리지 않도록 높은 슬롯부터 사용한다.
	// Lighting HLSL에서도 t20~t23을 같은 순서로 선언할 예정이다.
	static constexpr UINT LightingShaderSlot = 20;

	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, UINT width, UINT height);
	bool Resize(UINT width, UINT height);

	// Geometry Pass 시작 시 MRT와 깊이 버퍼를 출력 대상으로 묶고 초기화한다.
	void BindForGeometryPass();

	// Lighting Pass 시작 시 G-Buffer를 픽셀 셰이더 입력으로 묶는다.
	// 같은 텍스처를 RTV(쓰기)와 SRV(읽기)로 동시에 사용할 수 없으므로
	// 먼저 출력 대상에서 해제한 뒤 SRV로 바인딩해야 한다.
	void BindForLightingPass(UINT firstShaderSlot);

	// 다음 프레임에 다시 RTV로 쓰기 전에 이전 SRV 바인딩을 명시적으로 해제한다.
	void UnbindShaderResources(UINT firstShaderSlot);

	ID3D11DepthStencilView* GetDepthStencilView() const { return m_depthStencilView.Get(); }
	ID3D11ShaderResourceView* GetDepthShaderResourceView() const { return m_depthShaderResourceView.Get(); }
	ID3D11ShaderResourceView* GetShaderResourceView(Target target) const;
	UINT GetWidth() const { return m_width; }
	UINT GetHeight() const { return m_height; }

private:
	bool CreateColorTarget(size_t index, DXGI_FORMAT format);
	bool CreateDepthTarget();
	void ReleaseResources();

	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_context = nullptr;
	UINT m_width = 0;
	UINT m_height = 0;

	std::array<ComPtr<ID3D11Texture2D>, TargetCount> m_colorTextures;
	std::array<ComPtr<ID3D11RenderTargetView>, TargetCount> m_renderTargetViews;
	std::array<ComPtr<ID3D11ShaderResourceView>, TargetCount> m_shaderResourceViews;

	// Depth는 Geometry Pass에서는 DSV로 쓰고 Lighting Pass에서는 SRV로 읽는다.
	// 그래서 실제 텍스처는 R24G8_TYPELESS로 만들고, 각 View에서 용도를 확정한다.
	ComPtr<ID3D11Texture2D> m_depthTexture;
	ComPtr<ID3D11DepthStencilView> m_depthStencilView;
	ComPtr<ID3D11ShaderResourceView> m_depthShaderResourceView;
};
