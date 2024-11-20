#include "PipelineState.h"

///-------------------------------------------------------이 밑으로 경고뜸. 나중에 확인해볼것.
// operator=가 여러 번 정의되었기 때문이다. 
// 헤더 파일에 정의된 함수가 여러 번 포함될 수 있기 때문에, 
// 함수 정의를 소스 파일로 분리하여 중복 정의를 방지하는 것이 중요하다. -by Riun

void PipelineState::operator=(const PipelineState& _ps)
{
	m_pVertexShader = _ps.m_pVertexShader;
	m_PixelShader = _ps.m_PixelShader;
	m_pHullShader = _ps.m_pHullShader;
	m_pDomainShader = _ps.m_pDomainShader;
	m_pGeometryShader = _ps.m_pGeometryShader;
	m_pInputLayout = _ps.m_pInputLayout;

	m_BlendState = _ps.m_BlendState;
	m_DepthStencilState = _ps.m_DepthStencilState;
	m_RasterizerState = _ps.m_RasterizerState;

	int blendFactorsize = sizeof(m_blendFactor) / sizeof(m_blendFactor[0]);
	for (int i = 0; i < blendFactorsize; i++)
	{
		m_blendFactor[i] = _ps.m_blendFactor[i];
	}

	m_primitiveTopology = _ps.m_primitiveTopology;
}

void PipelineState::SetBlendFactor(const float blendFactor[4])
{
	int blendFactorsize = sizeof(m_blendFactor) / sizeof(m_blendFactor[0]);
	for (int i = 0; i < blendFactorsize; i++)
	{
		m_blendFactor[i] = blendFactor[i];
	}
}