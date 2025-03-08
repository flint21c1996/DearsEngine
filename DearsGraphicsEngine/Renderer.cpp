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
	delete mpTargetCamera;
	//delete m_pResourceManager;
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
	// ���� ���� ����
	m_pRenderHelper = new RendererHelper();


	CommonConstantBufferData CCBD;	//mpCommonConstantBuffer�� �ʱ�ȭ ��Ű�� ���� ����
	mpCommonConstantBuffer = RendererHelper::CreateConstantBuffer(m_pDevice, CCBD);
	return true;
}

void Renderer::BeginRender()
{
	// �������� ����Ʈ�� �����Ѵ�. (1����, m_pD3dScreenViewport ����)
	m_pDeviceContext->RSSetViewports(1, &m_pD3dScreenViewport);

	float clearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f }; // ����� ��
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), clearColor); // renderTargetView�� clearColor�� �� �����.

	float clearColor1[4] = { 0.f, 0.f, 0.f, 0.0f }; // ����� ��
	m_pDeviceContext->ClearRenderTargetView(m_pTempTargetview.Get(), clearColor1); // renderTargetView�� clearColor�� �� �����.

	//��� ���÷����� �������� ���
	m_pDeviceContext->VSSetSamplers(0, UINT(Dears::Graphics::sampleStates.size()),
		Dears::Graphics::sampleStates.data());
	m_pDeviceContext->PSSetSamplers(0, UINT(Dears::Graphics::sampleStates.size()),
		Dears::Graphics::sampleStates.data());

	// ����-���ĽǺ� ����
	m_pDeviceContext->ClearDepthStencilView(mpDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_pDeviceContext->ClearDepthStencilView(m_depthOnlyDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), mpDepthStencilView.Get());


	//ť��� ���ҽ��� GPU�� �̸� �������´�.
	SetCommonShaderResourceToGPU();
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
	// ���̴��� �ϰ��� ���� register(b1)***
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
	return true;
}

void Renderer::Render(ModelBuffer* _modelbuffer)
{

	// ���ؽ�/�ε��� ���� ����
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
	m_pDeviceContext->PSSetConstantBuffers(2, 1, _modelbuffer->m_PSPBRConstantBuffer.GetAddressOf());	//������ ����..?
	m_pDeviceContext->PSSetConstantBuffers(3, 1, _modelbuffer->m_PSThinFilmConstantBuffer.GetAddressOf());	//������ ����..?

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

	//m_pDeviceContext->RSSetViewports(1, &m_pD3dScreenViewport);
	m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), mpDepthStencilView.Get());

}

void Renderer::Render_CubeMap(ModelBuffer* _modelbuffer)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	// ť�����
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
	// ���� ü�� ����
	RendererHelper::CreateSwapChain(m_pDevice, m_hWnd, m_numQualityLevels, m_endScreenWidth, m_endScreenHeight, mpSwapChain);
	 
	// ���� ���� Ÿ�� �� ����
	RendererHelper::CreateRenderTargetView(m_pDevice, mpSwapChain, m_pRenderTargetView, mpBackBuffer);
	HRESULT hr = m_pDevice->CreateShaderResourceView(mpBackBuffer.Get(), nullptr, mpMainRenderTargetSRV.GetAddressOf());

	// �⺻ �� ��Ʈ ����
	RendererHelper::SetViewPort(m_pDeviceContext, m_startScreenWidth, m_startScreenHeight, m_endScreenWidth, m_endScreenHeight, m_pD3dScreenViewport);
	// ����� �� ��Ʈ
	RendererHelper::SetViewPort(m_pDeviceContext, 0, 0, 1920 *0.5, 1080 * 0.5, m_pD3dtempViewport);
	
	// ���� ���ٽ� �� ����
	RendererHelper::CreateDepthStencilBuffer(m_pDevice, m_numQualityLevels, m_endScreenWidth, m_endScreenHeight, mpDepthStencilView);

	//����� ���� Ÿ��
	RendererHelper::CreateRenderTargetView(m_pDevice, mpSwapChain, m_pTempTargetview, mpTempBuffer, 1, 1920 * 0.5, 1080 * 0.5);
	hr = m_pDevice->CreateShaderResourceView(mpTempBuffer.Get(), nullptr, mpTempTargetSRV.GetAddressOf());

	RendererHelper::CreateDepthOnlyBuffer(m_pDevice, m_ScreenWidth, m_ScreenHeight, m_depthOnlyDSV, m_depthOnlySRV);
	return true;
}

