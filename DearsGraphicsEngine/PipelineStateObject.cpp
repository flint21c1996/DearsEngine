#include "PipelineStateObject.h"

///-------------------------------------------------------이 밑으로 경고뜸. 나중에 확인해볼것.
// operator=가 여러 번 정의되었기 때문이다. 
// 헤더 파일에 정의된 함수가 여러 번 포함될 수 있기 때문에, 
// 함수 정의를 소스 파일로 분리하여 중복 정의를 방지하는 것이 중요하다. -by Riun

void PipelineStateObject::operator=(const PipelineStateObject& _pso)
{
	m_pVertexShader = _pso.m_pVertexShader;
	m_pPixelShader = _pso.m_pPixelShader;
	m_pHullShader = _pso.m_pHullShader;
	m_pDomainShader = _pso.m_pDomainShader;
	m_pGeometryShader = _pso.m_pGeometryShader;
	m_pInputLayout = _pso.m_pInputLayout;

	m_pBlendState = _pso.m_pBlendState;
	m_pDepthStencilState = _pso.m_pDepthStencilState;
	m_pRasterizerState = _pso.m_pRasterizerState;
	m_pSamplerState = _pso.m_pSamplerState;

	int blendFactorsize = sizeof(m_blendFactor) / sizeof(m_blendFactor[0]);
	for (int i = 0; i < blendFactorsize; i++)
	{
		m_blendFactor[i] = _pso.m_blendFactor[i];
	}

	m_primitiveTopology = _pso.m_primitiveTopology;
}

void PipelineStateObject::SetBlendFactor(const float blendFactor[4])
{
	int blendFactorsize = sizeof(m_blendFactor) / sizeof(m_blendFactor[0]);
	for (int i = 0; i < blendFactorsize; i++)
	{
		m_blendFactor[i] = blendFactor[i];
	}
}