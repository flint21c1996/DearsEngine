#pragma once

#include <d3d11.h>
#include <string>
#include <vector>
#include <wrl.h>

#include "RenderPassIO.h"

// 게임 엔진이 CreateRenderTarget()으로 요청한 오프스크린 렌더 타깃의 DX11 리소스를 소유한다.
//
// 바깥 계층은 RenderTargetHandle만 들고 있고, 실제 텍스처/RTV/SRV/DSV는 이 클래스 안에만 있다.
// 어떤 타깃을 어떤 깊이와 묶어서 출력으로 쓸지는 DearsGraphicsEngine이 RenderPassIO를 보고 결정하며,
// 이 클래스는 "만들고, 찾아 주는" 일만 한다.
class RenderTargetPool
{
public:
	struct Entry
	{
		RenderTargetDesc description;
		std::string name;
		UINT width = 0;
		UINT height = 0;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> colorTexture;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
		// RenderTargetDesc::withDepth가 false면 비어 있다.
		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthTexture;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
		// 타깃 전체를 덮는 뷰포트이다.
		D3D11_VIEWPORT viewport{};
	};

	// defaultWidth/defaultHeight는 RenderTargetDesc의 크기가 0일 때 쓰는 백 버퍼 크기이다.
	void Initialize(ID3D11Device* device, UINT defaultWidth, UINT defaultHeight);

	RenderTargetHandle Create(const RenderTargetDesc& desc);
	RenderTargetHandle Find(const std::string& name) const;
	// 유효하지 않은 핸들이면 nullptr을 돌려준다.
	const Entry* Get(RenderTargetHandle handle) const;
	// Called before generating UI draw data, with previous SRVs/RTVs unbound.
	// Handles stay stable; borrowed Entry pointers must be reacquired after resize.
	bool Resize(RenderTargetHandle handle, UINT width, UINT height);
	bool ResizeDefaultTargets(UINT width, UINT height);

private:
	ID3D11Device* m_device = nullptr;
	UINT m_defaultWidth = 0;
	UINT m_defaultHeight = 0;

	// 핸들 id는 "배열 인덱스 + 1"이다. 타깃을 중간에 지우지 않으므로 핸들이 무효화되지 않는다.
	std::vector<Entry> m_entries;
};
