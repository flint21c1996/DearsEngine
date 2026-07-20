#pragma once
#include "Renderer.h"
#include <directxtk/SimpleMath.h>
using namespace Dears;

Renderer::Renderer(HWND _hWnd, int _startScreenWidth, int _startScreenHeight, int _endScreenWidth, int _endScreenHeight,
				ComPtr<ID3D11Device>& _pDevice, ComPtr<ID3D11DeviceContext> _pDeviceContext)
{
	m_hWnd = _hWnd;
	m_startScreenWidth = _startScreenWidth;
	m_startScreenHeight = _startScreenHeight;
	m_endScreenWidth = _endScreenWidth;
	m_endScreenHeight = _endScreenHeight;
	m_numQualityLevels = 1;
	m_pRenderHelper = nullptr;
	m_pResourceManager = nullptr;
	m_pDevice = _pDevice;
	m_pDeviceContext = _pDeviceContext;

	m_ScreenWidth = m_endScreenWidth - m_startScreenWidth;
	m_ScreenHeight = m_endScreenHeight - m_startScreenHeight;
}

Renderer::~Renderer()
{
	// mpTargetCamera???뚯쑀?섏? ?딆쓬 (DearsGraphicsEngine??愿由?
	delete m_pRenderHelper;
}

void Renderer::SetCamera(Camera* _pTargetCamera)
{
	mpTargetCamera = _pTargetCamera;
}

bool Renderer::Initialize(GraphicsResourceManager* _pResourceManager)
{
	if (!InitalizeD3D())
	{
		return false;
	}
	m_pResourceManager = _pResourceManager;
	// 占쏙옙占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙
	m_pRenderHelper = new RendererHelper();


	CommonConstantBufferData CCBD;	//mpCommonConstantBuffer占쏙옙 占십깍옙화 占쏙옙키占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙
	mpCommonConstantBuffer = RendererHelper::CreateConstantBuffer(m_pDevice, CCBD);
	return true;
}

void Renderer::SetViewportWidth(int viewportWidth)
{
	m_pD3dScreenViewport.Width = static_cast<float>(viewportWidth);
}

void Renderer::BeginRender()
{
	// 占쏙옙占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙트占쏙옙 占쏙옙占쏙옙占싼댐옙. (1占쏙옙占쏙옙, m_pD3dScreenViewport 占쏙옙占쏙옙)
	m_pDeviceContext->RSSetViewports(1, &m_pD3dScreenViewport);

	float clearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f }; // 占쏙옙占쏙옙占?占쏙옙
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), clearColor); // renderTargetView占쏙옙 clearColor占쏙옙 占쏙옙 占쏙옙占쏙옙占?

	float clearColor1[4] = { 0.f, 0.f, 0.f, 0.0f }; // 占쏙옙占쏙옙占?占쏙옙
	m_pDeviceContext->ClearRenderTargetView(m_pTempTargetview.Get(), clearColor1); // renderTargetView占쏙옙 clearColor占쏙옙 占쏙옙 占쏙옙占쏙옙占?

	//占쏙옙占?占쏙옙占시뤄옙占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙占쏙옙 占쏙옙占?
	m_pDeviceContext->VSSetSamplers(0, UINT(Dears::Graphics::sampleStates.size()),
		Dears::Graphics::sampleStates.data());
	m_pDeviceContext->PSSetSamplers(0, UINT(Dears::Graphics::sampleStates.size()),
		Dears::Graphics::sampleStates.data());

	// 占쏙옙占쏙옙-占쏙옙占식실븝옙 占쏙옙占쏙옙
	m_pDeviceContext->ClearDepthStencilView(mpDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	// Shadow Map은 BeginTwoDimensionalShadowPass/BeginPointShadowFace에서 SRV를 해제한 뒤 지운다.
	// 여기서 먼저 Clear하면 이전 프레임의 t15/t16 SRV가 아직 바인딩된 상태일 수 있다.

	m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), mpDepthStencilView.Get());


	//큐占쏙옙占?占쏙옙占쌀쏙옙占쏙옙 GPU占쏙옙 占싱몌옙 占쏙옙占쏙옙占쏙옙占승댐옙.
	SetCommonShaderResourceToGPU();
}

void Renderer::BindMainRenderTarget(ID3D11DepthStencilView* depthStencilView)
{
	ID3D11DepthStencilView* targetDepth = depthStencilView ? depthStencilView : mpDepthStencilView.Get();
	m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), targetDepth);
	m_pDeviceContext->RSSetViewports(1, &m_pD3dScreenViewport);
}

