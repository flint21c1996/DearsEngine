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
	const D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;		//드라이버 타입을 정한다.

#if defined(DEBUG) || defined(_DEBUG)  
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	const D3D_FEATURE_LEVEL featureLevels[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_9_3 };
	D3D_FEATURE_LEVEL featureLevel;

	//디바이스를 생성
	if (FAILED(D3D11CreateDevice(
		nullptr,										//디바이스를 만들때 사용할 비디오 어댑터, 기본어댑터를 사용시 NULL을 전달
		driverType,										//만들 드라이버 유형
		0,												//소프트웨어 래스터라이저를 구현하는 DLL에 대한 핸들
		createDeviceFlags,								//디바이스를 생성할 때 사용핧 플래그를 설정하는 변수, 0 = 기본적으로 아무 플래그도 설정하지 않는다.
		featureLevels,									//만들려는 기능 수준의 순서를 결정하는 D3D_FEATURE_LEVEL에 대한 포인터
		ARRAYSIZE(featureLevels),						//featureLevels의 요소 수
		D3D11_SDK_VERSION,								//D3D11_CREATE_DEVICE_FLAG 열거형
		&_pDevice,										//만든 디바이스를 나타내는 ID3D11Device 개체에 대한 포인터의 주소, 이곳에다 반환한다.
		&featureLevel,									//성공할 경우 featureLevels 배열에서 첫번째 D3D_FEATURE_LEVEL 반환, 지원되는 기능 수준을 결정할 필요가 없는경우 NULL을 입력으로 제공
		&_pDeviceContext								//바이스 컨텍스트를 나타내는 ID3D11DeviceContext 개체에 대한 포인터의 주소
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
	// 4X MSAA 지원하는지 확인, 멀티샘플링이 몇단계까지 되는가?
	HRESULT hr = _pDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &_numQualityLevels);
	if (FAILED(hr) || _numQualityLevels <= 0)
	{
		std::cout << "MSAA not supported." << std::endl;
		_numQualityLevels = 0; // MSAA 강제로 끄기 -> 디퍼드 렌더링을 간접 체험할 때 사용해보자.
	}

	//스왑체인 - 다중버퍼링
	DXGI_SWAP_CHAIN_DESC sd;												//스왑체인의 정보를 담고있는 구조체
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferDesc.Width = _screenWidth;										//해상도 설정 - 백버퍼의 크기, DXGI_MODE_DESC의 정보를 채우는 것이다.
	sd.BufferDesc.Height = _screenHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;						//백버퍼의 색상규격
	sd.BufferCount = 1;														//백버퍼의 갯수
	sd.BufferDesc.RefreshRate.Numerator = 0xffffffff;						//지금은 제한값을 풀어놨다. //이전 : 일단 60프레임으로 고정을 한다. 수직동기화를 사용했다고 가정하자. 이부분을 0으로 설정한다면 운영체제나 드라이버가 모니터의 기본화면 새로고침빈도를 기반으로 적절한 값을 설정한다.
	sd.BufferDesc.RefreshRate.Denominator = 1;								//화면 고침 빈도의 분모를 설정하는 부분이다. numerator과 함께 사용되어 화면 새로고침 빈도를 나타낸다.
	sd.BufferUsage = DXGI_USAGE_SHADER_INPUT |								//후처리 필터 적용을 위해
						DXGI_USAGE_RENDER_TARGET_OUTPUT;					//BufferUsage = 어떤 용도로 사용할 것인가? /백 버퍼의 표면 사용량 및 엑세스 옵션을 설정하는 DXGI_USAGE 열거형 형식의 멤버, 여기서는 RT0렌더타겟이 그림을 그릴곳으로.
	sd.OutputWindow = _hWnd;												//출력할 윈도우의 핸들
	sd.Windowed = TRUE;														//전체화면을 사용할 것인가? /창모드
	//sd.Windowed = FALSE;													//전체화면 모드
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	if (_numQualityLevels > 0)
	{
		sd.SampleDesc.Count = 4; // how many multisamples, 이미지의 품질 수준, 품질이 높을 수록 성능이 낮아질수 있지만.. 
		sd.SampleDesc.Quality = _numQualityLevels - 1;
	}
	//No MSAA
	else
	{
		sd.SampleDesc.Count = 1; // how many multisamples, 안티앨리어싱이 없는 기본 샘플링 모드의 개수는 1이고 품질 수준은 0이다. /안티앨리어싱은 게단현상을 줄이기위한 기술이다. 예 - 한 픽셀안에 여러개의 샘플을 게산해 평균을 내는 멀티샘플링
		sd.SampleDesc.Quality = 0;
	}

	// IDXGIFactory를 이용한 CreateSwapChain()----------------------------------------------------------------------------------------
	//D3D11CreateDeviceAndSwapChain이라는 함수를 이용해서 디바이스와 스왑체인을 한번에 생산도 가능하다 -> 이 함수는 내부적으로 DXGIFactory를 가지고 있기 때문이다. 
	ComPtr<IDXGIDevice3> dxgiDevice;		//DXGI Device
	hr = _pDevice.As(&dxgiDevice);
	if (FAILED(hr))
	{
		std::cout << "Failed to get DXGI device." << std::endl;
		return false;
	}

	ComPtr<IDXGIAdapter> dxgiAdapter;		//DXGI 어댑터 인터페이스 포인터를 선언, 어댑터는 그래픽 카드나 비디오 카드를 의미
	hr = dxgiDevice->GetAdapter(&dxgiAdapter);
	if (FAILED(hr))
	{
		std::cout << "Failed to get DXGI adapter." << std::endl;
		return false;
	}

	ComPtr<IDXGIFactory> dxgiFactory;		//DXGI 팩토리 인터페이스 포인터를 선언, DXGI 팩토리는 다양한 DXGI 객체를 생성하는 데 사용
	hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(hr))
	{
		std::cout << "Failed to get DXGI factory." << std::endl;
		return false;
	}

	ComPtr<IDXGISwapChain> swapChain;		//스왑 체인 인터페이스 포인터를 선언, 스왑 체인은 렌더링된 이미지를 화면에 표시하는 데 사용되는 일련의 버퍼들을 관리
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
	if (_index == 0)				//0번째 RT를 담당할 것인가?
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
		//첫번재 RT가 아닌경우 버퍼를 생성한후 RT와 연결을 해주어야 한다.
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = _screenWidth;				//크기를 설정.
		desc.Height = _screenHeight;			//크기를 설정.
		desc.MipLevels = 1;						//텍스처의 해상도를 줄여 성능을 향상시키는 데 사용 1-원본텍스쳐, 풀스크린/이후 배수로 줄어듬
		desc.ArraySize = 1;						//텍스쳐의 배열을 의미한다. 큐브맵같이 여러개의 텍스쳐를 하나의 배열로 관리할때 주로 사용한다.
		desc.Usage = D3D11_USAGE_DEFAULT;		//GPU에서 읽기 및 쓰기 액세스가 필요한 리소스
		desc.CPUAccessFlags = 0;				//CPU 액세스 유형을 지정하는 플래그
		desc.MiscFlags = 0;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; //렌더타겟에 bind(바인드, 결합)되게 하겠다. 즉, GPU가 그림을 그리는 대상으로 사용하겠다는 뜻
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  //32비트를 쓸껀데 R8비트, G8비트, B8비트, A8비트로 나누어진(4개의 패널) unsigned normailize(0~1사이로 정규화된)값으로 쓰겠다.

		HRESULT hr = _pDevice->CreateTexture2D(&desc, nullptr, &_pBuffer);
		if (FAILED(hr))
		{
			std::cout << "Failed to create render target texture." << std::endl;
			return false;
		}

		// 렌더 타겟 뷰 생성
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
	rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;		//폴리곤을 채운다.
	rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.DepthClipEnable = true;							// zNear, zFar 확인에 필요

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
	rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;	//폴리곤을 비운다.(와이어 모드)
	rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.DepthClipEnable = true;							// zNear, zFar 확인에 필요

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
	D3D11_TEXTURE2D_DESC depthStencilBufferDesc = {};							// 깊이 스텐실 버퍼 특성 설명
	depthStencilBufferDesc.Width = _screenWidth;								// 스크린 너비 설정
	depthStencilBufferDesc.Height = _screenHeight;								// 스크린 높이 설정
	depthStencilBufferDesc.MipLevels = 1;										// 안줄이고 그대로 쓰겠다.
	depthStencilBufferDesc.ArraySize = 1;										// 한장만 쓰겠다.
	depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;				// 깊이 스텐실 버퍼 형실 설정 : 24비트 깊이와 8비트 스텐실
	///depthStencilBufferDesc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;		// 깊이 스텐실 버퍼 형실 설정 : 32비트 깊이와 8비트 스텐실 24비트는 안씀 = 64비트
	///depthStencilBufferDesc.Format = DXGI_FORMAT_D32_FLOAT;					// 깊이 스텐실 버퍼 형실 설정 : 32비트 깊이와 스탠실은 쓰지 않겠다. -> 깊이버퍼를 그릴때 이용, 32비트라 정밀도가 조금 더 있다.
	if (_numQualityLevels > 0)													// 멀티 샘플링이 지원되는 경우 깊이 스텐실 사용
	{
		depthStencilBufferDesc.SampleDesc.Count = 4;							// 멀티 샘플링 개수
		depthStencilBufferDesc.SampleDesc.Quality = _numQualityLevels - 1;		// 멀티 샘플링 품질 수준 설정 ID3D11Device::CheckMultisampleQualityLevels함수 호출을 통해 지원되는 품질수준의 개수를 의미
	}
	else
	{
		depthStencilBufferDesc.SampleDesc.Count = 1;
		depthStencilBufferDesc.SampleDesc.Quality = 0;
	}
	depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;							// 깊이 스텐실 버퍼의 사용 방법을 설정 : 기본 설정
	depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;				// 깊이 스텐실 버퍼의 바인딩 플래그 설정 : 깊이 스텐실 자원으로 바인딩됨
	depthStencilBufferDesc.CPUAccessFlags = 0;									// CPU에서의 액세스 플래그를 설정 : 엑세스 불가
	depthStencilBufferDesc.MiscFlags = 0;										// 기타 플래그를 설정 : 설정 없음

	ComPtr<ID3D11Texture2D> depthStencilBuffer;									// 깊이 스텐실 버퍼 변수 선언

	// 디버깅
	if (FAILED(_pDevice->CreateTexture2D(&depthStencilBufferDesc, 0, depthStencilBuffer.GetAddressOf())))			// 깊이 스텐실 버퍼를 생성하고, 이를 depthStencilBuffer에 저장
	{
		std::cout << "CreateTexture2D() failed." << std::endl;
		return false;
	}
	if (FAILED(_pDevice->CreateDepthStencilView(depthStencilBuffer.Get(), 0, _pdepthStencilView.GetAddressOf())))	// 깊이 스텐실 버퍼에 대한 깊이 스텐실 뷰를 생성하고, 이를 depthStencilView에 저장
	{
		std::cout << "CreateDepthStencilView() failed." << std::endl;
		return false;
	}
	return true;
}

