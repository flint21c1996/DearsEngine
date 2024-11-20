#pragma once

#include <d3d11.h>
#include "wrl.h"
using Microsoft::WRL::ComPtr;

//PipeLineState : 렌더링할 떄 Context의 상태 설정
/// 2024.5.20
/// Riun PipelineState라고 하면 약자가 PS라서 pixel shader와 겹치기 때문에 Object를 붙였다. 일반적으로 PSO로 불린다.
/// 또한 PipelineState는 단순한 상태 개념이고, 실제 적용하는 개체는 PipelineStateObject로 일반적으로 불린다.
class PipelineStateObject
{
public:
	ComPtr<ID3D11InputLayout> m_pInputLayout;
	ComPtr<ID3D11VertexShader> m_pVertexShader;
	ComPtr<ID3D11PixelShader> m_pPixelShader;

	ComPtr<ID3D11HullShader> m_pHullShader;
	ComPtr<ID3D11DomainShader> m_pDomainShader;
	ComPtr<ID3D11GeometryShader> m_pGeometryShader;

	ComPtr<ID3D11BlendState> m_pBlendState;

	ComPtr<ID3D11DepthStencilState> m_pDepthStencilState;

	ComPtr<ID3D11RasterizerState> m_pRasterizerState;

	ComPtr<ID3D11SamplerState> m_pSamplerState;	


	float m_blendFactor[4] = { 1.f, 1.f, 1.f, 1.f };		
	UINT m_stencilRef = 0;								   //스탠실 참조값

	D3D11_PRIMITIVE_TOPOLOGY m_primitiveTopology =
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

public:
	void operator=(const PipelineStateObject& _pso);
	void SetBlendFactor(const float blendFactor[4]);
};

// 블렌드 펙터(m_blendFactor[4])란
// 블렌딩 연산에서 소스 또는 대상 색상에 곱해지는 값
// 특히 D3D11_BLEND_BLEND_FACTOR, D3D11_BLEND_INV_BLEND_FACTOR 처럼 명시적으로 블렌드 팩터를 사용하는 블렌더 모드를 설절할 떄 사용.
// 이 값은 일반적으로 [R, G, B, A] 형태로 나타내며, 각각의 채널에 곱해져 색상을 조절하게 된다.
// ex) m_blendFactor[4] = [0.5, 0.5, 0.5, 0.5]일 경우 소스 또는 대상 색상의 각 채널 (R, G, B, A)에 0.5를 곱해 결과적으로 색상이 절반으로 희석되는 효과를 줄 수 있다. 
// 
// 
//