void Renderer::BeginTwoDimensionalShadowPass(UINT lightIndex)
{
	if (lightIndex >= m_twoDimensionalShadowDSVs.size())
	{
		return;
	}

	// 같은 Depth 리소스를 DSV(쓰기)와 SRV(읽기)로 동시에 묶을 수 없다.
	// 배열 전체를 하나의 SRV로 읽기 때문에 단 한 slice에 쓰더라도 t15/t16을 모두 먼저 해제해야 한다.
	ID3D11ShaderResourceView* nullShadowResources[2] = { nullptr, nullptr };
	m_pDeviceContext->PSSetShaderResources(15, 2, nullShadowResources);

	m_pDeviceContext->ClearDepthStencilView(
		m_twoDimensionalShadowDSVs[lightIndex].Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_activeShadowDSV = m_twoDimensionalShadowDSVs[lightIndex].Get();
	m_activeShadowViewport = &m_shadowViewport;
	m_pDeviceContext->RSSetViewports(1, m_activeShadowViewport);
	m_pDeviceContext->OMSetRenderTargets(0, nullptr, m_activeShadowDSV);
}

void Renderer::BeginPointShadowFace(UINT lightIndex, UINT faceIndex)
{
	if (lightIndex >= MAX_LIGHTS || faceIndex >= 6)
	{
		return;
	}
	const UINT arraySlice = lightIndex * 6 + faceIndex;

	// Point Light는 한 위치에서 +X, -X, +Y, -Y, +Z, -Z를 각각 렌더링한다.
	// 매 호출은 TextureCube 배열의 한 면만 DSV로 선택하며, 실제 메시 draw 코드는
	// 현재 면 번호나 TextureCube의 존재를 몰라도 된다.
	ID3D11ShaderResourceView* nullShadowResources[2] = { nullptr, nullptr };
	m_pDeviceContext->PSSetShaderResources(15, 2, nullShadowResources);

	m_activeShadowDSV = m_pointShadowCubeDSVs[arraySlice].Get();
	m_activeShadowViewport = &m_pointShadowViewport;
	m_pDeviceContext->ClearDepthStencilView(
		m_activeShadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_pDeviceContext->RSSetViewports(1, m_activeShadowViewport);
	m_pDeviceContext->OMSetRenderTargets(0, nullptr, m_activeShadowDSV);
}

void Renderer::EndShadowPass()
{
	// 모든 Depth 쓰기가 끝난 다음에만 두 Shadow Map을 셰이더 입력으로 연결한다.
	// t15는 Directional/Spot용 2D 맵, t16은 Point용 Cube 맵으로 고정한다.
	m_pDeviceContext->OMSetRenderTargets(
		1, m_pRenderTargetView.GetAddressOf(), mpDepthStencilView.Get());
	m_pDeviceContext->RSSetViewports(1, &m_pD3dScreenViewport);

	ID3D11ShaderResourceView* shadowResources[2] =
	{
		m_twoDimensionalShadowArraySRV.Get(),
		m_pointShadowCubeSRV.Get()
	};
	m_pDeviceContext->PSSetShaderResources(15, 2, shadowResources);
	m_activeShadowDSV = nullptr;
	m_activeShadowViewport = nullptr;
}

void Renderer::SetCommonShaderResource(ComPtr<ID3D11ShaderResourceView> _environmentTexture, ComPtr<ID3D11ShaderResourceView> _diffuseTexture,
								ComPtr<ID3D11ShaderResourceView> _specularTexture, ComPtr<ID3D11ShaderResourceView> _BRDFTexture)
{
	mpCubeMapEnvironmentResourceView = _environmentTexture;
	mpCubeMapDiffuseResourceView = _diffuseTexture;
	mpCubeMapSpecularResourceView = _specularTexture;
	mpCubeMapBRDFResourceView = _BRDFTexture;
}

void Renderer::SetCommonConstant(ComPtr<ID3D11Buffer>& commonConstsBuffer)
{
	// 占쏙옙占싱댐옙占쏙옙 占싹곤옙占쏙옙 占쏙옙占쏙옙 register(b1)***
	m_pDeviceContext->VSSetConstantBuffers(1, 1, commonConstsBuffer.GetAddressOf());
	m_pDeviceContext->PSSetConstantBuffers(1, 1, commonConstsBuffer.GetAddressOf());
}

bool Renderer::SetCommonShaderResourceToGPU()
{
	if (!mpCubeMapDiffuseResourceView || !mpCubeMapSpecularResourceView)
	{
		DEBUG_LOG("ERROR - SetCommonShaderResource()");
		return false;
	}
	ID3D11ShaderResourceView* pixelResources[4] =
	{
		mpCubeMapEnvironmentResourceView.Get(),
		mpCubeMapDiffuseResourceView.Get(),
		mpCubeMapSpecularResourceView.Get(),
		mpCubeMapBRDFResourceView.Get(),
	};
	m_pDeviceContext->PSSetShaderResources(10, 4, pixelResources);

	// 그림자 패스가 실행되지 않는 프레임에도 마지막으로 완성된 Shadow Map은 읽을 수 있다.
	// 실제 Depth 쓰기 직전에는 Begin*ShadowPass()가 이 슬롯을 안전하게 해제한다.
	ID3D11ShaderResourceView* shadowResources[2] =
	{
		m_twoDimensionalShadowArraySRV.Get(),
		m_pointShadowCubeSRV.Get()
	};
	m_pDeviceContext->PSSetShaderResources(15, 2, shadowResources);
	return true;
}

void Renderer::Render(ModelBuffer* _modelbuffer)
{

	// 占쏙옙占쌔쏙옙/占싸듸옙占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, _modelbuffer->m_pVertexBuffer.GetAddressOf(), &stride, &offset);
	m_pDeviceContext->IASetIndexBuffer(_modelbuffer->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	m_pDeviceContext->VSSetConstantBuffers(0, 1, _modelbuffer->m_pVSConstantBuffer.GetAddressOf());
	m_pDeviceContext->VSSetConstantBuffers(2, 1, _modelbuffer->m_BoneConstantBuffer.GetAddressOf());
	m_pDeviceContext->VSSetConstantBuffers(3, 1, _modelbuffer->m_TargetBoneConstantBuffer.GetAddressOf());
	m_pDeviceContext->VSSetConstantBuffers(5, 1, _modelbuffer->m_pVSWaterConstantBuffer.GetAddressOf());
	m_pDeviceContext->VSSetConstantBuffers(6, 1, _modelbuffer->m_VSPBRConstantBuffer.GetAddressOf());

	m_pDeviceContext->PSSetConstantBuffers(0, 1, _modelbuffer->m_pPSConstantBuffer.GetAddressOf());
	m_pDeviceContext->PSSetConstantBuffers(2, 1, _modelbuffer->m_PSPBRConstantBuffer.GetAddressOf());	//占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙..?
	m_pDeviceContext->PSSetConstantBuffers(3, 1, _modelbuffer->m_PSThinFilmConstantBuffer.GetAddressOf());	//占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙..?

	ID3D11ShaderResourceView* vertexResources[1] =
	{
		_modelbuffer->heightTex.Get(),
	};
	m_pDeviceContext->VSSetShaderResources(0, 1, vertexResources);

	ID3D11ShaderResourceView* pixelResources[1] = 
	{
		_modelbuffer->m_diffusetexture.Get(),
	};
	m_pDeviceContext->PSSetShaderResources(0, 1, pixelResources);
	ID3D11ShaderResourceView* pixelResources1[5] =
	{
		_modelbuffer->albedoTex.Get(),
		_modelbuffer->normalTex.Get(),
		_modelbuffer->aoTex.Get(),
		_modelbuffer->metallicTex.Get(),
		_modelbuffer->roughnessTex.Get(),
	};
	m_pDeviceContext->PSSetShaderResources(1, 5, pixelResources1);



	m_pDeviceContext->DrawIndexed(_modelbuffer->mNumIndices, 0, 0);
	
	//m_pDeviceContext->RSSetViewports(1, &m_pD3dtempViewport);
	//m_pDeviceContext->OMSetRenderTargets(1, m_pTempRednerTargetview.GetAddressOf(), NULL);
	//m_pDeviceContext->DrawIndexed(_modelbuffer->mNumIndices, 0, 0);

	// 렌더 타깃은 개별 오브젝트가 아니라 현재 Render Pass가 소유한다.
	// 여기서 매 Draw 뒤 백 버퍼로 복구하면 Geometry Pass의 첫 오브젝트만 G-Buffer에 기록되고,
	// 두 번째 오브젝트부터는 백 버퍼에 기록되는 문제가 생긴다.
	// 다음 패스의 ApplyRenderContext()가 필요한 렌더 타깃을 명시적으로 바인딩한다.

}

void Renderer::Render_CubeMap(ModelBuffer* _modelbuffer)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	// 큐占쏙옙占쏙옙占?
	m_pDeviceContext->IASetVertexBuffers(
		0, 1, _modelbuffer->m_pVertexBuffer.GetAddressOf(), &stride,
		&offset);
	m_pDeviceContext->IASetIndexBuffer(_modelbuffer->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_pDeviceContext->VSSetConstantBuffers(
		0, 1, _modelbuffer->m_pVSConstantBuffer.GetAddressOf());
	ID3D11ShaderResourceView* views[2] = {
	_modelbuffer->m_cubeMapDiffuseResourceView.Get(),
	_modelbuffer->m_cubeMapSpecularResourceView.Get()
	};
	m_pDeviceContext->PSSetShaderResources(0, 2, views);
	m_pDeviceContext->DrawIndexed(_modelbuffer->mNumIndices, 0, 0);

}

void Renderer::EndRender()
{
	HRESULT hr = (mpSwapChain->Present(0, 0));
	if (FAILED(hr))
	{
		LPWSTR output;
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
			FORMAT_MESSAGE_ALLOCATE_BUFFER,
			NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&output, 0,
			NULL);
		MessageBox(NULL, output, L"EndRender Error", MB_OK);
	}
}

void Renderer::Finalize()
{

}

bool Renderer::InitalizeD3D()
{
	// 占쏙옙占쏙옙 체占쏙옙 占쏙옙占쏙옙
	RendererHelper::CreateSwapChain(m_pDevice, m_hWnd, m_numQualityLevels, m_endScreenWidth, m_endScreenHeight, mpSwapChain);
	 
	// 占쏙옙占쏙옙 占쏙옙占쏙옙 타占쏙옙 占쏙옙 占쏙옙占쏙옙
	RendererHelper::CreateRenderTargetView(m_pDevice, mpSwapChain, m_pRenderTargetView, mpBackBuffer);
	HRESULT hr = m_pDevice->CreateShaderResourceView(mpBackBuffer.Get(), nullptr, mpMainRenderTargetSRV.GetAddressOf());

	// 占썩본 占쏙옙 占쏙옙트 占쏙옙占쏙옙
	RendererHelper::SetViewPort(m_pDeviceContext, m_startScreenWidth, m_startScreenHeight, m_endScreenWidth, m_endScreenHeight, m_pD3dScreenViewport);
	// 占쏙옙占쏙옙占?占쏙옙 占쏙옙트
	RendererHelper::SetViewPort(m_pDeviceContext, 0, 0, 1920 *0.5, 1080 * 0.5, m_pD3dtempViewport);
	// Shadow Map 텍스처는 m_ScreenWidth x m_ScreenHeight로 생성된다.
	// 메인 viewport는 Inspector 폭에 따라 줄어들 수 있으므로 그림자 전용 크기를 보관한다.
	RendererHelper::SetShadowViewport(
		m_pDeviceContext,
		static_cast<float>(m_ScreenWidth),
		static_cast<float>(m_ScreenHeight),
		m_shadowViewport);
	
	// 占쏙옙占쏙옙 占쏙옙占쌕쏙옙 占쏙옙 占쏙옙占쏙옙
	RendererHelper::CreateDepthStencilBuffer(m_pDevice, m_numQualityLevels, m_endScreenWidth, m_endScreenHeight, mpDepthStencilView);

	//占쏙옙占쏙옙占?占쏙옙占쏙옙 타占쏙옙
	RendererHelper::CreateRenderTargetView(m_pDevice, mpSwapChain, m_pTempTargetview, mpTempBuffer, 1, 1920 * 0.5, 1080 * 0.5);
	hr = m_pDevice->CreateShaderResourceView(mpTempBuffer.Get(), nullptr, mpTempTargetSRV.GetAddressOf());

	if (!CreateTwoDimensionalShadowArray())
	{
		return false;
	}

	if (!CreatePointShadowCube())
	{
		return false;
	}
	return true;
}

bool Renderer::CreateTwoDimensionalShadowArray()
{
	// Directional과 Spot은 모두 2D Depth Map을 사용한다.
	// 각 라이트마다 Texture를 따로 만들 수도 있지만 하나의 Texture2DArray로 묶으면
	// 바인딩 슬롯 하나(t15)만 사용하면서 lightIndex로 원하는 slice를 선택할 수 있다.
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = m_ScreenWidth;
	textureDesc.Height = m_ScreenHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = MAX_LIGHTS;
	textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	if (FAILED(m_pDevice->CreateTexture2D(
		&textureDesc, nullptr, m_twoDimensionalShadowArrayTexture.GetAddressOf())))
	{
		DEBUG_LOG("ERROR - 2D Shadow Array texture creation failed.");
		return false;
	}

	// Shadow Pass에서는 한 라이트의 slice만 Depth Target으로 묶어야 하므로
	// slice마다 ArraySize가 1인 DSV를 하나씩 만든다.
	for (UINT lightIndex = 0; lightIndex < m_twoDimensionalShadowDSVs.size(); ++lightIndex)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		dsvDesc.Texture2DArray.MipSlice = 0;
		dsvDesc.Texture2DArray.FirstArraySlice = lightIndex;
		dsvDesc.Texture2DArray.ArraySize = 1;
		if (FAILED(m_pDevice->CreateDepthStencilView(
			m_twoDimensionalShadowArrayTexture.Get(),
			&dsvDesc,
			m_twoDimensionalShadowDSVs[lightIndex].GetAddressOf())))
		{
			DEBUG_LOG("ERROR - 2D Shadow Array DSV creation failed.");
			return false;
		}
	}

	// 메인 조명 패스에서는 배열 전체를 한 번에 읽고, HLSL의 세 번째 좌표로 slice를 고른다.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = 1;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.ArraySize = MAX_LIGHTS;
	if (FAILED(m_pDevice->CreateShaderResourceView(
		m_twoDimensionalShadowArrayTexture.Get(),
		&srvDesc,
		m_twoDimensionalShadowArraySRV.GetAddressOf())))
	{
		DEBUG_LOG("ERROR - 2D Shadow Array SRV creation failed.");
		return false;
	}

	return true;
}

bool Renderer::CreatePointShadowCube()
{
	// Cube Array도 실제 저장 형태는 Texture2D 배열이다.
	// Point Light 하나당 여섯 slice를 연속으로 배치하고 MAX_LIGHTS개 분량을 미리 확보한다.
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = m_pointShadowMapSize;
	textureDesc.Height = m_pointShadowMapSize;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = MAX_LIGHTS * 6;
	textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	if (FAILED(m_pDevice->CreateTexture2D(
		&textureDesc, nullptr, m_pointShadowCubeTexture.GetAddressOf())))
	{
		DEBUG_LOG("ERROR - Point Shadow Cube texture creation failed.");
		return false;
	}

	// DSV 하나는 Point Light 한 개의 한 방향만 가리킨다.
	// FirstArraySlice는 lightIndex * 6 + faceIndex와 동일한 선형 slice 번호다.
	for (UINT arraySlice = 0; arraySlice < m_pointShadowCubeDSVs.size(); ++arraySlice)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		dsvDesc.Texture2DArray.MipSlice = 0;
		dsvDesc.Texture2DArray.FirstArraySlice = arraySlice;
		dsvDesc.Texture2DArray.ArraySize = 1;
		if (FAILED(m_pDevice->CreateDepthStencilView(
			m_pointShadowCubeTexture.Get(),
			&dsvDesc,
			m_pointShadowCubeDSVs[arraySlice].GetAddressOf())))
		{
			DEBUG_LOG("ERROR - Point Shadow Cube face DSV creation failed.");
			return false;
		}
	}

	// 렌더가 끝난 뒤에는 전체 배열을 TextureCubeArray로 읽는다.
	// 셰이더는 방향 벡터와 lightIndex를 함께 넘겨 Cube와 면을 동시에 선택한다.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
	srvDesc.TextureCubeArray.MostDetailedMip = 0;
	srvDesc.TextureCubeArray.MipLevels = 1;
	srvDesc.TextureCubeArray.First2DArrayFace = 0;
	srvDesc.TextureCubeArray.NumCubes = MAX_LIGHTS;
	if (FAILED(m_pDevice->CreateShaderResourceView(
		m_pointShadowCubeTexture.Get(),
		&srvDesc,
		m_pointShadowCubeSRV.GetAddressOf())))
	{
		DEBUG_LOG("ERROR - Point Shadow Cube SRV creation failed.");
		return false;
	}

	m_pointShadowViewport.TopLeftX = 0.0f;
	m_pointShadowViewport.TopLeftY = 0.0f;
	m_pointShadowViewport.Width = static_cast<float>(m_pointShadowMapSize);
	m_pointShadowViewport.Height = static_cast<float>(m_pointShadowMapSize);
	m_pointShadowViewport.MinDepth = 0.0f;
	m_pointShadowViewport.MaxDepth = 1.0f;
	return true;
}