bool RendererHelper::CreateDepthOnlyBuffer(ComPtr<ID3D11Device>& _pDevice, int _screenWidth, int _screenHeight, ComPtr<ID3D11DepthStencilView>& _pdepthStencilView,
	ComPtr<ID3D11ShaderResourceView>& _pdepthOnlySRV)
{
	D3D11_TEXTURE2D_DESC depthStencilBufferDesc = {};							// 깊이 스텐실 버퍼 특성 설명
	depthStencilBufferDesc.Width = _screenWidth;								// 스크린 너비 설정
	depthStencilBufferDesc.Height = _screenHeight;								// 스크린 높이 설정
	depthStencilBufferDesc.MipLevels = 1;										// MipLevel 검색 후 참조, 1 : 미설정
	depthStencilBufferDesc.ArraySize = 1;										// ArraySize 검색 후 참조, 1 : 미설정
	depthStencilBufferDesc.Format = DXGI_FORMAT_R32_TYPELESS;					// 깊이 스텐실 버퍼 형식 설정 : 일단은 R32비트를 쓰겠다. 해석방법은 지정하지 않는다.
	depthStencilBufferDesc.SampleDesc.Count = 1;
	depthStencilBufferDesc.SampleDesc.Quality = 0;
	depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;							// 깊이 스텐실 버퍼의 사용 방법을 설정 : 기본 설정
	depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;				// 깊이 스텐실 버퍼의 바인딩 플래그 설정 : 깊이 스텐실 자원으로 바인딩됨
	depthStencilBufferDesc.CPUAccessFlags = 0;									// CPU에서의 액세스 플래그를 설정 : 엑세스 불가
	depthStencilBufferDesc.MiscFlags = 0;										// 기타 플래그를 설정 : 설정 없음

	ComPtr<ID3D11Texture2D> depthStencilBuffer;									// 깊이 스텐실 버퍼 변수 선언

	// 디버깅
	if (FAILED(_pDevice->CreateTexture2D(&depthStencilBufferDesc, 0, depthStencilBuffer.GetAddressOf())))			// 깊이 스텐실 버퍼를 생성하고, 이를 depthStencilBuffer에 저장
	{
		std::cout << "CreateTexture2D() failed." << std::endl;
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;			 //ID3D11Texture2D의 포맷이 DXGI_FORMAT_R32_TYPELESS이므로 DXGI_FORMAT_D32_FLOAT로 포맷해서 할당받는다. 깊이값을 해석할때 주로 쓴다.
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	if (FAILED(_pDevice->CreateDepthStencilView(depthStencilBuffer.Get(), &dsvDesc, _pdepthStencilView.GetAddressOf())))	// 깊이 스텐실 버퍼에 대한 깊이 스텐실 뷰를 생성하고, 이를 depthStencilView에 저장
	{
		std::cout << "CreateDepthStencilView() failed." << std::endl;
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;			//세이더 리소스 같은 경우는 동일한 데이터를 R채널의 32비트 부동 소수점 값으로 해석하기 위함이다.
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
	bufferIDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
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

	// UAV 생성
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;                        // Structured Buffer의 경우 DXGI_FORMAT_UNKNOWN 설정
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.NumElements = _count;                         // 버퍼에 저장된 요소 개수 설정

	ComPtr<ID3D11UnorderedAccessView> uav;
	HRESULT hr = _pDevice->CreateUnorderedAccessView(_structuredBuffer.Get(), &uavDesc, uav.GetAddressOf());
	if (FAILED(hr)) {
		std::cout << "CreateUnorderedAccessView() failed. " << std::hex << hr << std::endl;
		return nullptr; // 에러 처리
	}

	return uav;

}
/// <summary>
/// 
/// </summary>
/// <param name="_pDevice"></param>
/// <param name="_structuredBuffer"></param>
/// <param name="_count">요소의 개수 / -1 일경우 srvDesc는 nullptr로 생성된다.</param>
/// <returns></returns>
ComPtr<ID3D11ShaderResourceView> RendererHelper::CreateShaderResourceView(ComPtr<ID3D11Device>& _pDevice, ComPtr<ID3D11Buffer> _structuredBuffer, std::optional<unsigned int> _count)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.BufferEx.NumElements = _count.value_or(0);		//값이 없으면 0 사용
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
		return nullptr; // 에러 처리
	}
	return srv;
}



