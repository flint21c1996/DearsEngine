#include "MeshRenderer.h"

#include "DearsGraphicsEngine.h"
#include "GraphicsCommon.h"
#include "Renderer.h"
#include "ShaderCatalog.h"

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
	EnsureDeferredGeometryPipeline();

	m_pGraphicsEngine->SetPipelineState(m_deferredGeometryPSO);
	m_pGraphicsEngine->mpRenderer->Render(modelBuffer);
}

void MeshRenderer::EnsureDeferredGeometryPipeline()
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
}

bool MeshRenderer::RenderWithShaders(
	ModelBuffer* modelBuffer,
	const std::string& vertexShader,
	const std::string& pixelShader)
{
	const std::string key = vertexShader + "|" + pixelShader;
	auto found = m_shaderPipelines.find(key);
	if (found == m_shaderPipelines.end())
	{
		PipelineStateObject pipeline;
		if (!BuildShaderPipeline(vertexShader, pixelShader, pipeline))
		{
			return false;
		}
		found = m_shaderPipelines.emplace(key, pipeline).first;
	}

	// Renderer::Render()는 오브젝트가 가진 상수 버퍼와 텍스처를 슬롯 규칙대로 전부 바인딩한다.
	// 그래서 셰이더 조합이 달라져도 그리기 코드는 같고, 셰이더가 읽는 슬롯에
	// 오브젝트의 데이터가 없으면(예: 본 버퍼 없는 메시에 Skinned) 그 값이 0으로 읽힐 뿐이다.
	m_pGraphicsEngine->SetPipelineState(found->second);
	m_pGraphicsEngine->mpRenderer->Render(modelBuffer);
	return true;
}

bool MeshRenderer::BuildShaderPipeline(
	const std::string& vertexShader,
	const std::string& pixelShader,
	PipelineStateObject& pipeline)
{
	// 입출력 구조체 계열이 다른 셰이더를 묶으면 DX11이 Draw를 버리거나 쓰레기 값을 넘긴다.
	if (!ShaderCatalog::AreCompatible(vertexShader, pixelShader))
	{
		return false;
	}

	// Vertex Shader를 원래 쓰던 PSO를 바탕으로 삼는다.
	// Input Layout, Rasterizer/Depth/Blend 상태가 그 Vertex Shader에 맞게 이미 설정되어 있기 때문이다.
	const PipelineStateObject* basePipeline = nullptr;
	if (vertexShader == ShaderCatalog::VertexShader::Static)
	{
		basePipeline = &Dears::Graphics::BasicGeometryPSO;
	}
	else if (vertexShader == ShaderCatalog::VertexShader::Skinned)
	{
		basePipeline = &Dears::Graphics::AnimeGeometryPSO;
	}
	else if (vertexShader == ShaderCatalog::VertexShader::Equipment)
	{
		basePipeline = &Dears::Graphics::EquipmentGeometryPSO;
	}
	else if (vertexShader == ShaderCatalog::VertexShader::Water)
	{
		basePipeline = &Dears::Graphics::WaterPSO;
	}
	else if (vertexShader == ShaderCatalog::VertexShader::Billboard)
	{
		basePipeline = &Dears::Graphics::TestPSO;
	}
	else if (vertexShader == ShaderCatalog::VertexShader::Pbr)
	{
		basePipeline = &Dears::Graphics::PBRPSO;
	}
	if (!basePipeline)
	{
		return false;
	}

	// Pixel Shader는 그 셰이더를 가진 PSO에서 빌려 온다.
	ComPtr<ID3D11PixelShader> selectedPixelShader;
	bool writesGBuffer = false;
	if (pixelShader == ShaderCatalog::PixelShader::Lit)
	{
		selectedPixelShader = Dears::Graphics::BasicGeometryPSO.m_pPixelShader;
	}
	else if (pixelShader == ShaderCatalog::PixelShader::Unlit)
	{
		selectedPixelShader = Dears::Graphics::TestPSO.m_pPixelShader;
	}
	else if (pixelShader == ShaderCatalog::PixelShader::PbrLit)
	{
		selectedPixelShader = Dears::Graphics::PBRPSO.m_pPixelShader;
	}
	else if (pixelShader == ShaderCatalog::PixelShader::ThinFilm)
	{
		selectedPixelShader = Dears::Graphics::ThinFilmPSO.m_pPixelShader;
	}
	else if (pixelShader == ShaderCatalog::PixelShader::GBuffer)
	{
		EnsureDeferredGeometryPipeline();
		selectedPixelShader = m_deferredGeometryPSO.m_pPixelShader;
		writesGBuffer = true;
	}
	if (!selectedPixelShader)
	{
		return false;
	}

	pipeline = *basePipeline;
	pipeline.m_pPixelShader = selectedPixelShader;
	if (writesGBuffer)
	{
		// G-Buffer에는 Normal/Material 같은 "값"을 그대로 기록해야 하므로 블렌딩을 끈다.
		pipeline.m_pBlendState.Reset();
	}
	return true;
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

void MeshRenderer::RenderPbrDepthMap(ModelBuffer* modelBuffer)
{
	if (!m_isPbrShadowPipelineInitialized)
	{
		// Depth/Rasterizer 상태는 기존 Shadow PSO를 그대로 재사용한다.
		// Vertex Shader만 PBR 전용으로 교체해 메인 PBR Pass와 같은 Height 변형을 수행한다.
		m_pbrShadowPSO = Dears::Graphics::depthOnlyPSO;
		const std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		RendererHelper::CreateVertexShaderAndInputLayout(
			m_pGraphicsEngine->m_pDevice,
			"../DearsGraphicsEngine/Shader/PBRShadowVertexShader.hlsl",
			inputElements,
			m_pbrShadowPSO.m_pVertexShader,
			m_pbrShadowPSO.m_pInputLayout);
		m_isPbrShadowPipelineInitialized = true;
	}

	m_pGraphicsEngine->SetPipelineState(m_pbrShadowPSO);
	m_pGraphicsEngine->mpRenderer->RenderPbrDepthMap(modelBuffer);
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