void Renderer::SetPipelineState(const PipelineStateObject& _pso)
{
	m_pDeviceContext->VSSetShader(_pso.m_pVertexShader.Get(), 0, 0);
	m_pDeviceContext->PSSetShader(_pso.m_pPixelShader.Get(), 0, 0);
	m_pDeviceContext->IASetInputLayout(_pso.m_pInputLayout.Get());
	m_pDeviceContext->RSSetState(_pso.m_pRasterizerState.Get());
	m_pDeviceContext->OMSetBlendState(_pso.m_pBlendState.Get(), _pso.m_blendFactor, 0xffffffff);		 //0xffffffff - 占쏙옙占?占쏙옙占쏙옙占쏙옙 활占쏙옙화 占쏙옙키占쌘댐옙.
	m_pDeviceContext->OMSetDepthStencilState(_pso.m_pDepthStencilState.Get(), _pso.m_stencilRef);
	m_pDeviceContext->IASetPrimitiveTopology(_pso.m_primitiveTopology);
	//m_pDeviceContext->PSSetSamplers(0, 1, _pso.m_pSamplerState.GetAddressOf());
}

void Renderer::LightUpdate(CommonConstantBufferData* _psBufferData)
{
	
}

void Renderer::UpdateCommonConstantBuffer(CommonConstantBufferData& _CommonBufferData)
{
	RendererHelper::UpdateBuffer(m_pDeviceContext, _CommonBufferData, mpCommonConstantBuffer);

	// 占쏙옙占싱댐옙占쏙옙 占싹곤옙占쏙옙 占쏙옙占쏙옙 register(b1)***
	m_pDeviceContext->VSSetConstantBuffers(1, 1, mpCommonConstantBuffer.GetAddressOf());
	m_pDeviceContext->PSSetConstantBuffers(1, 1, mpCommonConstantBuffer.GetAddressOf());
}
int a = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;