ComPtr<ID3D11ShaderResourceView> RendererHelper::Create3DTexture(ComPtr<ID3D11Device>& _pDevice, std::string _filename)
{
	//RGB 형식의 이미지를 RGBA 형식으로 변환. stb_image.h를 사용하여 이미지를 로드하고, 변환된 데이터를 std::vector<uint8_t>에 저장
	int width(0), height(0), channels(0);												// 이미지의 폭(width), 높이(height), 채널 수(channels)를 나타내는 변수를 선언하고 초기값을 0으로 설정
	unsigned char* img = stbi_load(_filename.c_str(), &width, &height, &channels, 0);	//지정된 파일(filename)에서 이미지를 읽어와서 img 포인터에 저장/ width, height, channels 변수에는 이미지의 크기와 채널 수가 설정됨
	if (!img)
	{
		DEBUG_LOG("Failed to load texture: " << _filename);
		return nullptr; // 에러 처리 추가
	}

	std::vector<uint8_t> image;										//변환된 이미지 데이터를 저장할 벡터를 정의
	image.resize(width * height * 4);								//이미지 데이터를 저장할 벡터(image)의 크기를 이미지의 폭(width), 높이(height), 4개의 채널(RGBA)을 고려하여 설정

	for (size_t pixel = 0; pixel < width * height; pixel++)			// 이미지의 모든 픽셀에 대해 반복하는 루프
	{
		for (size_t rgb = 0; rgb < 3; rgb++)						// 각 픽셀의 RGB 값을 처리하는 루프
		{
			image[4 * pixel + rgb] = img[pixel * channels + rgb];	// 현재 픽셀의 RGB 값을 image 벡터에 저장. 4 * pixel + rgb는 각 픽셀에서 R, G, B 각 채널에 해당하는 인덱스를 계산
		}
		image[4 * pixel + 3] = 255;									//각 픽셀의 Alpha 채널을 255(불투명)로 설정
	}

	stbi_image_free(img); // 이미지 메모리 해제

	//텍스처 데이터의 설명을 설정.
	D3D11_TEXTURE2D_DESC texDesc = {};				// 모든 멤버를 0 또는 null로 초기화
	texDesc.Width = width;							// 텍스처의 가로 크기를 이미지의 가로 크기(width)로 설정
	texDesc.Height = height;						// 텍스처의 세로 크기를 이미지의 세로 크기(height)로 설정
	texDesc.MipLevels = texDesc.ArraySize = 1;		// 텍스처의 미세한 수준 수 및 배열의 크기를 1로 설정 (사용 안함)
	// cf. MipLevels : 텍스처의 해상도를 줄여 성능을 향상시키는 데 사용, 원본 텍스처의 해상도를 반으로 줄여가면서 생성, 멀리 떨어진 객체에 대한 텍스처 매핑을 더 효과적으로 처리
	// cf. ArraySize : 여러 개의 텍스처를 하나의 텍스처로 취급하는 기능, 텍스처 배열을 사용하면 다른 크기 또는 다른 이미지의 여러 버전을 하나의 텍스처로 쉽게 사용
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// 텍스처의 픽셀 형식을 32비트 RGBA 형식으로 지정
	texDesc.SampleDesc.Count = 1;					// 멀티샘플링 수를 1로 설정 : 멀티 샘플링 사용 X
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;			// 텍스처의 사용 방법 : 텍스처가 변하지 않음을 의미하며, 초기화 후에는 수정되지 않는다
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // 텍스처를 셰이더 리소스로 바인딩하도록 설정

	// Fill in the subresource data.
	D3D11_SUBRESOURCE_DATA InitData;				// 텍스처의 초기 데이터를 설정
	InitData.pSysMem = image.data();				// 초기 데이터로 이미지 벡터의 데이터 포인터를 설정
	InitData.SysMemPitch = texDesc.Width * sizeof(uint8_t) * 4; // 이미지의 가로 행 사이의 바이트 단위 거리를 설정 

	ComPtr<ID3D11Texture2D> pTexture;
	ComPtr<ID3D11ShaderResourceView> pTextureResourceView;

	HRESULT hr = _pDevice->CreateTexture2D(&texDesc, &InitData, pTexture.GetAddressOf());
	if (FAILED(hr))
	{
		DEBUG_LOG("CreateTexture2D() failed. " << std::hex << hr);
		return nullptr; // 에러 처리
	}
	hr = _pDevice->CreateShaderResourceView(pTexture.Get(), nullptr, pTextureResourceView.GetAddressOf());
	if (FAILED(hr))
	{
		DEBUG_LOG("CreateShaderResourceView() failed. " << std::hex << hr);
		return nullptr; // 에러 처리
	}

	return pTextureResourceView;
}

