#include "PipelineStateObject.h"

///-------------------------------------------------------�� ������ ����. ���߿� Ȯ���غ���.
// operator=�� ���� �� ���ǵǾ��� �����̴�. 
// ��� ���Ͽ� ���ǵ� �Լ��� ���� �� ���Ե� �� �ֱ� ������, 
// �Լ� ���Ǹ� �ҽ� ���Ϸ� �и��Ͽ� �ߺ� ���Ǹ� �����ϴ� ���� �߿��ϴ�. -by Riun

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