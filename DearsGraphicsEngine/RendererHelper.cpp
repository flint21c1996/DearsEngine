#include<iostream>
#include "RendererHelper.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")
#include "stb_image.h"

RendererHelper::RendererHelper()
{

}

RendererHelper::~RendererHelper()
{

}

bool RendererHelper::CreateDevice(ComPtr<ID3D11Device>& _pDevice, ComPtr<ID3D11DeviceContext>& _pDeviceContext)
{
	UINT createDeviceFlags = 0;											//
	const D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;		//����̹� Ÿ���� ���Ѵ�.

#if defined(DEBUG) || defined(_DEBUG)  
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	const D3D_FEATURE_LEVEL featureLevels[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_9_3 };
	D3D_FEATURE_LEVEL featureLevel;

	//����̽��� ����
	if (FAILED(D3D11CreateDevice(
		nullptr,										//����̽��� ���鶧 ����� ���� �����, �⺻����͸� ���� NULL�� ����
		driverType,										//���� ����̹� ����
		0,												//����Ʈ���� �����Ͷ������� �����ϴ� DLL�� ���� �ڵ�
		createDeviceFlags,								//����̽��� ������ �� ����C �÷��׸� �����ϴ� ����, 0 = �⺻������ �ƹ� �÷��׵� �������� �ʴ´�.
		featureLevels,									//������� ��� ������ ������ �����ϴ� D3D_FEATURE_LEVEL�� ���� ������
		ARRAYSIZE(featureLevels),						//featureLevels�� ��� ��
		D3D11_SDK_VERSION,								//D3D11_CREATE_DEVICE_FLAG ������
		&_pDevice,										//���� ����̽��� ��Ÿ���� ID3D11Device ��ü�� ���� �������� �ּ�, �̰����� ��ȯ�Ѵ�.
		&featureLevel,									//������ ��� featureLevels �迭���� ù��° D3D_FEATURE_LEVEL ��ȯ, �����Ǵ� ��� ������ ������ �ʿ䰡 ���°�� NULL�� �Է����� ����
		&_pDeviceContext								//���̽� ���ؽ�Ʈ�� ��Ÿ���� ID3D11DeviceContext ��ü�� ���� �������� �ּ�
	)))
	{
		std::cout << "D3D11CreateDevice() failed." << std::endl;
		return false;
	}
	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		std::cout << "D3D Feature Level 11 unsupported." << std::endl;
		return false;
	}

	return true;
}

bool RendererHelper::CreateSwapChain(ComPtr<ID3D11Device>& _pDevice,
	HWND _hWnd, UINT& _numQualityLevels, int _screenWidth, int _screenHeight, ComPtr<IDXGISwapChain>& _pSwapChain)
{
	// 4X MSAA �����ϴ��� Ȯ��, ��Ƽ���ø��� ��ܰ���� �Ǵ°�?
	HRESULT hr = _pDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &_numQualityLevels);
	if (FAILED(hr) || _numQualityLevels <= 0)
	{
		std::cout << "MSAA not supported." << std::endl;
		_numQualityLevels = 0; // MSAA ������ ���� -> ���۵� �������� ���� ü���� �� ����غ���.
	}

	//����ü�� - ���߹��۸�
	DXGI_SWAP_CHAIN_DESC sd;												//����ü���� ������ ����ִ� ����ü
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferDesc.Width = _screenWidth;										//�ػ� ���� - ������� ũ��, DXGI_MODE_DESC�� ������ ä��� ���̴�.
	sd.BufferDesc.Height = _screenHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;						//������� ����԰�
	sd.BufferCount = 1;														//������� ����
	sd.BufferDesc.RefreshRate.Numerator = 0xffffffff;						//������ ���Ѱ��� Ǯ�����. //���� : �ϴ� 60���������� ������ �Ѵ�. ��������ȭ�� ����ߴٰ� ��������. �̺κ��� 0���� �����Ѵٸ� �ü���� ����̹��� ������� �⺻ȭ�� ���ΰ�ħ�󵵸� ������� ������ ���� �����Ѵ�.
	sd.BufferDesc.RefreshRate.Denominator = 1;								//ȭ�� ��ħ ���� �и� �����ϴ� �κ��̴�. numerator�� �Բ� ���Ǿ� ȭ�� ���ΰ�ħ �󵵸� ��Ÿ����.
	sd.BufferUsage = DXGI_USAGE_SHADER_INPUT |								//��ó�� ���� ������ ����
						DXGI_USAGE_RENDER_TARGET_OUTPUT;					//BufferUsage = � �뵵�� ����� ���ΰ�? /�� ������ ǥ�� ��뷮 �� ������ �ɼ��� �����ϴ� DXGI_USAGE ������ ������ ���, ���⼭�� RT0����Ÿ���� �׸��� �׸�������.
	sd.OutputWindow = _hWnd;												//����� �������� �ڵ�
	sd.Windowed = TRUE;														//��üȭ���� ����� ���ΰ�? /â���
	//sd.Windowed = FALSE;													//��üȭ�� ���
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	if (_numQualityLevels > 0)
	{
		sd.SampleDesc.Count = 4; // how many multisamples, �̹����� ǰ�� ����, ǰ���� ���� ���� ������ �������� ������.. 
		sd.SampleDesc.Quality = _numQualityLevels - 1;
	}
	//No MSAA
	else
	{
		sd.SampleDesc.Count = 1; // how many multisamples, ��Ƽ�ٸ������ ���� �⺻ ���ø� ����� ������ 1�̰� ǰ�� ������ 0�̴�. /��Ƽ�ٸ������ �Դ������� ���̱����� ����̴�. �� - �� �ȼ��ȿ� �������� ������ �Ի��� ����� ���� ��Ƽ���ø�
		sd.SampleDesc.Quality = 0;
	}

	// IDXGIFactory�� �̿��� CreateSwapChain()----------------------------------------------------------------------------------------
	//D3D11CreateDeviceAndSwapChain�̶�� �Լ��� �̿��ؼ� ����̽��� ����ü���� �ѹ��� ���굵 �����ϴ� -> �� �Լ��� ���������� DXGIFactory�� ������ �ֱ� �����̴�. 
	ComPtr<IDXGIDevice3> dxgiDevice;		//DXGI Device
	hr = _pDevice.As(&dxgiDevice);
	if (FAILED(hr))
	{
		std::cout << "Failed to get DXGI device." << std::endl;
		return false;
	}

	ComPtr<IDXGIAdapter> dxgiAdapter;		//DXGI ����� �������̽� �����͸� ����, ����ʹ� �׷��� ī�峪 ���� ī�带 �ǹ�
	hr = dxgiDevice->GetAdapter(&dxgiAdapter);
	if (FAILED(hr))
	{
		std::cout << "Failed to get DXGI adapter." << std::endl;
		return false;
	}

	ComPtr<IDXGIFactory> dxgiFactory;		//DXGI ���丮 �������̽� �����͸� ����, DXGI ���丮�� �پ��� DXGI ��ü�� �����ϴ� �� ���
	hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(hr))
	{
		std::cout << "Failed to get DXGI factory." << std::endl;
		return false;
	}

	ComPtr<IDXGISwapChain> swapChain;		//���� ü�� �������̽� �����͸� ����, ���� ü���� �������� �̹����� ȭ�鿡 ǥ���ϴ� �� ���Ǵ� �Ϸ��� ���۵��� ����
	hr = dxgiFactory->CreateSwapChain(_pDevice.Get(), &sd, &swapChain);
	if (FAILED(hr))
	{
		std::cout << "Failed to create swap chain." << std::endl;
		return false;
	}

	swapChain.As(&_pSwapChain);

	return true;
}