void Renderer::RenderDepthMap(ModelBuffer* _modelbuffer)
{
	SetPipelineState(Dears::Graphics::depthOnlyPSO);
	// BeginTwoDimensionalShadowPass/BeginPointShadowFace가 고른 현재 Depth Target을 사용한다.
	// 활성 타깃이 없으면 기존 2D 맵을 기본값으로 사용해 과거 호출 경로도 안전하게 유지한다.
	ID3D11DepthStencilView* shadowDSV = m_activeShadowDSV
		? m_activeShadowDSV
		: m_twoDimensionalShadowDSVs[0].Get();
	const D3D11_VIEWPORT* shadowViewport = m_activeShadowViewport
		? m_activeShadowViewport
		: &m_shadowViewport;
	m_pDeviceContext->RSSetViewports(1, shadowViewport);
	m_pDeviceContext->OMSetRenderTargets(0, nullptr, shadowDSV);


	// 占쏙옙占쌔쏙옙/占싸듸옙占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, _modelbuffer->m_pVertexBuffer.GetAddressOf(), &stride, &offset);
	m_pDeviceContext->IASetIndexBuffer(_modelbuffer->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	m_pDeviceContext->VSSetConstantBuffers(0, 1, _modelbuffer->m_pVSConstantBuffer.GetAddressOf());

	m_pDeviceContext->PSSetConstantBuffers(0, 1, _modelbuffer->m_pPSConstantBuffer.GetAddressOf());

	ID3D11ShaderResourceView* pixelResources[1] =
	{
		_modelbuffer->m_diffusetexture.Get(),
	};
	m_pDeviceContext->PSSetShaderResources(0, 1, pixelResources);


	m_pDeviceContext->DrawIndexed(_modelbuffer->mNumIndices, 0, 0);


	m_pDeviceContext->RSSetViewports(1, &m_pD3dScreenViewport);

	m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), mpDepthStencilView.Get());

}

