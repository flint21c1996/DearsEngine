#include "PostProcessRenderer.h"

#include "DearsGraphicsEngine.h"
#include "GraphicsCommon.h"
#include "Renderer.h"

PostProcessRenderer::PostProcessRenderer(DearsGraphicsEngine* graphicsEngine)
	: m_pGraphicsEngine(graphicsEngine)
{
}

void PostProcessRenderer::Initialize()
{
	m_pPostProcessingBuffer = std::make_unique<ModelBuffer>();
	m_pPostProcessingBuffer->m_pVertexBuffer = m_pGraphicsEngine->Get_VertexBuffer("BillBoardSquare");
	m_pPostProcessingBuffer->m_pIndexBuffer = m_pGraphicsEngine->Get_IndexBuffer("BillBoardSquare");
	m_pPostProcessingBuffer->mNumIndices = m_pGraphicsEngine->Get_NumIndex("BillBoardSquare");
}

void PostProcessRenderer::Render()
{
	// 현재 후처리는 두 단계로 되어 있다.
	// 1. 메인 렌더 타겟을 작은 임시 버퍼에 샘플링한다.
	// 2. 임시 버퍼를 다시 메인 렌더 타겟에 그리면서 후처리 픽셀 셰이더를 적용한다.
	//
	// 기존 코드가 10번 반복하도록 되어 있어서 동작을 그대로 유지한다.
	// 나중에 효과별 패스가 명확해지면 반복 횟수도 설정값이나 패스 목록으로 바꾸는 것이 좋다.
	m_pGraphicsEngine->mpRenderer->SetPipelineState(Dears::Graphics::samplerPSO);
	for (int i = 0; i < 10; i++)
	{
		m_pGraphicsEngine->mpRenderer->RenderSampler(m_pPostProcessingBuffer.get());

		m_pGraphicsEngine->mpRenderer->SetPipelineState(Dears::Graphics::postEffectPSO);
		m_pGraphicsEngine->mpRenderer->RenderPostProcessing(m_pPostProcessingBuffer.get());
	}
}