bool RendererHelper::CreateRenderTargetView(ComPtr<ID3D11Device>& _pDevice,
	ComPtr<IDXGISwapChain>& _pSwapChain,
	ComPtr<ID3D11RenderTargetView>& _pRenderTargetView,
	ComPtr<ID3D11Texture2D>& _pBuffer,
	UINT _index,
	UINT _screenWidth,
	UINT _screenHeight
)
{
	if (_index == 0)				//0��° RT�� ����� ���ΰ�?
	{
		_pSwapChain->GetBuffer(0, IID_PPV_ARGS(_pBuffer.GetAddressOf()));
		if (_pBuffer)
		{
			_pDevice->CreateRenderTargetView(_pBuffer.Get(), nullptr, _pRenderTargetView.GetAddressOf());
		}
		else
		{
			std::cout << "CreateRenderTargetView() failed." << std::endl;
			return false;
		}
		return true;
	}
	else
	{
		if (_index > 7)
		{
			std::cout << "CreateRenderTargetView() failed." << std::endl;
			return false;
		}
		//ù���� RT�� �ƴѰ�� ���۸� �������� RT�� ������ ���־�� �Ѵ�.
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = _screenWidth;				//ũ�⸦ ����.
		desc.Height = _screenHeight;			//ũ�⸦ ����.
		desc.MipLevels = 1;						//�ؽ�ó�� �ػ󵵸� �ٿ� ������ ����Ű�� �� ��� 1-�����ؽ���, Ǯ��ũ��/���� ����� �پ��
		desc.ArraySize = 1;						//�ؽ����� �迭�� �ǹ��Ѵ�. ť��ʰ��� �������� �ؽ��ĸ� �ϳ��� �迭�� �����Ҷ� �ַ� ����Ѵ�.
		desc.Usage = D3D11_USAGE_DEFAULT;		//GPU���� �б� �� ���� �׼����� �ʿ��� ���ҽ�
		desc.CPUAccessFlags = 0;				//CPU �׼��� ������ �����ϴ� �÷���
		desc.MiscFlags = 0;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; //����Ÿ�ٿ� bind(���ε�, ����)�ǰ� �ϰڴ�. ��, GPU�� �׸��� �׸��� ������� ����ϰڴٴ� ��
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  //32��Ʈ�� ������ R8��Ʈ, G8��Ʈ, B8��Ʈ, A8��Ʈ�� ��������(4���� �г�) unsigned normailize(0~1���̷� ����ȭ��)������ ���ڴ�.

		HRESULT hr = _pDevice->CreateTexture2D(&desc, nullptr, &_pBuffer);
		if (FAILED(hr))
		{
			std::cout << "Failed to create render target texture." << std::endl;
			return false;
		}

		// ���� Ÿ�� �� ����
		hr = _pDevice->CreateRenderTargetView(_pBuffer.Get(), nullptr, &_pRenderTargetView);
		if (FAILED(hr))
		{
			std::cout << "Failed to create render target view." << std::endl;
			return false;
		}
		return true;
	}
}