void Renderer::RenderPbrDepthMap(ModelBuffer* _modelbuffer)
{
	// MeshRenderer가 Height 변형용 PBR Shadow PSO를 먼저 설정한다.
	// 여기서는 현재 라이트의 Shadow DSV를 선택하고 PBR 정점 리소스를 바인딩해 Draw만 수행한다.
	ID3D11DepthStencilView* shadowDSV = m_activeShadowDSV
		? m_activeShadowDSV
		: m_twoDimensionalShadowDSVs[0].Get();
	const D3D11_VIEWPORT* shadowViewport = m_activeShadowViewport
		? m_activeShadowViewport
		: &m_shadowViewport;
	m_pDeviceContext->RSSetViewports(1, shadowViewport);
	m_pDeviceContext->OMSetRenderTargets(0, nullptr, shadowDSV);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(
		0, 1, _modelbuffer->m_pVertexBuffer.GetAddressOf(), &stride, &offset);
	m_pDeviceContext->IASetIndexBuffer(
		_modelbuffer->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// PBRVertexShader와 PBRShadowVertexShader는 같은 b6 구조를 공유한다.
	// useHeightMap과 heightScale을 그대로 읽기 때문에 화면과 그림자의 변형 크기가 일치한다.
	m_pDeviceContext->VSSetConstantBuffers(
		6, 1, _modelbuffer->m_VSPBRConstantBuffer.GetAddressOf());
	ID3D11ShaderResourceView* heightResource = _modelbuffer->heightTex.Get();
	m_pDeviceContext->VSSetShaderResources(0, 1, &heightResource);

	m_pDeviceContext->DrawIndexed(_modelbuffer->mNumIndices, 0, 0);

	// 다음 Shadow Item이 일반 StaticMesh일 수 있으므로 Height Texture 바인딩을 명시적으로 지운다.
	// 전용 PSO로 경로를 나눴지만 리소스 상태도 정리해 두면 이후 RHI 전환 시 의존성이 더 선명하다.
	ID3D11ShaderResourceView* nullResource = nullptr;
	m_pDeviceContext->VSSetShaderResources(0, 1, &nullResource);
	m_pDeviceContext->RSSetViewports(1, &m_pD3dScreenViewport);
	m_pDeviceContext->OMSetRenderTargets(
		1, m_pRenderTargetView.GetAddressOf(), mpDepthStencilView.Get());
}

void Renderer::RenderAniDepthMap(ModelBuffer* _modelbuffer)
{
	SetPipelineState(Dears::Graphics::depthAniOnlyPSO);
	ID3D11DepthStencilView* shadowDSV = m_activeShadowDSV
		? m_activeShadowDSV
		: m_twoDimensionalShadowDSVs[0].Get();
	const D3D11_VIEWPORT* shadowViewport = m_activeShadowViewport
		? m_activeShadowViewport
		: &m_shadowViewport;
	m_pDeviceContext->RSSetViewports(1, shadowViewport);
	m_pDeviceContext->OMSetRenderTargets(0, nullptr, shadowDSV);


	// 占쏙옙占쌔쏙옙/占싸듸옙占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, _modelbuffer->m_pVertexBuffer.GetAddressOf(), &stride, &offset);
	m_pDeviceContext->IASetIndexBuffer(_modelbuffer->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	m_pDeviceContext->VSSetConstantBuffers(0, 1, _modelbuffer->m_pVSConstantBuffer.GetAddressOf());
	m_pDeviceContext->VSSetConstantBuffers(2, 1, _modelbuffer->m_BoneConstantBuffer.GetAddressOf());

	m_pDeviceContext->PSSetConstantBuffers(0, 1, _modelbuffer->m_pPSConstantBuffer.GetAddressOf());

	ID3D11ShaderResourceView* pixelResources[1] =
	{
		_modelbuffer->m_diffusetexture.Get(),
	};
	m_pDeviceContext->PSSetShaderResources(0, 1, pixelResources);


	m_pDeviceContext->DrawIndexed(_modelbuffer->mNumIndices, 0, 0);


	m_pDeviceContext->RSSetViewports(1, &m_pD3dScreenViewport);
	m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), mpDepthStencilView.Get());

}

