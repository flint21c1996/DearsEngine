#pragma once
#include<windows.h>
#include<wrl.h>
#include<d3d11.h>
#include <array>
#include <directxtk/SimpleMath.h>
#include "ModelBuffer.h"
#include "GraphicsResourceManager.h"
#include "RendererHelper.h"
#include "Camera.h"
#include "PipelineStateObject.h "
#include "GraphicsCommon.h"

using Microsoft::WRL::ComPtr;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;
using DirectX::SimpleMath::Quaternion;


class Renderer
{
public:
	
	Renderer(HWND _hWnd, int _startScreenWidth, int _startScreenHeight, int _endScreenWidth, int _endScreenHeight,
			ComPtr<ID3D11Device>& _pDevice, ComPtr<ID3D11DeviceContext> _pDeviceContext);
	~Renderer();
private:
	HWND m_hWnd;				
	int m_startScreenWidth;		
	int m_startScreenHeight;	
	int m_endScreenWidth;		
	int m_endScreenHeight;		

	int m_ScreenWidth;
	int m_ScreenHeight;

private:
	// D3D 肄붿뼱 由ъ냼??
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pDeviceContext;
	ComPtr<IDXGISwapChain> mpSwapChain;

	// ?뚮뜑 ?寃?
	ComPtr<ID3D11Texture2D> mpBackBuffer;
	ComPtr<ID3D11ShaderResourceView> mpMainRenderTargetSRV;
	ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;

	// ?꾩떆 踰꾪띁 (?ъ뒪???꾨줈?몄떛??
	ComPtr<ID3D11Texture2D> mpTempBuffer;
	ComPtr<ID3D11ShaderResourceView> mpTempTargetSRV;
	ComPtr<ID3D11RenderTargetView> m_pTempTargetview;

	// 酉고룷??
	D3D11_VIEWPORT m_pD3dScreenViewport;
	D3D11_VIEWPORT m_pD3dtempViewport;
	// Shadow Map은 에디터 패널 때문에 폭이 줄어드는 메인 viewport와 크기가 다르다.
	// 전용 viewport를 사용해야 Depth가 Shadow Map 텍스처 전체 영역에 기록된다.
	D3D11_VIEWPORT m_shadowViewport;

	// 源딆씠/?ㅽ뀗??
	ComPtr<ID3D11DepthStencilView> mpDepthStencilView;
	// Directional/Spot Light는 각각 2D Shadow Map 한 장을 사용한다.
	// Texture2DArray의 slice 번호를 GPU Light 배열 인덱스와 같게 두면,
	// 별도의 DX11 리소스 포인터를 Light 구조체에 넣지 않아도 셰이더가 자기 맵을 찾을 수 있다.
	ComPtr<ID3D11Texture2D> m_twoDimensionalShadowArrayTexture;
	std::array<ComPtr<ID3D11DepthStencilView>, MAX_LIGHTS> m_twoDimensionalShadowDSVs;
	ComPtr<ID3D11ShaderResourceView> m_twoDimensionalShadowArraySRV;

	// Point Light 한 개는 여섯 방향을 보므로 Cube 하나가 Texture2D slice 여섯 장을 사용한다.
	// 여러 Point Light를 동시에 지원하기 위해 MAX_LIGHTS개의 Cube를 하나의 Cube Array로 묶는다.
	// 실제 slice 번호는 lightIndex * 6 + faceIndex로 계산한다.
	ComPtr<ID3D11Texture2D> m_pointShadowCubeTexture;
	std::array<ComPtr<ID3D11DepthStencilView>, MAX_LIGHTS * 6> m_pointShadowCubeDSVs;
	ComPtr<ID3D11ShaderResourceView> m_pointShadowCubeSRV;
	D3D11_VIEWPORT m_pointShadowViewport{};
	UINT m_pointShadowMapSize = 1024;

	// 기존 Shadow Draw 함수는 현재 활성 DSV가 2D인지 Cube의 한 면인지 몰라도 된다.
	// Begin*ShadowPass가 아래 두 포인터만 바꿔 주고 실제 메시 렌더 코드는 이를 공통으로 사용한다.
	ID3D11DepthStencilView* m_activeShadowDSV = nullptr;
	const D3D11_VIEWPORT* m_activeShadowViewport = nullptr;

