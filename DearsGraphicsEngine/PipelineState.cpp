#include "PipelineState.h"

///-------------------------------------------------------�� ������ ����. ���߿� Ȯ���غ���.
// operator=�� ���� �� ���ǵǾ��� �����̴�. 
// ��� ���Ͽ� ���ǵ� �Լ��� ���� �� ���Ե� �� �ֱ� ������, 
// �Լ� ���Ǹ� �ҽ� ���Ϸ� �и��Ͽ� �ߺ� ���Ǹ� �����ϴ� ���� �߿��ϴ�. -by Riun

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