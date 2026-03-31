#pragma once
#include<windows.h>
#include<wrl.h>
#include<d3d11.h>
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

	// 源딆씠/?ㅽ뀗??
	ComPtr<ID3D11DepthStencilView> mpDepthStencilView;
	ComPtr<ID3D11DepthStencilView> m_depthOnlyDSV;
	ComPtr<ID3D11ShaderResourceView> m_depthOnlySRV;

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

	void SetCamera(Camera* _pTargetCamera);
	bool Initialize(GraphicsResourceManager* _pResourceManager);
	void SetViewportWidth(int viewportWidth);
	void BeginRender();

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
	void RenderAniDepthMap(ModelBuffer* _modelbuffer);
	void RenderEquipDepthMap(ModelBuffer* _modelbuffer);

	void RenderEdge(ModelBuffer* _modelbuffer);
	void RenderSampler(ModelBuffer* _modelbuffer);
	void RenderPostProcessing(ModelBuffer* _modelbuffer);

	void Rend_InstancedModels(ModelBuffer* _modelBuffers);

	unsigned int objectIndex = 0;

private:
	bool InitalizeD3D();
};