void RendererHelper::SetViewPort(ComPtr<ID3D11DeviceContext>& _pDeviceContext,
	int TopLeftX, int TopLeftY, int ScreenWidth, int ScreenHeight,
	D3D11_VIEWPORT& _screenViewPort)
{
	ZeroMemory(&_screenViewPort, sizeof(D3D11_VIEWPORT));
	_screenViewPort.TopLeftX = static_cast<float>(TopLeftX);
	_screenViewPort.TopLeftY = static_cast<float>(TopLeftY);
	_screenViewPort.Width = static_cast<float>(ScreenWidth);
	_screenViewPort.Height = static_cast<float>(ScreenHeight);
	_screenViewPort.MinDepth = 0.0f;
	_screenViewPort.MaxDepth = 1.0f; // Note: important for depth buffering

	_pDeviceContext->RSSetViewports(1, &_screenViewPort);
}

void RendererHelper::SetShadowViewport(ComPtr<ID3D11DeviceContext>& _pDeviceContext,
	float m_shadowWidth, float m_shadowHeight, D3D11_VIEWPORT& _screenViewPort)
{
	// Set the viewport
	ZeroMemory(&_screenViewPort, sizeof(D3D11_VIEWPORT));
	_screenViewPort.TopLeftX = 0;
	_screenViewPort.TopLeftY = 0;
	_screenViewPort.Width = float(m_shadowWidth);
	_screenViewPort.Height = float(m_shadowHeight);
	_screenViewPort.MinDepth = 0.0f;
	_screenViewPort.MaxDepth = 1.0f;

	_pDeviceContext->RSSetViewports(1, &_screenViewPort);
}

ComPtr<ID3D11RasterizerState> RendererHelper::CreateSolidRasterizerState(ComPtr<ID3D11Device>& _pDevice)
{
	ComPtr<ID3D11RasterizerState> D3dRasterizerSate;
	D3D11_RASTERIZER_DESC rastDesc;
	ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));		// Need this
	rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;		//�������� ä���.
	rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.DepthClipEnable = true;							// zNear, zFar Ȯ�ο� �ʿ�

	if (FAILED(_pDevice->CreateRasterizerState(&rastDesc, D3dRasterizerSate.GetAddressOf())))
	{
		return nullptr;
	}
	return D3dRasterizerSate;
}

ComPtr<ID3D11RasterizerState> RendererHelper::CreateWireRasterizerState(ComPtr<ID3D11Device>& _pDevice)
{
	ComPtr<ID3D11RasterizerState> D3dRasterizerSate;
	D3D11_RASTERIZER_DESC rastDesc;
	ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));		// Need this
	rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;	//�������� ����.(���̾� ���)
	rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.DepthClipEnable = true;							// zNear, zFar Ȯ�ο� �ʿ�

	if (FAILED(_pDevice->CreateRasterizerState(&rastDesc, D3dRasterizerSate.GetAddressOf())))
	{
		return nullptr;
	}
	return D3dRasterizerSate;
}

bool RendererHelper::CreateDepthStencilBuffer(ComPtr<ID3D11Device>& _pDevice,
	UINT& _numQualityLevels,
	int _screenWidth, int _screenHeight,
	ComPtr<ID3D11DepthStencilView>& _pdepthStencilView)
{
	D3D11_TEXTURE2D_DESC depthStencilBufferDesc = {};							// ���� ���ٽ� ���� Ư�� ����
	depthStencilBufferDesc.Width = _screenWidth;								// ��ũ�� �ʺ� ����
	depthStencilBufferDesc.Height = _screenHeight;								// ��ũ�� ���� ����
	depthStencilBufferDesc.MipLevels = 1;										// �����̰� �״�� ���ڴ�.
	depthStencilBufferDesc.ArraySize = 1;										// ���常 ���ڴ�.
	depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;				// ���� ���ٽ� ���� ���� ���� : 24��Ʈ ���̿� 8��Ʈ ���ٽ�
	///depthStencilBufferDesc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;		// ���� ���ٽ� ���� ���� ���� : 32��Ʈ ���̿� 8��Ʈ ���ٽ� 24��Ʈ�� �Ⱦ� = 64��Ʈ
	///depthStencilBufferDesc.Format = DXGI_FORMAT_D32_FLOAT;					// ���� ���ٽ� ���� ���� ���� : 32��Ʈ ���̿� ���Ľ��� ���� �ʰڴ�. -> ���̹��۸� �׸��� �̿�, 32��Ʈ�� ���е��� ���� �� �ִ�.
	if (_numQualityLevels > 0)													// ��Ƽ ���ø��� �����Ǵ� ��� ���� ���ٽ� ���
	{
		depthStencilBufferDesc.SampleDesc.Count = 4;							// ��Ƽ ���ø� ����
		depthStencilBufferDesc.SampleDesc.Quality = _numQualityLevels - 1;		// ��Ƽ ���ø� ǰ�� ���� ���� ID3D11Device::CheckMultisampleQualityLevels�Լ� ȣ���� ���� �����Ǵ� ǰ�������� ������ �ǹ�
	}
	else
	{
		depthStencilBufferDesc.SampleDesc.Count = 1;
		depthStencilBufferDesc.SampleDesc.Quality = 0;
	}
	depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;							// ���� ���ٽ� ������ ��� ����� ���� : �⺻ ����
	depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;				// ���� ���ٽ� ������ ���ε� �÷��� ���� : ���� ���ٽ� �ڿ����� ���ε���
	depthStencilBufferDesc.CPUAccessFlags = 0;									// CPU������ �׼��� �÷��׸� ���� : ������ �Ұ�
	depthStencilBufferDesc.MiscFlags = 0;										// ��Ÿ �÷��׸� ���� : ���� ����

	ComPtr<ID3D11Texture2D> depthStencilBuffer;									// ���� ���ٽ� ���� ���� ����

	// �����
	if (FAILED(_pDevice->CreateTexture2D(&depthStencilBufferDesc, 0, depthStencilBuffer.GetAddressOf())))			// ���� ���ٽ� ���۸� �����ϰ�, �̸� depthStencilBuffer�� ����
	{
		std::cout << "CreateTexture2D() failed." << std::endl;
		return false;
	}
	if (FAILED(_pDevice->CreateDepthStencilView(depthStencilBuffer.Get(), 0, _pdepthStencilView.GetAddressOf())))	// ���� ���ٽ� ���ۿ� ���� ���� ���ٽ� �並 �����ϰ�, �̸� depthStencilView�� ����
	{
		std::cout << "CreateDepthStencilView() failed." << std::endl;
		return false;
	}
	return true;
}