	// ?먮툕留?SRV
	ComPtr<ID3D11ShaderResourceView> mpCubeMapEnvironmentResourceView;
	ComPtr<ID3D11ShaderResourceView> mpCubeMapDiffuseResourceView;
	ComPtr<ID3D11ShaderResourceView> mpCubeMapSpecularResourceView;
	ComPtr<ID3D11ShaderResourceView> mpCubeMapBRDFResourceView;

	// 怨듭슜 ?곸닔 踰꾪띁
	ComPtr<ID3D11Buffer> mpCommonConstantBuffer;

	UINT m_numQualityLevels;

	// ?대? ?ы띁 諛?鍮꾩냼??李몄“
	GraphicsResourceManager* m_pResourceManager;
	RendererHelper* m_pRenderHelper;
	Camera* mpTargetCamera;

public:
	ComPtr<ID3D11Device> GetDevice() const { return m_pDevice; }
	// 그림자 맵의 소유권은 Renderer가 유지하고, 디버그 패스에는 읽기 전용 뷰만 빌려준다.
	// 외부에서 ComPtr를 보관하거나 리소스를 교체하지 못하게 raw pointer로 반환한다.
	ID3D11ShaderResourceView* GetShadowMapShaderResourceView() const { return m_twoDimensionalShadowArraySRV.Get(); }
	ID3D11ShaderResourceView* GetPointShadowCubeShaderResourceView() const { return m_pointShadowCubeSRV.Get(); }

	void SetCamera(Camera* _pTargetCamera);
	bool Initialize(GraphicsResourceManager* _pResourceManager);
	void SetViewportWidth(int viewportWidth);
	void BeginRender();

	// G-Buffer 쓰기가 끝난 뒤 최종 조명/포워드 결과를 다시 백 버퍼에 기록한다.
	// depthStencilView를 받는 이유는 Geometry Pass의 깊이를 Forward Pass에서도
	// 이어서 사용해야 불투명 물체 앞뒤 관계가 유지되기 때문이다.
	void BindMainRenderTarget(ID3D11DepthStencilView* depthStencilView = nullptr);
	void BeginTwoDimensionalShadowPass(UINT lightIndex);
	void BeginPointShadowFace(UINT lightIndex, UINT faceIndex);
	void EndShadowPass();

	void SetCommonShaderResource(ComPtr<ID3D11ShaderResourceView> _environmentTexture,
									ComPtr<ID3D11ShaderResourceView> _diffuseTexture,
									ComPtr<ID3D11ShaderResourceView> _specularTexture,
									ComPtr<ID3D11ShaderResourceView> _BRDFTexture);

	//占쏙옙占쏙옙 占쏙옙占싱댐옙 占쏙옙占쌀쏙옙占쏙옙 GPU占쏙옙 Set占쏙옙占쌔댐옙
	bool SetCommonShaderResourceToGPU();


	//占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙트占쏙옙 Set占쏙옙占쌔댐옙.(Render 호占쏙옙占쏙옙占쏙옙 占십쇽옙)common
	void SetCommonConstant(ComPtr<ID3D11Buffer>& commonConstsBuffer);
	void Render(ModelBuffer* _modelbuffer);
	void Render_CubeMap(ModelBuffer* _modelbuffer);
	void EndRender();
	void Finalize();
	void SetPipelineState(const PipelineStateObject& _pso);

	void LightUpdate(CommonConstantBufferData* _psBufferData);
	void UpdateCommonConstantBuffer(CommonConstantBufferData& _CommonBufferData);

	void RenderDepthMap(ModelBuffer* _modelbuffer);
	void RenderPbrDepthMap(ModelBuffer* _modelbuffer);
	void RenderAniDepthMap(ModelBuffer* _modelbuffer);
	void RenderEquipDepthMap(ModelBuffer* _modelbuffer);

	void RenderEdge(ModelBuffer* _modelbuffer);
	void RenderSampler(ModelBuffer* _modelbuffer);
	void RenderPostProcessing(ModelBuffer* _modelbuffer);

	void Rend_InstancedModels(ModelBuffer* _modelBuffers);

	unsigned int objectIndex = 0;

private:
	bool InitalizeD3D();
	bool CreateTwoDimensionalShadowArray();
	bool CreatePointShadowCube();
};