ComPtr<ID3D11ShaderResourceView> RendererHelper::Create2DTexture(ComPtr<ID3D11Device>& _pDevice, std::string _filename)
{
	//RGB 형식의 이미지를 RGBA 형식으로 변환. stb_image.h를 사용하여 이미지를 로드하고, 변환된 데이터를 std::vector<uint8_t>에 저장
	int width(0), height(0);												// 이미지의 폭(width), 높이(height), 채널 수(channels)를 나타내는 변수를 선언하고 초기값을 0으로 설정
	unsigned char* img = stbi_load(_filename.c_str(), &width, &height, NULL, 4);	//지정된 파일(filename)에서 이미지를 읽어와서 img 포인터에 저장/ width, height, channels 변수에는 이미지의 크기와 채널 수가 설정됨
	if (!img)
	{
		std::cout << "이미지 로드에 실패했습니다." << std::endl;
		return nullptr;
	}

	//텍스처 데이터의 설명을 설정.
	D3D11_TEXTURE2D_DESC texDesc = {};				// 모든 멤버를 0 또는 null로 초기화
	texDesc.Width = width;							// 텍스처의 가로 크기를 이미지의 가로 크기(width)로 설정
	texDesc.Height = height;						// 텍스처의 세로 크기를 이미지의 세로 크기(height)로 설정
	texDesc.MipLevels = texDesc.ArraySize = 1;		// 텍스처의 미세한 수준 수 및 배열의 크기를 1로 설정 (사용 안함)
	// cf. MipLevels : 텍스처의 해상도를 줄여 성능을 향상시키는 데 사용, 원본 텍스처의 해상도를 반으로 줄여가면서 생성, 멀리 떨어진 객체에 대한 텍스처 매핑을 더 효과적으로 처리
	// cf. ArraySize : 여러 개의 텍스처를 하나의 텍스처로 취급하는 기능, 텍스처 배열을 사용하면 다른 크기 또는 다른 이미지의 여러 버전을 하나의 텍스처로 쉽게 사용
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// 텍스처의 픽셀 형식을 32비트 RGBA 형식으로 지정
	texDesc.SampleDesc.Count = 1;					// 멀티샘플링 수를 1로 설정 : 멀티 샘플링 사용 X
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;			// 텍스처의 사용 방법 : 텍스처가 변하지 않음을 의미하며, 초기화 후에는 수정되지 않는다
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // 텍스처를 셰이더 리소스로 바인딩하도록 설정
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
	// std::string을 std::wstring으로 변환
	std::wstring wfilename = std::wstring(_filename.begin(), _filename.end());

	ComPtr<ID3D11Texture2D> pTexture;
	ComPtr<ID3D11ShaderResourceView> pTextureResourceView;

	auto hr = DirectX::CreateDDSTextureFromFileEx(
		_pDevice.Get(), wfilename.c_str(), 0, D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE, 0,
		D3D11_RESOURCE_MISC_TEXTURECUBE, // 큐브맵용 텍스춰
		DirectX::DX11::DDS_LOADER_FLAGS(false), (ID3D11Resource**)pTexture.GetAddressOf(),
		pTextureResourceView.GetAddressOf(), nullptr);

	if (FAILED(hr)) {
		std::cout << "CreateDDSTextureFromFileEx() failed" << std::endl;
		return nullptr;
	}
	return pTextureResourceView;
}