void Renderer::RenderEquipDepthMap(ModelBuffer* _modelbuffer)
{
	SetPipelineState(Dears::Graphics::depthEquipOnlyPSO);
	ID3D11DepthStencilView* shadowDSV = m_activeShadowDSV
		? m_activeShadowDSV
		: m_twoDimensionalShadowDSVs[0].Get();
	const D3D11_VIEWPORT* shadowViewport = m_activeShadowViewport
		? m_activeShadowViewport
		: &m_shadowViewport;
	m_pDeviceContext->RSSetViewports(1, shadowViewport);
	m_pDeviceContext->OMSetRenderTargets(0, nullptr, shadowDSV);


	// 占쏙옙占쌔쏙옙/占싸듸옙占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, _modelbuffer->m_pVertexBuffer.GetAddressOf(), &stride, &offset);
	m_pDeviceContext->IASetIndexBuffer(_modelbuffer->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	m_pDeviceContext->VSSetConstantBuffers(0, 1, _modelbuffer->m_pVSConstantBuffer.GetAddressOf());
	m_pDeviceContext->VSSetConstantBuffers(3, 1, _modelbuffer->m_TargetBoneConstantBuffer.GetAddressOf());

	m_pDeviceContext->PSSetConstantBuffers(0, 1, _modelbuffer->m_pPSConstantBuffer.GetAddressOf());

	ID3D11ShaderResourceView* pixelResources[1] =
	{
		_modelbuffer->m_diffusetexture.Get(),
	};
	m_pDeviceContext->PSSetShaderResources(0, 1, pixelResources);


	m_pDeviceContext->DrawIndexed(_modelbuffer->mNumIndices, 0, 0);


	m_pDeviceContext->RSSetViewports(1, &m_pD3dScreenViewport);
	//m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), mpDepthStencilView.Get());
	m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), mpDepthStencilView.Get());

}

