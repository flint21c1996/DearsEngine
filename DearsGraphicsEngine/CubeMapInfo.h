#pragma once
#include<wrl.h>
#include<d3d11.h>

using Microsoft::WRL::ComPtr;
struct CubeMapInfo
{
	ComPtr<ID3D11ShaderResourceView> mpCubeMapResourceView;
	ComPtr<ID3D11PixelShader> mpPixelShader;
	ComPtr<ID3D11VertexShader> mpVertexShader;
	ComPtr<ID3D11InputLayout> mpInputLayout;
};