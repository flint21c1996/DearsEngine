#pragma once

#include <d3d11.h>
#include "wrl.h"
using Microsoft::WRL::ComPtr;

//PipeLineState : 렌더링할 떄 Context의 상태 설정
class PipelineState
{
public:
	ComPtr<ID3D11InputLayout> m_pInputLayout;
	ComPtr<ID3D11VertexShader> m_pVertexShader;

	ComPtr<ID3D11HullShader> m_pHullShader;
	ComPtr<ID3D11DomainShader> m_pDomainShader;
	ComPtr<ID3D11GeometryShader> m_pGeometryShader;

	ComPtr<ID3D11PixelShader> m_PixelShader;
	
	
	ComPtr<ID3D11BlendState> m_BlendState;

	ComPtr<ID3D11DepthStencilState> m_DepthStencilState;

	ComPtr<ID3D11RasterizerState> m_RasterizerState;

	float m_blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	UINT m_stencilRef = 0;								   //스탠실 참조값

	D3D11_PRIMITIVE_TOPOLOGY m_primitiveTopology =
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

public:
	void operator=(const PipelineState& _ps);
	void SetBlendFactor(const float blendFactor[4]);
};