bool RendererHelper::CreateDepthOnlyBuffer(ComPtr<ID3D11Device>& _pDevice, int _screenWidth, int _screenHeight, ComPtr<ID3D11DepthStencilView>& _pdepthStencilView,
	ComPtr<ID3D11ShaderResourceView>& _pdepthOnlySRV)
{
	D3D11_TEXTURE2D_DESC depthStencilBufferDesc = {};							// ���� ���ٽ� ���� Ư�� ����
	depthStencilBufferDesc.Width = _screenWidth;								// ��ũ�� �ʺ� ����
	depthStencilBufferDesc.Height = _screenHeight;								// ��ũ�� ���� ����
	depthStencilBufferDesc.MipLevels = 1;										// MipLevel �˻� �� ����, 1 : �̼���
	depthStencilBufferDesc.ArraySize = 1;										// ArraySize �˻� �� ����, 1 : �̼���
	depthStencilBufferDesc.Format = DXGI_FORMAT_R32_TYPELESS;					// ���� ���ٽ� ���� ���� ���� : �ϴ��� R32��Ʈ�� ���ڴ�. �ؼ������ �������� �ʴ´�.
	depthStencilBufferDesc.SampleDesc.Count = 1;
	depthStencilBufferDesc.SampleDesc.Quality = 0;
	depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;							// ���� ���ٽ� ������ ��� ����� ���� : �⺻ ����
	depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;				// ���� ���ٽ� ������ ���ε� �÷��� ���� : ���� ���ٽ� �ڿ����� ���ε���
	depthStencilBufferDesc.CPUAccessFlags = 0;									// CPU������ �׼��� �÷��׸� ���� : ������ �Ұ�
	depthStencilBufferDesc.MiscFlags = 0;										// ��Ÿ �÷��׸� ���� : ���� ����

	ComPtr<ID3D11Texture2D> depthStencilBuffer;									// ���� ���ٽ� ���� ���� ����

	// �����
	if (FAILED(_pDevice->CreateTexture2D(&depthStencilBufferDesc, 0, depthStencilBuffer.GetAddressOf())))			// ���� ���ٽ� ���۸� �����ϰ�, �̸� depthStencilBuffer�� ����
	{
		std::cout << "CreateTexture2D() failed." << std::endl;
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;			 //ID3D11Texture2D�� ������ DXGI_FORMAT_R32_TYPELESS�̹Ƿ� DXGI_FORMAT_D32_FLOAT�� �����ؼ� �Ҵ�޴´�. ���̰��� �ؼ��Ҷ� �ַ� ����.
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	if (FAILED(_pDevice->CreateDepthStencilView(depthStencilBuffer.Get(), &dsvDesc, _pdepthStencilView.GetAddressOf())))	// ���� ���ٽ� ���ۿ� ���� ���� ���ٽ� �並 �����ϰ�, �̸� depthStencilView�� ����
	{
		std::cout << "CreateDepthStencilView() failed." << std::endl;
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;			//���̴� ���ҽ� ���� ���� ������ �����͸� Rä���� 32��Ʈ �ε� �Ҽ��� ������ �ؼ��ϱ� �����̴�.
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	if (FAILED(_pDevice->CreateShaderResourceView(depthStencilBuffer.Get(), &srvDesc, _pdepthOnlySRV.GetAddressOf())))
	{
		std::cout << "CreateDepthStencilView() failed." << std::endl;
		return false;
	}

	return true;
}

ComPtr<ID3D11Buffer> RendererHelper::CreateIndexBuffer(ComPtr<ID3D11Device>& _pDevice, const unsigned int _numIndices, unsigned int* _indices)
{
	D3D11_BUFFER_DESC bufferIDesc = {};
	bufferIDesc.Usage = D3D11_USAGE_IMMUTABLE; // �ʱ�ȭ �� ����X
	bufferIDesc.ByteWidth = UINT(sizeof(uint32_t) * _numIndices);
	bufferIDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferIDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
	bufferIDesc.StructureByteStride = sizeof(uint32_t);

	D3D11_SUBRESOURCE_DATA indexBufferData = {};
	indexBufferData.pSysMem = _indices;
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;

	ComPtr<ID3D11Buffer> indexBuffer;

	_pDevice->CreateBuffer(&bufferIDesc, &indexBufferData, indexBuffer.GetAddressOf());
	return indexBuffer;
}

ComPtr<ID3D11UnorderedAccessView> RendererHelper::CreateUnorderedAccessView(ComPtr<ID3D11Device>& _pDevice, ComPtr<ID3D11Buffer> _structuredBuffer, unsigned int _count)
{

	// UAV ����
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;                        // Structured Buffer�� ��� DXGI_FORMAT_UNKNOWN ����
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.NumElements = _count;                         // ���ۿ� ����� ��� ���� ����

	ComPtr<ID3D11UnorderedAccessView> uav;
	HRESULT hr = _pDevice->CreateUnorderedAccessView(_structuredBuffer.Get(), &uavDesc, uav.GetAddressOf());
	if (FAILED(hr)) {
		std::cout << "CreateUnorderedAccessView() failed. " << std::hex << hr << std::endl;
		return nullptr; // ���� ó��
	}

	return uav;

}
/// <summary>
/// 
/// </summary>
/// <param name="_pDevice"></param>
/// <param name="_structuredBuffer"></param>
/// <param name="_count">����� ���� / -1 �ϰ�� srvDesc�� nullptr�� �����ȴ�.</param>
/// <returns></returns>
ComPtr<ID3D11ShaderResourceView> RendererHelper::CreateShaderResourceView(ComPtr<ID3D11Device>& _pDevice, ComPtr<ID3D11Buffer> _structuredBuffer, std::optional<unsigned int> _count)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.BufferEx.NumElements = _count.value_or(0);		//���� ������ 0 ���
	ComPtr<ID3D11ShaderResourceView> srv;
	HRESULT hr;
	if (!_count.has_value())
	{
		hr = _pDevice->CreateShaderResourceView(_structuredBuffer.Get(), nullptr, srv.GetAddressOf());
	}
	else 
	{
		hr = _pDevice->CreateShaderResourceView(_structuredBuffer.Get(), &srvDesc, srv.GetAddressOf());
	}
	if (FAILED(hr))
	{
		std::cout << "CreateShaderResourceView() failed. " << std::hex << hr << std::endl;
		return nullptr; // ���� ó��
	}
	return srv;
}



ComPtr<ID3D11ShaderResourceView> RendererHelper::Create3DTexture(ComPtr<ID3D11Device>& _pDevice, std::string _filename)
{
	//RGB ������ �̹����� RGBA �������� ��ȯ. stb_image.h�� ����Ͽ� �̹����� �ε��ϰ�, ��ȯ�� �����͸� std::vector<uint8_t>�� ����
	int width(0), height(0), channels(0);												// �̹����� ��(width), ����(height), ä�� ��(channels)�� ��Ÿ���� ������ �����ϰ� �ʱⰪ�� 0���� ����
	unsigned char* img = stbi_load(_filename.c_str(), &width, &height, &channels, 0);	//������ ����(filename)���� �̹����� �о�ͼ� img �����Ϳ� ����/ width, height, channels �������� �̹����� ũ��� ä�� ���� ������
	if (!img)
	{
		DEBUG_LOG("Failed to load texture: " << _filename);
		return nullptr; // ���� ó�� �߰�
	}

	std::vector<uint8_t> image;										//��ȯ�� �̹��� �����͸� ������ ���͸� ����
	image.resize(width * height * 4);								//�̹��� �����͸� ������ ����(image)�� ũ�⸦ �̹����� ��(width), ����(height), 4���� ä��(RGBA)�� ����Ͽ� ����

	for (size_t pixel = 0; pixel < width * height; pixel++)			// �̹����� ��� �ȼ��� ���� �ݺ��ϴ� ����
	{
		for (size_t rgb = 0; rgb < 3; rgb++)						// �� �ȼ��� RGB ���� ó���ϴ� ����
		{
			image[4 * pixel + rgb] = img[pixel * channels + rgb];	// ���� �ȼ��� RGB ���� image ���Ϳ� ����. 4 * pixel + rgb�� �� �ȼ����� R, G, B �� ä�ο� �ش��ϴ� �ε����� ���
		}
		image[4 * pixel + 3] = 255;									//�� �ȼ��� Alpha ä���� 255(������)�� ����
	}

	stbi_image_free(img); // �̹��� �޸� ����

	//�ؽ�ó �������� ������ ����.
	D3D11_TEXTURE2D_DESC texDesc = {};				// ��� ����� 0 �Ǵ� null�� �ʱ�ȭ
	texDesc.Width = width;							// �ؽ�ó�� ���� ũ�⸦ �̹����� ���� ũ��(width)�� ����
	texDesc.Height = height;						// �ؽ�ó�� ���� ũ�⸦ �̹����� ���� ũ��(height)�� ����
	texDesc.MipLevels = texDesc.ArraySize = 1;		// �ؽ�ó�� �̼��� ���� �� �� �迭�� ũ�⸦ 1�� ���� (��� ����)
	// cf. MipLevels : �ؽ�ó�� �ػ󵵸� �ٿ� ������ ����Ű�� �� ���, ���� �ؽ�ó�� �ػ󵵸� ������ �ٿ����鼭 ����, �ָ� ������ ��ü�� ���� �ؽ�ó ������ �� ȿ�������� ó��
	// cf. ArraySize : ���� ���� �ؽ�ó�� �ϳ��� �ؽ�ó�� ����ϴ� ���, �ؽ�ó �迭�� ����ϸ� �ٸ� ũ�� �Ǵ� �ٸ� �̹����� ���� ������ �ϳ��� �ؽ�ó�� ���� ���
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// �ؽ�ó�� �ȼ� ������ 32��Ʈ RGBA �������� ����
	texDesc.SampleDesc.Count = 1;					// ��Ƽ���ø� ���� 1�� ���� : ��Ƽ ���ø� ��� X
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;			// �ؽ�ó�� ��� ��� : �ؽ�ó�� ������ ������ �ǹ��ϸ�, �ʱ�ȭ �Ŀ��� �������� �ʴ´�
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // �ؽ�ó�� ���̴� ���ҽ��� ���ε��ϵ��� ����

	// Fill in the subresource data.
	D3D11_SUBRESOURCE_DATA InitData;				// �ؽ�ó�� �ʱ� �����͸� ����
	InitData.pSysMem = image.data();				// �ʱ� �����ͷ� �̹��� ������ ������ �����͸� ����
	InitData.SysMemPitch = texDesc.Width * sizeof(uint8_t) * 4; // �̹����� ���� �� ������ ����Ʈ ���� �Ÿ��� ���� 

	ComPtr<ID3D11Texture2D> pTexture;
	ComPtr<ID3D11ShaderResourceView> pTextureResourceView;

	HRESULT hr = _pDevice->CreateTexture2D(&texDesc, &InitData, pTexture.GetAddressOf());
	if (FAILED(hr))
	{
		DEBUG_LOG("CreateTexture2D() failed. " << std::hex << hr);
		return nullptr; // ���� ó��
	}
	hr = _pDevice->CreateShaderResourceView(pTexture.Get(), nullptr, pTextureResourceView.GetAddressOf());
	if (FAILED(hr))
	{
		DEBUG_LOG("CreateShaderResourceView() failed. " << std::hex << hr);
		return nullptr; // ���� ó��
	}

	return pTextureResourceView;
}

ComPtr<ID3D11ShaderResourceView> RendererHelper::Create2DTexture(ComPtr<ID3D11Device>& _pDevice, std::string _filename)
{
	//RGB ������ �̹����� RGBA �������� ��ȯ. stb_image.h�� ����Ͽ� �̹����� �ε��ϰ�, ��ȯ�� �����͸� std::vector<uint8_t>�� ����
	int width(0), height(0);												// �̹����� ��(width), ����(height), ä�� ��(channels)�� ��Ÿ���� ������ �����ϰ� �ʱⰪ�� 0���� ����
	unsigned char* img = stbi_load(_filename.c_str(), &width, &height, NULL, 4);	//������ ����(filename)���� �̹����� �о�ͼ� img �����Ϳ� ����/ width, height, channels �������� �̹����� ũ��� ä�� ���� ������
	if (!img)
	{
		std::cout << "�̹��� �ε忡 �����߽��ϴ�." << std::endl;
		return nullptr;
	}

	//�ؽ�ó �������� ������ ����.
	D3D11_TEXTURE2D_DESC texDesc = {};				// ��� ����� 0 �Ǵ� null�� �ʱ�ȭ
	texDesc.Width = width;							// �ؽ�ó�� ���� ũ�⸦ �̹����� ���� ũ��(width)�� ����
	texDesc.Height = height;						// �ؽ�ó�� ���� ũ�⸦ �̹����� ���� ũ��(height)�� ����
	texDesc.MipLevels = texDesc.ArraySize = 1;		// �ؽ�ó�� �̼��� ���� �� �� �迭�� ũ�⸦ 1�� ���� (��� ����)
	// cf. MipLevels : �ؽ�ó�� �ػ󵵸� �ٿ� ������ ����Ű�� �� ���, ���� �ؽ�ó�� �ػ󵵸� ������ �ٿ����鼭 ����, �ָ� ������ ��ü�� ���� �ؽ�ó ������ �� ȿ�������� ó��
	// cf. ArraySize : ���� ���� �ؽ�ó�� �ϳ��� �ؽ�ó�� ����ϴ� ���, �ؽ�ó �迭�� ����ϸ� �ٸ� ũ�� �Ǵ� �ٸ� �̹����� ���� ������ �ϳ��� �ؽ�ó�� ���� ���
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// �ؽ�ó�� �ȼ� ������ 32��Ʈ RGBA �������� ����
	texDesc.SampleDesc.Count = 1;					// ��Ƽ���ø� ���� 1�� ���� : ��Ƽ ���ø� ��� X
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;			// �ؽ�ó�� ��� ��� : �ؽ�ó�� ������ ������ �ǹ��ϸ�, �ʱ�ȭ �Ŀ��� �������� �ʴ´�
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // �ؽ�ó�� ���̴� ���ҽ��� ���ε��ϵ��� ����
	texDesc.CPUAccessFlags = 0;

	ComPtr<ID3D11Texture2D> pTexture;
	ComPtr<ID3D11ShaderResourceView> pTextureResourceView;

	D3D11_SUBRESOURCE_DATA subResource;
	subResource.pSysMem = img;
	subResource.SysMemPitch = texDesc.Width * 4;
	subResource.SysMemSlicePitch = 0;
	_pDevice->CreateTexture2D(&texDesc, &subResource, &pTexture);

	// Create texture view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	_pDevice->CreateShaderResourceView(pTexture.Get(), &srvDesc, pTextureResourceView.GetAddressOf());

	stbi_image_free(img);

	return pTextureResourceView;
}

ComPtr<ID3D11ShaderResourceView> RendererHelper::CreateDDSTexture(ComPtr<ID3D11Device>& _pDevice, std::string _filename)
{
	// std::string�� std::wstring���� ��ȯ
	std::wstring wfilename = std::wstring(_filename.begin(), _filename.end());

	ComPtr<ID3D11Texture2D> pTexture;
	ComPtr<ID3D11ShaderResourceView> pTextureResourceView;

	auto hr = DirectX::CreateDDSTextureFromFileEx(
		_pDevice.Get(), wfilename.c_str(), 0, D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE, 0,
		D3D11_RESOURCE_MISC_TEXTURECUBE, // ť��ʿ� �ؽ���
		DirectX::DX11::DDS_LOADER_FLAGS(false), (ID3D11Resource**)pTexture.GetAddressOf(),
		pTextureResourceView.GetAddressOf(), nullptr);

	if (FAILED(hr)) {
		std::cout << "CreateDDSTextureFromFileEx() failed" << std::endl;
		return nullptr;
	}
	return pTextureResourceView;
}

///�׸��ڿ� shadowMap Texture�� SPV, DSV�� �����Ѵ�.
bool RendererHelper::CreateShadowMapAndShaderResourceView(ComPtr<ID3D11Device>& _pDevice, int _screenWidth, int _screenHeight,
	ComPtr<ID3D11DepthStencilView>& _pShadowMap, ComPtr<ID3D11ShaderResourceView>& _pShadowResourceView)
{
	D3D11_TEXTURE2D_DESC depthBufferDesc = {};								   //���� ���Ľ� ������ Ư���� �����ϴ� ����ü�� �ʱ�ȭ�Ѵ�.
	depthBufferDesc.Width = _screenWidth;
	depthBufferDesc.Height = _screenHeight;
	depthBufferDesc.MipLevels = 1;											   //�Ӹ��� �������� ���� �ػ󵵸� ����ϴ� ��������, ���̹��ۿ����� �ʿ����� �ʴ�.
	depthBufferDesc.ArraySize = 1;											   //�迭�� ũ�⸦ 1�� ����(���� �ؽ���), ������ ���� ���� ���� �ؽ��ķ� ���ȴ�. 
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;							   //�⺻ ������� �����Ͽ� GPU�� �а� ���� �뵵�� ���
	depthBufferDesc.Format = DXGI_FORMAT_R32_TYPELESS;

	depthBufferDesc.SampleDesc.Count = 1;									   //�Ϲ������� ������ʿ����� ��Ƽ ���ø��� ������� �ʱ⿡ 1�� ����(��Ȱ��ȭ)
	depthBufferDesc.SampleDesc.Quality = 0;									   //��Ƽ���ø� ǰ���� 0���� ����
	depthBufferDesc.CPUAccessFlags = 0;										   //CPU.������ ��Ȱ��ȭ
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;		//���� ���Ľ� �� �� ���̴� ���ҽ� ��� ��� ��밡���ϵ��� ����. ��, �� �ؽ��Ĵ� ���̹��۷� ���ɼ� �ְ�, ���̴����� �б�뵵�� ���� ���� �ִ�.
	depthBufferDesc.MiscFlags = 0;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;

	ComPtr<ID3D11Texture2D> shadowDepthBuffer;
	// �����
	if (FAILED(_pDevice->CreateTexture2D(&depthBufferDesc, 0, shadowDepthBuffer.GetAddressOf())))
	{
		std::cout << "CreateShaderResourceView - CreateTexture2D() failed." << std::endl;
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	//ComPtr<ID3D11DepthStencilView> ShadowMap;
	if (FAILED(_pDevice->CreateDepthStencilView(shadowDepthBuffer.Get(), &descDSV, _pShadowMap.GetAddressOf())))
	{
		std::cout << "CreateShaderResourceView - CreateDepthStencilView() failed." << std::endl;
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srcDesc = {};
	srcDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srcDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srcDesc.Texture2D.MipLevels = 1;

	//ComPtr<ID3D11ShaderResourceView> ShadowResourceView;
	if (FAILED(_pDevice->CreateShaderResourceView(shadowDepthBuffer.Get(), &srcDesc, _pShadowResourceView.GetAddressOf())))
	{
		std::cout << "CreateShaderResourceView - CreateShaderResourceView() failed." << std::endl;
		return false;
	}
	return true;

}

ComPtr<ID3D11DepthStencilState> RendererHelper::CreateDepthStencilState(ComPtr<ID3D11Device>& _pDevice)
{
	ComPtr<ID3D11DepthStencilState> pDepthStencilState;

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
	if (FAILED(_pDevice->CreateDepthStencilState(&depthStencilDesc, pDepthStencilState.GetAddressOf())))
	{
		std::cout << "CreateDepthStencilState() failed." << std::endl;
		return nullptr;
	}

	return pDepthStencilState;
}

ComPtr<ID3D11SamplerState> RendererHelper::CreateSamplerState(ComPtr<ID3D11Device>& _pDevice)
{
	ComPtr<ID3D11SamplerState> pSamplerState;

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	if (FAILED(_pDevice->CreateSamplerState(&sampDesc, pSamplerState.GetAddressOf())))
	{
		std::cout << "CreateSamplerState() failed." << std::endl;
		return nullptr;
	}
	return pSamplerState;
}

/// inputElements������ �������� m_vertexShader, m_inputLayout�� �����Ѵ�.
void RendererHelper::CreateVertexShaderAndInputLayout(ComPtr<ID3D11Device>& device, const std::string& filename, const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputElements,
	ComPtr<ID3D11VertexShader>& m_vertexShader, ComPtr<ID3D11InputLayout>& m_inputLayout)
{
	ComPtr<ID3DBlob> shaderBlob;
	ComPtr<ID3DBlob> errorBlob;

	UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	// ���̴��� �������� �̸��� "main"�� �Լ��� ����
	// D3D_COMPILE_STANDARD_FILE_INCLUDE �߰�: ���̴����� include ���
	HRESULT hr = D3DCompileFromFile(
		StringToWstring(filename).c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
		"vs_5_0", compileFlags, 0, &shaderBlob, &errorBlob);

	CheckResult(hr, errorBlob.Get());

	hr = device->CreateVertexShader(shaderBlob->GetBufferPointer(),
		shaderBlob->GetBufferSize(), NULL,
		&m_vertexShader);
	auto e = shaderBlob->GetBufferPointer();
	auto e1 = shaderBlob->GetBufferSize();
#if _DEBUG
	if (FAILED(hr))
	{
		DEBUG_LOG(filename << "failed CreateVertexShader.");
	}
#endif

	hr = device->CreateInputLayout(inputElements.data(), UINT(inputElements.size()),
		shaderBlob->GetBufferPointer(),
		shaderBlob->GetBufferSize(), &m_inputLayout);

#if _DEBUG
	if (FAILED(hr))
	{
		DEBUG_LOG("hr - " << hr << " / " << filename << " - failed CreateInputLayout.");
	}
#endif
}

std::wstring RendererHelper::StringToWstring(const std::string& str)
{
	std::wstring wstr(str.begin(), str.end());
	return wstr;
}


void RendererHelper::CheckResult(HRESULT hr, ID3DBlob* errorBlob)
{
	if (FAILED(hr))
	{
		// ������ ���� ���
		if ((hr & D3D11_ERROR_FILE_NOT_FOUND) != 0)
		{
			DEBUG_LOG("File not found.");
		}

		// ���� �޽����� ������ ���
		if (errorBlob)
		{
			DEBUG_LOG("Shader compile error\n" << (char*)errorBlob->GetBufferPointer());
			errorBlob->Release(); // ���� ��� ����
		}
	}
}


void RendererHelper::CreatePixelShader(ComPtr<ID3D11Device>& device,
	const std::string& filename,
	ComPtr<ID3D11PixelShader>& _pixelShader)
{
	ComPtr<ID3DBlob> shaderBlob;	//D3D �����Ϸ��� �������� ���̴� �ڵ��� ����Ʈ �ڵ尡 ����
	ComPtr<ID3DBlob> errorBlob;		//�����޼����� �����ϴµ� ���

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	dwShaderFlags |= D3DCOMPILE_DEBUG;
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;

#endif // _DEBUG


	//���̴��� �̸��� �������� "main"�� �Լ��� ����
	HRESULT hr = D3DCompileFromFile(
		StringToWstring(filename).c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
		"ps_5_0", dwShaderFlags, 0, &shaderBlob, &errorBlob);	//�־��� ���Ͽ��� �ȼ� ���̴��� ������.


	CheckResult(hr, errorBlob.Get());																					//�Լ��� �������� Ȯ��

	device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, &_pixelShader);		//�ȼ� ���̴� ����

}

void RendererHelper::CreateComputeShader(ComPtr<ID3D11Device>& device, const std::string& filename, ComPtr<ID3D11ComputeShader>& _computeShader)
{
	{
		ComPtr<ID3DBlob> shaderBlob;				//�����ϵ� ���̴� �ڵ带 ������ ID3DBlob ��ü, �����ϵ� ���̴� �����Ͱ� ���⿡ ����ȴ�.
		ComPtr<ID3DBlob> errorBlob;					//������ �������� �߻��� ���� ������ �����ϴ� ID3DBlob��ü, �����߻��� ���⿡ �����޼����� ��´�.

		UINT compileFlags = 0;						//���̴� ������ �� ����� �÷��׸� �����Ѵ�.
#if defined(DEBUG) || defined(_DEBUG)
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;			//����� ������ �ΰ��� �÷��׸� �߰��Ѵ�.
		///D3DCOMPILE_DEBUG				: ����� ������ ���Խ�Ų��.
		///D3DCOMPILE_SKIP_OPTIMIZATION	: ����ȭ�� ������ ������� �����Ѵ�.
#endif

		// ��ǻƮ ���̴� ������ �������Ͽ� Blob�� ����
		HRESULT hr = D3DCompileFromFile(
			StringToWstring(filename).c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main",									// ��ǻƮ ���̴��� ���� �Լ� �̸�
			"cs_5_0",								// ��ǻƮ ���̴��� Ÿ�� ��������
			compileFlags, 0, &shaderBlob, &errorBlob);

		CheckResult(hr, errorBlob.Get());

		// ��ǻƮ ���̴� ����
		hr = device->CreateComputeShader(
			shaderBlob->GetBufferPointer(),			//�����ϵ� ���̴� �ڵ��� �����ּ�
			shaderBlob->GetBufferSize(),			//���̴� �ڵ��� ũ��(����Ʈ ����)
			nullptr,								//nullptr : �⺻ �������̽��� ����ϰڴ�.
			&_computeShader);

#if _DEBUG
		if (FAILED(hr)) {
			DEBUG_LOG(filename << " failed to create Compute Shader.");
		}
#endif
	}
}