///그림자용 shadowMap Texture와 SPV, DSV를 생성한다.
bool RendererHelper::CreateShadowMapAndShaderResourceView(ComPtr<ID3D11Device>& _pDevice, int _screenWidth, int _screenHeight,
	ComPtr<ID3D11DepthStencilView>& _pShadowMap, ComPtr<ID3D11ShaderResourceView>& _pShadowResourceView)
{
	D3D11_TEXTURE2D_DESC depthBufferDesc = {};								   //깊이 스탠실 버퍼의 특성을 설명하는 구조체를 초기화한다.
	depthBufferDesc.Width = _screenWidth;
	depthBufferDesc.Height = _screenHeight;
	depthBufferDesc.MipLevels = 1;											   //밉맵은 텟츠쳐의 여러 해상도를 사용하는 것이지만, 깊이버퍼에서는 필요하지 않다.
	depthBufferDesc.ArraySize = 1;											   //배열의 크기를 1로 설정(단일 텍스쳐), 섀도우 맵은 보통 단일 텍스쳐로 사용된다. 
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;							   //기본 사용으로 설정하여 GPU가 읽고 쓰는 용도로 사용
	depthBufferDesc.Format = DXGI_FORMAT_R32_TYPELESS;

	depthBufferDesc.SampleDesc.Count = 1;									   //일반적으로 섀도우맵에서는 멀티 샘플링을 사용하지 않기에 1로 설정(비활성화)
	depthBufferDesc.SampleDesc.Quality = 0;									   //멀티샘플링 품질을 0으로 설정
	depthBufferDesc.CPUAccessFlags = 0;										   //CPU.엑세스 비활성화
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;		//깊이 스탠실 뷰 와 셰이더 리소스 뷰로 모두 사용가능하도록 설정. 즉, 이 텍스쳐는 깊이버퍼로 사용될수 있고, 셰이더에서 읽기용도로 사용될 수도 있다.
	depthBufferDesc.MiscFlags = 0;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;

	ComPtr<ID3D11Texture2D> shadowDepthBuffer;
	// 디버깅
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

/// inputElements정보를 바탕으로 m_vertexShader, m_inputLayout을 세팅한다.
void RendererHelper::CreateVertexShaderAndInputLayout(ComPtr<ID3D11Device>& device, const std::string& filename, const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputElements,
	ComPtr<ID3D11VertexShader>& m_vertexShader, ComPtr<ID3D11InputLayout>& m_inputLayout)
{
	ComPtr<ID3DBlob> shaderBlob;
	ComPtr<ID3DBlob> errorBlob;

	UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	// 쉐이더의 시작점의 이름이 "main"인 함수로 지정
	// D3D_COMPILE_STANDARD_FILE_INCLUDE 추가: 쉐이더에서 include 사용
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
		// 파일이 없을 경우
		if ((hr & D3D11_ERROR_FILE_NOT_FOUND) != 0)
		{
			DEBUG_LOG("File not found.");
		}

		// 에러 메시지가 있으면 출력
		if (errorBlob)
		{
			DEBUG_LOG("Shader compile error\n" << (char*)errorBlob->GetBufferPointer());
			errorBlob->Release(); // 오류 블롭 해제
		}
	}
}