void Renderer::SetPipelineState(const PipelineStateObject& _pso)
{
	m_pDeviceContext->VSSetShader(_pso.m_pVertexShader.Get(), 0, 0);
	m_pDeviceContext->PSSetShader(_pso.m_pPixelShader.Get(), 0, 0);
	m_pDeviceContext->IASetInputLayout(_pso.m_pInputLayout.Get());
	m_pDeviceContext->RSSetState(_pso.m_pRasterizerState.Get());
	m_pDeviceContext->OMSetBlendState(_pso.m_pBlendState.Get(), _pso.m_blendFactor, 0xffffffff);		 //0xffffffff - ��� ������ Ȱ��ȭ ��Ű�ڴ�.
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

	// ���̴��� �ϰ��� ���� register(b1)***
	m_pDeviceContext->VSSetConstantBuffers(1, 1, mpCommonConstantBuffer.GetAddressOf());
	m_pDeviceContext->PSSetConstantBuffers(1, 1, mpCommonConstantBuffer.GetAddressOf());
}
int a = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;

void Renderer::RenderDepthMap(ModelBuffer* _modelbuffer)
{
	SetPipelineState(Dears::Graphics::depthOnlyPSO);
//	m_pDeviceContext->RSSetViewports(1, &m_pD3dtempViewport);
	m_pDeviceContext->RSSetViewports(1, &m_pD3dScreenViewport);
	
	//ȭ�鿡 ����, -> �밭 Ȯ���Ҽ� �ִ�
	//m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_depthOnlyDSV.Get());
	
	m_pDeviceContext->OMSetRenderTargets(0,NULL, m_depthOnlyDSV.Get());
	//�ٸ� ����Ÿ�ٿ� �׸���
	//m_pDeviceContext->OMSetRenderTargets(1, m_pTempRednerTargetview.GetAddressOf(), m_depthOnlyDSV.Get());


	// ���ؽ�/�ε��� ���� ����
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

	m_pDeviceContext->PSSetShaderResources(15, 1, m_depthOnlySRV.GetAddressOf());	  //(�ؽ����� ��ȣ(�ε���), SRV�� ����, ������(�ּҰ�))
}

void Renderer::RenderAniDepthMap(ModelBuffer* _modelbuffer)
{
	SetPipelineState(Dears::Graphics::depthAniOnlyPSO);
	//	m_pDeviceContext->RSSetViewports(1, &m_pD3dtempViewport);
	m_pDeviceContext->RSSetViewports(1, &m_pD3dScreenViewport);

	//ȭ�鿡 ����, -> �밭 Ȯ���Ҽ� �ִ�
	//m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_depthOnlyDSV.Get());

	m_pDeviceContext->OMSetRenderTargets(0, NULL, m_depthOnlyDSV.Get());
	//�ٸ� ����Ÿ�ٿ� �׸���
	//m_pDeviceContext->OMSetRenderTargets(1, m_pTempRednerTargetview.GetAddressOf(), m_depthOnlyDSV.Get());


	// ���ؽ�/�ε��� ���� ����
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

	m_pDeviceContext->PSSetShaderResources(15, 1, m_depthOnlySRV.GetAddressOf());	  //(�ؽ����� ��ȣ(�ε���), SRV�� ����, ������(�ּҰ�))
}

