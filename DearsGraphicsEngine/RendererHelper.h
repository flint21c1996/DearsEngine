#pragma once
#include <wrl.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>                       // DXGIFactory
#include <dxgi1_4.h>                    // DXGIFactory4
#include <string>
#include <directxtk/DDSTextureLoader.h> // 큐브맵 읽을 때 필요
#include <vector>
using Microsoft::WRL::ComPtr;
#include <iostream>
#include <optional>
#ifdef _DEBUG
#define DEBUG_LOG(message) std::cout<<message<<std::endl
#else
#define DEBUG_LOG(message)
#endif // DEBUG


namespace Dears
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr)) {
			// 디버깅할 때 여기에 중단점 찍으면 편함
			throw std::exception();
		}
	}
}

/// 렌더러의 세팅, 수정을 도와주는 클래스
// 2024.5.20. Riun 수정사항 : 포인터를 매개변수로 그냥 가져와서, 원본이 아닌 복사본이 초기화되었다...
// 매개변수를 참조로 가져오도록 바꾸었다.
class RendererHelper
{
public:
	RendererHelper();
	~RendererHelper();

public:
	///디바이스를 생성한다.
	static bool CreateDevice(ComPtr<ID3D11Device>& _pDevice, ComPtr<ID3D11DeviceContext>& _pDeviceContext);

	///IDXGIFactory를 이용한 CreateSwapChain.
	static bool CreateSwapChain(ComPtr<ID3D11Device>& _pDevice,
		HWND _hWnd, UINT& _numQualityLevels, int _screenWidth, int _screenHeight, ComPtr<IDXGISwapChain>& _pSwapChain);

	///Device내의 RenderTargetView를 생성한다. 
	static bool CreateRenderTargetView(ComPtr<ID3D11Device>& _pDevice,
		ComPtr<IDXGISwapChain>& _pSwapChain,
		ComPtr<ID3D11RenderTargetView>& _pRenderTargetView,
		ComPtr < ID3D11Texture2D>& _pBuffer,
		UINT _index = 0,
		UINT _screenWidth = 0,
		UINT _screenHeight = 0
	);

	///DeviceContext내의 ViewPort를 세팅한다.
	static void SetViewPort(ComPtr<ID3D11DeviceContext>& _pDeviceContext,
		int _startScreenWidth, int _startScreenHeight, int _endScreenWidth, int _endScreenHeight,
		D3D11_VIEWPORT& _screenViewPort);

	///그림자 구현에 필요한 뷰포트를 세팅한다.
	static void SetShadowViewport(ComPtr<ID3D11DeviceContext>& _pDeviceContext, float m_shadowWidth, float m_shadowHeight,
		D3D11_VIEWPORT& _screenViewPort);

	///Device내의 RasterizerState를 생성한다. 
	static ComPtr<ID3D11RasterizerState> CreateSolidRasterizerState(ComPtr<ID3D11Device>& _pDevice);
	static ComPtr<ID3D11RasterizerState> CreateWireRasterizerState(ComPtr<ID3D11Device>& _pDevice);

	/// Create Depth Buffer state (view)
	static bool CreateDepthStencilBuffer(ComPtr<ID3D11Device>& _pDevice,
		UINT& _numQualityLevels,
		int _screenWidth, int _screenHeight,
		ComPtr<ID3D11DepthStencilView>& _pdepthStencilView);

	/// Create Depth Buffer state (view)
	static bool CreateDepthOnlyBuffer(ComPtr<ID3D11Device>& _pDevice,
		int _screenWidth, int _screenHeight,
		ComPtr<ID3D11DepthStencilView>& _pdepthStencilView,
		ComPtr<ID3D11ShaderResourceView>& _pdepthOnlySRV);


	//VertexBuffer를 만든다. 
	template<typename T_VERTEX>
	static ComPtr<ID3D11Buffer> CreateVertexBuffer(ComPtr<ID3D11Device>& _pDevice, unsigned int _mNumVertices, T_VERTEX* _vertices);

	//IndexBuffer를 만든다. 
	static ComPtr<ID3D11Buffer> CreateIndexBuffer(ComPtr<ID3D11Device>& _pDevice, const unsigned int _numIndices, unsigned int* _indices);