void Renderer::RenderEdge(ModelBuffer* _modelbuffer)
{
	// 占쏙옙占쌔쏙옙/占싸듸옙占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pDeviceContext->RSSetViewports(1, &m_pD3dScreenViewport);
	m_pDeviceContext->IASetVertexBuffers(0, 1, _modelbuffer->m_pVertexBuffer.GetAddressOf(), &stride, &offset);
	m_pDeviceContext->IASetIndexBuffer(_modelbuffer->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_pDeviceContext->VSSetConstantBuffers(0, 1, _modelbuffer->m_pVSConstantBuffer.GetAddressOf());
	m_pDeviceContext->VSSetConstantBuffers(4, 1, _modelbuffer->m_pVSEdgeConstantBuffer.GetAddressOf());
	m_pDeviceContext->PSSetConstantBuffers(2, 1, _modelbuffer->m_pPSEdgeConstantBuffer.GetAddressOf());
	m_pDeviceContext->DrawIndexed(_modelbuffer->mNumIndices, 0, 0);


}

void Renderer::RenderSampler(ModelBuffer* _modelbuffer)
{
	// 占쏙옙占쌔쏙옙/占싸듸옙占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, _modelbuffer->m_pVertexBuffer.GetAddressOf(), &stride, &offset);
	m_pDeviceContext->IASetIndexBuffer(_modelbuffer->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	m_pDeviceContext->RSSetViewports(1, &m_pD3dtempViewport);

	m_pDeviceContext->OMSetRenderTargets(1, m_pTempTargetview.GetAddressOf(), nullptr);
	m_pDeviceContext->PSSetShaderResources(0, 1, mpMainRenderTargetSRV.GetAddressOf());

	m_pDeviceContext->DrawIndexed(_modelbuffer->mNumIndices, 0, 0);
 
// 	m_pDeviceContext->RSSetViewports(1, &m_pD3dScreenViewport);
// 	m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), nullptr);
// 	m_pDeviceContext->PSSetShaderResources(0, 1, mpTempTargetSRV.GetAddressOf());
// 	m_pDeviceContext->DrawIndexed(_modelbuffer->mNumIndices, 0, 0);
}

/////
void Renderer::RenderPostProcessing(ModelBuffer* _modelbuffer)
{
	// 占쏙옙占쌔쏙옙/占싸듸옙占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, _modelbuffer->m_pVertexBuffer.GetAddressOf(), &stride, &offset);
	m_pDeviceContext->IASetIndexBuffer(_modelbuffer->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	
	m_pDeviceContext->RSSetViewports(1, &m_pD3dScreenViewport);

	m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), nullptr);
	m_pDeviceContext->PSSetShaderResources(0, 1, mpTempTargetSRV.GetAddressOf());

	m_pDeviceContext->DrawIndexed(_modelbuffer->mNumIndices, 0, 0);

// 	m_pDeviceContext->RSSetViewports(1, &m_pD3dScreenViewport);
// 	m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), nullptr);
// 	m_pDeviceContext->PSSetShaderResources(0, 1, mpTempTargetSRV.GetAddressOf());
// 	m_pDeviceContext->DrawIndexed(_modelbuffer->mNumIndices, 0, 0);

}

//占싸쏙옙占싹쏙옙 占쏙옙占쏙옙 占쏙옙占?vertexConstantBuffer占쌤울옙占쏙옙 instanceConstantBuffer占쏙옙 GPU占쏙옙 占쏙옙占쏙옙占쌍억옙占쏙옙磯占?
void Renderer::Rend_InstancedModels(ModelBuffer* _modelBuffers)
{
		/// 占싸쏙옙占싹쏙옙 占쏙옙占쏙옙 占쏙옙占?VertexConstantBuffer 占쌤울옙占쏙옙 InstanceConstantBuffer占쏙옙 GPU占쏙옙 占쏙옙占쏙옙占쌍억옙占?占싼댐옙.

	// 1. 占쏙옙占쌔쏙옙/占싸듸옙占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙
		UINT stride = sizeof(Vertex); // 占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙占쏙옙 크占쏙옙
		UINT offset = 0;
		m_pDeviceContext->IASetVertexBuffers(0, 1, _modelBuffers->m_pVertexBuffer.GetAddressOf(), &stride, &offset); // 占쏙옙占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙
		m_pDeviceContext->IASetIndexBuffer(_modelBuffers->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0); // 占싸듸옙占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙

		// 3. 占싸쏙옙占싹쏙옙 占쏙옙占?占쏙옙占쏙옙 占쌩곤옙
		// 占싸쏙옙占싹쏙옙 占쏙옙占쏙옙占싶곤옙 占쏙옙占쏙옙獵占?占쏙옙占?占쏙옙占쌜몌옙 占쏙옙占쏙옙占쌔억옙 占쏙옙
		m_pDeviceContext->VSSetConstantBuffers(0, 1, _modelBuffers->m_InstanceConstantBuffer.GetAddressOf()); // Instance Constant Buffer (占싸쏙옙占싹쏙옙 占쏙옙占쏙옙)

		// 4. 占싫쇽옙 占쏙옙占싱댐옙 占쏙옙占?占쏙옙占쏙옙
		m_pDeviceContext->PSSetConstantBuffers(0, 1, _modelBuffers->m_pPSConstantBuffer.GetAddressOf());

		// 5. 占쏙옙占싱댐옙 占쏙옙占쌀쏙옙 (占쌔쏙옙처)
		ID3D11ShaderResourceView* pixelResources[1] =
		{
			_modelBuffers->m_diffusetexture.Get(),
		};
		m_pDeviceContext->PSSetShaderResources(0, 1, pixelResources); // 占쌔쏙옙처 占쏙옙占쏙옙

		// 6. 占싸쏙옙占싹쏙옙 占쏙옙恝占?호占쏙옙
		// 占쏙옙占쏙옙 DrawIndexed 占쏙옙占?DrawIndexedInstanced占쏙옙 占쏙옙占쏙옙臼占?占싸쏙옙占싹쏙옙占쏙옙 占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙 占쏙옙 占쏙옙占쏙옙占쏙옙
		UINT instanceCount = _modelBuffers->mNumInstances; // 占싸쏙옙占싹쏙옙 占쏙옙占쏙옙
		m_pDeviceContext->DrawIndexedInstanced(_modelBuffers->mNumIndices, instanceCount, 0, 0, 0);

		// 7. 占쏙옙占쏙옙트 占쏙옙 占쏙옙占쏙옙 타占쏙옙 占쏙옙占쏙옙
		//m_pDeviceContext->RSSetViewports(1, &m_pD3dScreenViewport);
		//m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), mpDepthStencilView.Get());

}

