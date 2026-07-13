#include "MeshRenderer.h"

#include "DearsGraphicsEngine.h"
#include "GraphicsCommon.h"
#include "Renderer.h"

MeshRenderer::MeshRenderer(DearsGraphicsEngine* graphicsEngine)
	: m_pGraphicsEngine(graphicsEngine)
{
}

void MeshRenderer::RenderAnimatedModel(ModelBuffer* modelBuffer)
{
	// 스키닝 애니메이션용 정점 셰이더/입력 레이아웃이 묶인 PSO를 사용한다.
	m_pGraphicsEngine->SetPipelineState(Dears::Graphics::AnimeGeometryPSO);
	m_pGraphicsEngine->mpRenderer->Render(modelBuffer);
}

void MeshRenderer::RenderStaticModel(ModelBuffer* modelBuffer)
{
	// 가장 기본적인 정적 메시 렌더링 경로이다.
	// 지금 RenderDispatcher의 StaticMesh 타입은 최종적으로 여기로 들어온다.
	m_pGraphicsEngine->SetPipelineState(Dears::Graphics::BasicGeometryPSO);
	m_pGraphicsEngine->mpRenderer->Render(modelBuffer);
}

void MeshRenderer::RenderPbrModel(ModelBuffer* modelBuffer)
{
	// PBR 재질을 사용하는 메시 렌더링 경로이다.
	m_pGraphicsEngine->SetPipelineState(Dears::Graphics::PBRPSO);
	m_pGraphicsEngine->mpRenderer->Render(modelBuffer);
}

void MeshRenderer::RenderDeferredGeometry(ModelBuffer* modelBuffer)
{
	if (!m_isDeferredGeometryPipelineInitialized)
	{
		// 기존 PBR 정점 처리와 렌더 상태는 재사용하고, 조명 대신 MRT를 출력하는 PS만 교체한다.
		m_deferredGeometryPSO = Dears::Graphics::PBRPSO;
		RendererHelper::CreatePixelShader(
			m_pGraphicsEngine->m_pDevice,
			"../DearsGraphicsEngine/Shader/DeferredGeometryPixelShader.hlsl",
			m_deferredGeometryPSO.m_pPixelShader);

		// nullptr Blend State는 DX11 기본 상태, 즉 블렌딩 비활성 상태를 뜻한다.
		m_deferredGeometryPSO.m_pBlendState.Reset();
		m_isDeferredGeometryPipelineInitialized = true;
	}

	m_pGraphicsEngine->SetPipelineState(m_deferredGeometryPSO);
	m_pGraphicsEngine->mpRenderer->Render(modelBuffer);
}

void MeshRenderer::RenderThinFilmModel(ModelBuffer* modelBuffer)
{
	// 얇은 막 간섭 효과용 셰이더를 사용하는 렌더링 경로이다.
	m_pGraphicsEngine->SetPipelineState(Dears::Graphics::ThinFilmPSO);
	m_pGraphicsEngine->mpRenderer->Render(modelBuffer);
}

void MeshRenderer::RenderEquipmentModel(ModelBuffer* modelBuffer)
{
	// 캐릭터 장비처럼 특정 본이나 변환에 붙는 메시를 그리는 경로이다.
	m_pGraphicsEngine->SetPipelineState(Dears::Graphics::EquipmentGeometryPSO);
	m_pGraphicsEngine->mpRenderer->Render(modelBuffer);
}

void MeshRenderer::RenderOpacityModel(ModelBuffer* modelBuffer)
{
	// 반투명 메시 렌더링 경로이다.
	// 실제 투명도 값은 SetOpacityFactor()에서 OpacityPSO에 미리 설정된다.
	m_pGraphicsEngine->SetPipelineState(Dears::Graphics::OpacityPSO);
	m_pGraphicsEngine->mpRenderer->Render(modelBuffer);
}

void MeshRenderer::RenderEdgeModel(ModelBuffer* modelBuffer)
{
	// 1차로 기본 메시를 펀칭하듯 그린 뒤, 2차로 외곽선 전용 렌더를 수행한다.
	// 기존 동작을 그대로 옮긴 것이며, 나중에 post-process outline으로 바꿀 수도 있다.
	m_pGraphicsEngine->SetPipelineState(Dears::Graphics::PunchingPSO);
	m_pGraphicsEngine->mpRenderer->Render(modelBuffer);

	m_pGraphicsEngine->SetPipelineState(Dears::Graphics::EdgePSO);
	m_pGraphicsEngine->mpRenderer->RenderEdge(modelBuffer);
}

void MeshRenderer::RenderWater(ModelBuffer* modelBuffer)
{
	// 물 렌더링 전용 PSO를 사용하는 경로이다.
	m_pGraphicsEngine->SetPipelineState(Dears::Graphics::WaterPSO);
	m_pGraphicsEngine->mpRenderer->Render(modelBuffer);
}

void MeshRenderer::RenderDepthMap(ModelBuffer* modelBuffer)
{
	// 그림자 맵에 사용할 깊이만 렌더링한다.
	m_pGraphicsEngine->mpRenderer->RenderDepthMap(modelBuffer);
}

void MeshRenderer::RenderAnimatedDepthMap(ModelBuffer* modelBuffer)
{
	// 스키닝 메시의 그림자 깊이 렌더링 경로이다.
	m_pGraphicsEngine->mpRenderer->RenderAniDepthMap(modelBuffer);
}

void MeshRenderer::RenderEquipmentDepthMap(ModelBuffer* modelBuffer)
{
	// 장비 메시의 그림자 깊이 렌더링 경로이다.
	m_pGraphicsEngine->mpRenderer->RenderEquipDepthMap(modelBuffer);
}

void MeshRenderer::RenderInstancedModels(ModelBuffer* modelBuffer)
{
	// 같은 메시를 여러 위치에 반복해서 그릴 때 사용하는 인스턴싱 경로이다.
	m_pGraphicsEngine->SetPipelineState(Dears::Graphics::BasicInstancingPSO);
	m_pGraphicsEngine->mpRenderer->Rend_InstancedModels(modelBuffer);
}

void MeshRenderer::RenderBillboard(ModelBuffer* modelBuffer)
{
	// 카메라를 향하는 평면 메시 렌더링 경로이다.
	m_pGraphicsEngine->SetPipelineState(Dears::Graphics::TestPSO);
	m_pGraphicsEngine->mpRenderer->Render(modelBuffer);
}

void MeshRenderer::RenderCubeMap(ModelBuffer* modelBuffer)
{
	// 환경 배경이나 반사에 사용할 큐브맵 렌더링 경로이다.
	m_pGraphicsEngine->SetPipelineState(Dears::Graphics::CubeMapGeometryPSO);
	m_pGraphicsEngine->mpRenderer->Render_CubeMap(modelBuffer);
}

void MeshRenderer::SetOpacityFactor(float blendFactor[4])
{
	// 현재 OpacityPSO는 전역 상태처럼 쓰이고 있다.
	// RHI 도입 시에는 PSO를 직접 수정하기보다 렌더 상태 객체나 draw command에 담는 형태가 더 좋다.
	for (int i = 0; i < 4; i++)
	{
		Dears::Graphics::OpacityPSO.m_blendFactor[i] = blendFactor[i];
	}
}