	//ConstantBuffer를 만든다.
	template<typename T_Constant>
	static ComPtr<ID3D11Buffer> CreateConstantBuffer(ComPtr<ID3D11Device>& _pDevice, const T_Constant& _constantBufferStruct);

	//StructedBuffer을 만든다.
	template<typename T_Struct>
	static ComPtr<ID3D11Buffer> CreateStructuredBuffer(ComPtr<ID3D11Device>& _pDevice, const T_Struct& _constantBufferStruct, unsigned int _count);

	//UAV를 만든다
	static ComPtr<ID3D11UnorderedAccessView> CreateUnorderedAccessView(ComPtr<ID3D11Device>& _pDevice, ComPtr<ID3D11Buffer> _structuredBuffer, unsigned int _count);

	//SRV를 만든다
	static ComPtr<ID3D11ShaderResourceView> CreateShaderResourceView(ComPtr<ID3D11Device>& _pDevice, ComPtr<ID3D11Buffer> _structuredBuffer, std::optional<unsigned int> _count = std::nullopt);

	//스테이징 버퍼를 복사한다.
	template<typename S_Struct>
	static void CopyFromStagingBuffer(ComPtr<ID3D11DeviceContext>& context, ComPtr<ID3D11Buffer>& buffer, UINT _size, S_Struct* destination);

	//스테이징 버퍼의 내용을 structuredbuffer -> GPU로 복사한다.
	template<typename S_Struct>
	static void UpdateDataOnGPU(ComPtr<ID3D11DeviceContext>& context,
		ComPtr<ID3D11Buffer>& structuredBuffer,  // UAV로 바인딩된 GPU 버퍼
		ComPtr<ID3D11Buffer>& stagingBuffer,      // CPU 접근용 스테이징 버퍼
		UINT _size,
		S_Struct* destination);



	//스테이징 버퍼를 만든다.
	template<typename S_Struct>
	static ComPtr<ID3D11Buffer>CreateStagingBuffer(ComPtr<ID3D11Device>& _pDevice, const unsigned int numElements, const unsigned int sizeElement, S_Struct* _stagingStruct);

	//Texture를 만든다. 만들어서 그래픽스 컨테이너로 보내야한다. -> 이거 나중에 다른곳으로 빼는게 맞는거 같다.
	static ComPtr<ID3D11ShaderResourceView> Create3DTexture(ComPtr<ID3D11Device>& _pDevice, std::string _filename);
	static ComPtr<ID3D11ShaderResourceView> Create2DTexture(ComPtr<ID3D11Device>& _pDevice, std::string _filename);
	static ComPtr<ID3D11ShaderResourceView> CreateDDSTexture(ComPtr<ID3D11Device>& _pDevice, std::string _filename);

	//버퍼를 업데이트한다. 대부분 ConstantBuffer을 업데이트함.
	template<typename T_Constant>
	static void UpdateBuffer(ComPtr<ID3D11DeviceContext>& _pDeviceContext, const T_Constant& _ConstantBufferStruct, ComPtr<ID3D11Buffer>& _buffer);

	//그림자 맵과, 리소스뷰를 만든다.
	static bool CreateShadowMapAndShaderResourceView(ComPtr<ID3D11Device>& _pDevice, int _screenWidth, int _screenHeight,
		ComPtr<ID3D11DepthStencilView>& _pShadowMap, ComPtr<ID3D11ShaderResourceView>& _pShadowResourceView);

	// Create depth stencil state
	static ComPtr<ID3D11DepthStencilState> CreateDepthStencilState(ComPtr<ID3D11Device>& _pDevice);

	//샘플러스테이트를 만든다.
	static ComPtr<ID3D11SamplerState> CreateSamplerState(ComPtr<ID3D11Device>& _pDevice);

	//버텍스 세이더와 inputlayout을 만든다
	static void CreateVertexShaderAndInputLayout(
		ComPtr<ID3D11Device>& device, const std::string& filename,
		const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputElements,
		ComPtr<ID3D11VertexShader>& m_vertexShader,
		ComPtr<ID3D11InputLayout>& m_inputLayout);