void RendererHelper::CreatePixelShader(ComPtr<ID3D11Device>& device,
	const std::string& filename,
	ComPtr<ID3D11PixelShader>& _pixelShader)
{
	ComPtr<ID3DBlob> shaderBlob;	//D3D 컴파일러가 컴파일한 셰이더 코드의 바이트 코드가 저장
	ComPtr<ID3DBlob> errorBlob;		//오류메세지를 저장하는데 사용

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	dwShaderFlags |= D3DCOMPILE_DEBUG;
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;

#endif // _DEBUG


	//셰이더의 이름이 시작점이 "main"인 함수로 지정
	HRESULT hr = D3DCompileFromFile(
		StringToWstring(filename).c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
		"ps_5_0", dwShaderFlags, 0, &shaderBlob, &errorBlob);	//주어진 파일에서 픽셸 쉐이더를 컴파일.


	CheckResult(hr, errorBlob.Get());																					//함수의 실행결과를 확인

	device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, &_pixelShader);		//픽셸 쉐이더 생성

}

void RendererHelper::CreateComputeShader(ComPtr<ID3D11Device>& device, const std::string& filename, ComPtr<ID3D11ComputeShader>& _computeShader)
{
	{
		ComPtr<ID3DBlob> shaderBlob;				//컴파일된 셰이더 코드를 저장할 ID3DBlob 객체, 컴파일된 셰이더 데이터가 여기에 저장된다.
		ComPtr<ID3DBlob> errorBlob;					//컴파일 과정에서 발생한 에러 정보를 저장하는 ID3DBlob객체, 에러발생시 여기에 오류메세지를 담는다.

		UINT compileFlags = 0;						//셰이더 컴파일 시 사용할 플래그를 지정한다.
#if defined(DEBUG) || defined(_DEBUG)
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;			//디버그 빌드라면 두개의 플래그를 추가한다.
		///D3DCOMPILE_DEBUG				: 디버그 정보를 포함시킨다.
		///D3DCOMPILE_SKIP_OPTIMIZATION	: 최적화를 생략해 디버깅을 쉽게한다.
#endif

		// 컴퓨트 셰이더 파일을 컴파일하여 Blob에 저장
		HRESULT hr = D3DCompileFromFile(
			StringToWstring(filename).c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"main",									// 컴퓨트 셰이더의 시작 함수 이름
			"cs_5_0",								// 컴퓨트 셰이더의 타겟 프로파일
			compileFlags, 0, &shaderBlob, &errorBlob);

		CheckResult(hr, errorBlob.Get());

		// 컴퓨트 셰이더 생성
		hr = device->CreateComputeShader(
			shaderBlob->GetBufferPointer(),			//컴파일된 셰이더 코드의 시작주소
			shaderBlob->GetBufferSize(),			//셰이더 코드의 크기(바이트 단위)
			nullptr,								//nullptr : 기본 인터페이스를 사용하겠다.
			&_computeShader);

#if _DEBUG
		if (FAILED(hr)) {
			DEBUG_LOG(filename << " failed to create Compute Shader.");
		}
#endif
	}
}