void Renderer::RenderEquipDepthMap(ModelBuffer* _modelbuffer)
{
	SetPipelineState(Dears::Graphics::depthEquipOnlyPSO);
	//	m_pDeviceContext->RSSetViewports(1, &m_pD3dtempViewport);
	m_pDeviceContext->RSSetViewports(1, &m_pD3dScreenViewport);

	//ȭ�鿡 ����, -> �밭 Ȯ���Ҽ� �ִ�
	//m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_depthOnlyDSV.Get());

	m_pDeviceContext->OMSetRenderTargets(0, NULL, m_depthOnlyDSV.Get());
	//�ٸ� ����Ÿ�ٿ� �׸���
	//m_pDeviceContext->OMSetRenderTargets(1, m_pTempRednerTargetview.GetAddressOf(), m_depthOnlyDSV.Get());


	// ���ؽ�/�ε��� ���� ����
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

	m_pDeviceContext->PSSetShaderResources(15, 1, m_depthOnlySRV.GetAddressOf());	  //(�ؽ����� ��ȣ(�ε���), SRV�� ����, ������(�ּҰ�))
}

void Renderer::RenderEdge(ModelBuffer* _modelbuffer)
{
	// ���ؽ�/�ε��� ���� ����
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
	// ���ؽ�/�ε��� ���� ����
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
	// ���ؽ�/�ε��� ���� ����
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

//�ν��Ͻ� ���� ��� vertexConstantBuffer�ܿ��� instanceConstantBuffer�� GPU�� �����־���Ѵ�.
void Renderer::Rend_InstancedModels(ModelBuffer* _modelBuffers)
{
		/// �ν��Ͻ� ���� ��� VertexConstantBuffer �ܿ��� InstanceConstantBuffer�� GPU�� �����־�� �Ѵ�.

	// 1. ���ؽ�/�ε��� ���� ����
		UINT stride = sizeof(Vertex); // ���� �������� ũ��
		UINT offset = 0;
		m_pDeviceContext->IASetVertexBuffers(0, 1, _modelBuffers->m_pVertexBuffer.GetAddressOf(), &stride, &offset); // ���� ���� ����
		m_pDeviceContext->IASetIndexBuffer(_modelBuffers->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0); // �ε��� ���� ����

		// 3. �ν��Ͻ� ��� ���� �߰�
		// �ν��Ͻ� �����Ͱ� ����ִ� ��� ���۸� �����ؾ� ��
		m_pDeviceContext->VSSetConstantBuffers(0, 1, _modelBuffers->m_InstanceConstantBuffer.GetAddressOf()); // Instance Constant Buffer (�ν��Ͻ� ����)

		// 4. �ȼ� ���̴� ��� ����
		m_pDeviceContext->PSSetConstantBuffers(0, 1, _modelBuffers->m_pPSConstantBuffer.GetAddressOf());

		// 5. ���̴� ���ҽ� (�ؽ�ó)
		ID3D11ShaderResourceView* pixelResources[1] =
		{
			_modelBuffers->m_diffusetexture.Get(),
		};
		m_pDeviceContext->PSSetShaderResources(0, 1, pixelResources); // �ؽ�ó ����

		// 6. �ν��Ͻ� ��ο� ȣ��
		// ���� DrawIndexed ��� DrawIndexedInstanced�� ����Ͽ� �ν��Ͻ��� �� ���� ���� �� ������
		UINT instanceCount = _modelBuffers->mNumInstances; // �ν��Ͻ� ����
		m_pDeviceContext->DrawIndexedInstanced(_modelBuffers->mNumIndices, instanceCount, 0, 0, 0);

		// 7. ����Ʈ �� ���� Ÿ�� ����
		//m_pDeviceContext->RSSetViewports(1, &m_pD3dScreenViewport);
		//m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), mpDepthStencilView.Get());

}