	//픽셀셰이더를 만든다.
	static void CreatePixelShader(ComPtr<ID3D11Device>& device, const std::string& filename, ComPtr<ID3D11PixelShader>& _pixelShader);

	static void CreateComputeShader(ComPtr<ID3D11Device>& device, const std::string& filename, ComPtr<ID3D11ComputeShader>& _computeShader);

	//string -> wstring 변환
	static std::wstring StringToWstring(const std::string& str);

	//오류를 체크한다.
	static void CheckResult(HRESULT hr, ID3DBlob* errorBlob);
};

template<typename T_VERTEX>
static ComPtr<ID3D11Buffer>
RendererHelper::CreateVertexBuffer(ComPtr<ID3D11Device>& _pDevice, unsigned int _mNumVertices, T_VERTEX* _vertices)
{
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
	bufferDesc.ByteWidth = UINT(sizeof(T_VERTEX) * _mNumVertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
	bufferDesc.StructureByteStride = sizeof(T_VERTEX);

	D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 }; // MS 예제에서 초기화하는 방식
	vertexBufferData.pSysMem = _vertices;
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;

	ComPtr<ID3D11Buffer> vertexBuffer;

	HRESULT hr =
		_pDevice->CreateBuffer(&bufferDesc, &vertexBufferData, vertexBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		std::cout << "CreateVertexBuffer() failed. " << std::hex << hr << std::endl;
		return nullptr;
	};
	return vertexBuffer;
}

template<typename T_Constant>
ComPtr<ID3D11Buffer>
RendererHelper::CreateConstantBuffer(ComPtr<ID3D11Device>& _pDevice, const T_Constant& _constantBufferStruct)
{
	// 버텍스 컨스턴트 버퍼 생성
	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(_constantBufferStruct);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	// Fill in the subresource data.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = &_constantBufferStruct;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	ComPtr<ID3D11Buffer> pConstantBuffer;

	HRESULT hr = _pDevice->CreateBuffer(&cbDesc, &InitData, pConstantBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		std::cout << "CreateConstantBuffer() failed. " << std::hex << hr << std::endl;
		return nullptr; // 에러 처리
	}
	return pConstantBuffer;
}

//StructuredBuffer
//D3D에서 구조화된 데이터를 GPU에 저장하기 위한 버퍼, 구조체 형태로 구성된 데이터의 배열을 GPU에 저장하여 셰이더에서 개별 항목을 접근할 수 있도록 설계

//UAV = Unordered Access View
//D3D에서 GPU가 ㅏ메모리에 순서 없이 자유롭게 접근하여 읽고 쓸 수 있도록 하는 뷰 /  
//주로 컴퓨트 세이더에서 사용되며, 데이터를 읽고 쓰는 순서가 정해져 있지 않아 병렬로 여러 쓰레드에서 동시에 데이터에 접근하고 처리할수 있다. 

template<typename T_Struct>
ComPtr<ID3D11Buffer> RendererHelper::CreateStructuredBuffer(ComPtr<ID3D11Device>& _pDevice, const T_Struct& _constantBufferStruct, unsigned int _count)
{
	// 파티클 버퍼 생성
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;												// GPU가 주로 사용하고, CPU에서 수시로 업데이트하지 않음
	bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS |								//버퍼를 UAV로 사용할 수 있도록	//Compute Shader
		D3D11_BIND_SHADER_RESOURCE;														// 버퍼를 SRV로 사용할 수 있도록	//Vertex Shader
	bufferDesc.ByteWidth = sizeof(_constantBufferStruct) * _count;						//개수에 맞춰 버퍼 크기 설정
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;						//Structured buffer로 쓸 것이다.
	bufferDesc.StructureByteStride = sizeof(_constantBufferStruct);						// 각 구조체의 크기 설정, 한번 게산할때 얼마만큼 읽는가

	///참고로 sturectured buffer은 D3D11_BIND_VERTEX_BUFFER로 사용 불가.

	ComPtr<ID3D11Buffer> structuredBuffer;
	HRESULT hr = _pDevice->CreateBuffer(&bufferDesc, nullptr, structuredBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		std::cout << "CreateStructuredBuffer() failed. " << std::hex << hr << std::endl;
		return nullptr; // 에러 처리
	}
	return structuredBuffer;

}

