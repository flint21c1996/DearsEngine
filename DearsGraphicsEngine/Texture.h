#pragma once
#include <wrl/client.h>
#include <d3d11.h>

using Microsoft::WRL::ComPtr;

/*
ID3D11Texture2D 객체는 텍스처의 실제 데이터를 GPU에 저장하는 반면,
ID3D11ShaderResourceView는 그 텍스처 데이터를 셰이더 프로그램에서 사용할 수 있도록 하는 인터페이스 역할
*/
struct Texture
{
	//2D 텍스쳐를 나타낸다.
	ComPtr<ID3D11Texture2D> mpTexture;

	//텍스쳐를 셰이더에서 사용할 수 있도록 하는 뷰를 나타낸다. 이를 통해 버텍스 셰이더나 픽셀 세이더에서 해당 텍스쳐를 읽을 수 있다.
	ComPtr<ID3D11ShaderResourceView> mpTextureResourceView;
};