/// <summary>
/// 
/// </summary>
/// <typeparam name="S_Struct"></typeparam>
/// <param name="_pDevice"></param>
/// <param name="_stagingdBuffer"></param>
/// <param name="numElements">요소의 개수</param>
/// <param name="sizeElement">요소의 사이즈 / sizeof(element)</param>
/// <param name="_destination">목적지</param>
/// <returns></returns>
template<typename S_Struct>
ComPtr<ID3D11Buffer> RendererHelper::CreateStagingBuffer(ComPtr<ID3D11Device>& _pDevice,
	const unsigned int numElements, const unsigned int sizeElement, S_Struct* _destination)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ByteWidth = numElements * sizeElement;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
	desc.StructureByteStride = sizeElement;

	ComPtr<ID3D11Buffer> stagingBuffer;
	//초기 값이 있을 경우
	if (_destination)
	{
		D3D11_SUBRESOURCE_DATA bufferData;
		ZeroMemory(&bufferData, sizeof(bufferData));
		bufferData.pSysMem = _destination;

		HRESULT hr = _pDevice->CreateBuffer(&desc, &bufferData, stagingBuffer.GetAddressOf());
		if (FAILED(hr))
		{
			std::cout << "CreateStagingBuffer() failed. " << std::hex << hr << std::endl;
			return nullptr;
		}
	}
	//초기값이 없을 경우 -> 이후 GPU나 CPU에서 데이터를 직접 설정하거나 수정해야한다. 
	else 
	{
		HRESULT hr = _pDevice->CreateBuffer(&desc, NULL, stagingBuffer.GetAddressOf());
	
		if (FAILED(hr))
		{
			std::cout << "CreateStagingBuffer() failed. " << std::hex << hr << std::endl;
			return nullptr;
		}
	}
	return stagingBuffer;

}

template<typename T_Constant>
void RendererHelper::UpdateBuffer(ComPtr<ID3D11DeviceContext>& _pDeviceContext, const T_Constant& _ConstantBufferStruct, ComPtr<ID3D11Buffer>& _buffer)
{
	D3D11_MAPPED_SUBRESOURCE ms;
	_pDeviceContext->Map(_buffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, &_ConstantBufferStruct, sizeof(_ConstantBufferStruct));
	_pDeviceContext->Unmap(_buffer.Get(), NULL);
}

/// <summary>
/// 
/// </summary>
/// <typeparam name="S_Struct"></typeparam>
/// <param name="context"></param>
/// <param name="buffer">복사될 데이터버퍼</param>
/// <param name="size">복사할 데이터의 크기</param>
/// <param name="destination">복사할 데이터의 주소값</param>
template<typename S_Struct>
void RendererHelper::CopyFromStagingBuffer(ComPtr<ID3D11DeviceContext>& context, ComPtr<ID3D11Buffer>& buffer, UINT _size, S_Struct* destination)
{
	D3D11_MAPPED_SUBRESOURCE ms;
	context->Map(buffer.Get(), NULL, D3D11_MAP_READ, NULL, &ms);
	memcpy(destination, ms.pData, _size);
	context->Unmap(buffer.Get(), NULL);
}

template<typename S_Struct>
void RendererHelper::UpdateDataOnGPU(ComPtr<ID3D11DeviceContext>& context, 
									ComPtr<ID3D11Buffer>& structuredBuffer,		// UAV로 바인딩된 GPU 버퍼
									ComPtr<ID3D11Buffer>& stagingBuffer,		// CPU 접근용 스테이징 버퍼
									UINT _size, S_Struct* destination)
{
	// 스테이징 버퍼에 데이터 매핑
	D3D11_MAPPED_SUBRESOURCE ms;
	HRESULT hr = context->Map(stagingBuffer.Get(), 0, D3D11_MAP_WRITE, 0, &ms);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to map staging buffer for write.");
	}

	// CPU 데이터 복사
	memcpy(ms.pData, destination,  _size);
	context->Unmap(stagingBuffer.Get(), 0);

	// UAV 구조화 버퍼로 데이터 복사
	context->CopyResource(structuredBuffer.Get(), stagingBuffer.Get());

}
