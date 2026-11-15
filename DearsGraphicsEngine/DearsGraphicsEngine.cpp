#if DEBUG
#incldue<iostream>
#endif
#include "DearsGraphicsEngine.h"
#include "DebugRenderer.h"
#include "DeferredLightingRenderer.h"
#include "GraphicsAssetManager.h"
#include "GraphicsCommon.h"
#include "GBuffer.h"
#include "GBufferDebugPanel.h"
#include "GBufferDebugRenderer.h"
#include "MeshRenderer.h"
#include "ParticleRenderer.h"
#include "PostProcessRenderer.h"
#include "RenderTargetPool.h"
#include "ShadowMapDebugRenderer.h"
#include "UiRenderer.h"


DearsGraphicsEngine::DearsGraphicsEngine(HWND _hWnd, int screenWidth, int screenHeight)
    : m_hWnd(_hWnd)
    , m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
    , m_pTargetCamera(nullptr)
{
}

DearsGraphicsEngine::~DearsGraphicsEngine()
{

	// unique_ptr???먮룞?쇰줈 ?댁젣
	// m_pTargetCamera???뚯쑀?섏? ?딆쑝誘濡?delete?섏? ?딆쓬

}

void DearsGraphicsEngine::Initialize()
{
	
	RendererHelper::CreateDevice(m_pDevice, m_pDeviceContext);
	
	// 由ъ냼??留ㅻ땲?瑜?癒쇱? ?앹꽦 (Renderer 珥덇린?붿뿉 ?꾩슂)
	m_pResourceManager = std::make_unique<GraphicsResourceManager>(m_pDevice, m_pDeviceContext);

	// ?뚮뜑???앹꽦
	mpRenderer = std::make_unique<Renderer>(m_hWnd, 0, 0, m_screenWidth, m_screenHeight, m_pDevice, m_pDeviceContext);
	mpRenderer->Initialize(m_pResourceManager.get());

	// 최초에는 전체 렌더 크기로 만들고, 에디터 패널 너비가 결정되면
	// SetRenderViewportWidth()에서 실제 3D 뷰포트 크기로 다시 생성한다.
	m_pGBuffer = std::make_unique<GBuffer>();
	m_pGBuffer->Initialize(m_pDevice.Get(), m_pDeviceContext.Get(), m_screenWidth, m_screenHeight);

	// 게임 엔진이 패스 출력/입력으로 쓸 오프스크린 타깃을 요청하면 여기서 만들어 보관한다.
	m_pRenderTargetPool = std::make_unique<RenderTargetPool>();
	m_pRenderTargetPool->Initialize(
		m_pDevice.Get(),
		static_cast<UINT>(m_screenWidth),
		static_cast<UINT>(m_screenHeight));

	mpAnimationHelper = std::make_unique<AnimationHelper>();

	m_pUiRenderer = std::make_unique<UiRenderer>(
		m_hWnd,
		m_pDevice,
		m_pDeviceContext,
		m_screenWidth,
		m_screenHeight,
		m_pResourceManager.get());

	// 디퍼드 구현 중에는 각 G-Buffer를 최종 화면과 함께 즉시 확인할 수 있어야 한다.
	// 패널은 GBuffer의 소유권을 갖지 않고 엔진이 소유한 객체를 읽기만 한다.
	m_pGBufferDebugRenderer = std::make_unique<GBufferDebugRenderer>();
	m_pGBufferDebugRenderer->Initialize(m_pDevice.Get(), m_pDeviceContext.Get(), m_pGBuffer.get());
	m_pGBufferDebugPanel = std::make_unique<GBufferDebugPanel>(*m_pGBuffer, *m_pGBufferDebugRenderer);
	m_pUiRenderer->AddEditorPanel(m_pGBufferDebugPanel.get());

	// Geometry Pass가 채운 G-Buffer를 실제 Scene Color로 바꾸는 Fullscreen Lighting 렌더러다.
	// G-Buffer 리소스는 DearsGraphicsEngine이 계속 소유하고, 이 객체는 Lighting Draw만 담당한다.
	m_pDeferredLightingRenderer = std::make_unique<DeferredLightingRenderer>();
	m_pDeferredLightingRenderer->Initialize(m_pDevice.Get(), m_pDeviceContext.Get());

	// Shadow Map 미리보기는 실제 Depth 리소스를 복사하지 않고, 매 프레임 선형 깊이 색상만 만든다.
	// 패널 자체는 Scene이 선택 상태와 함께 등록하고 그래픽스 엔진은 변환된 SRV만 제공한다.
	m_pShadowMapDebugRenderer = std::make_unique<ShadowMapDebugRenderer>();
	m_pShadowMapDebugRenderer->Initialize(m_pDevice.Get(), m_pDeviceContext.Get());

	m_pAssetManager = std::make_unique<GraphicsAssetManager>(
		m_pResourceManager.get(),
		m_pUiRenderer.get());

	mpLightHelper = std::make_unique<LightHelper>();

	m_pDebugRenderer = std::make_unique<DebugRenderer>(this);
	m_pDebugRenderer->Initialize();

	m_pMeshRenderer = std::make_unique<MeshRenderer>(this);

	Dears::Graphics::InitCommonStates(m_pDevice);

	MeshData cubeMeshData = GeometryGenerator::MakeBox(1000);
	std::reverse(cubeMeshData.indices.begin(), cubeMeshData.indices.end());
	m_pResourceManager->AddModel(cubeMeshData, "CubeMap");

	MeshData SphereMeshData = GeometryGenerator::MakeSphere(1.f, 300, 300 ,{ 2.0f, 2.0f });
	m_pResourceManager->AddModel(SphereMeshData, "MySphere");

	MeshData SquareMeshData = GeometryGenerator::MakeSquare(1);
	m_pResourceManager->AddModel(SquareMeshData, "MySquare");

	MeshData BillBoardSquareMeshData = GeometryGenerator::BillboradSquare(1);
	m_pResourceManager->AddModel(BillBoardSquareMeshData, "BillBoardSquare");

	MeshData boxMeshData = GeometryGenerator::MakeBox(1);
	m_pResourceManager->AddModel(boxMeshData, "MyBox");

	m_pParticleRenderer = std::make_unique<ParticleRenderer>(
		m_pDevice,
		m_pDeviceContext,
		m_pResourceManager.get(),
		MAX_PARTICLE);
	m_pParticleRenderer->Initialize();

	m_pPostProcessRenderer = std::make_unique<PostProcessRenderer>(this);
	m_pPostProcessRenderer->Initialize();
}

void DearsGraphicsEngine::Update()
{
	m_pParticleRenderer->Update();
}

void DearsGraphicsEngine::BeginEditorFrame()
{
    UIBegineRender();
    // 도킹 영역은 그 안에 붙을 창들(Viewport, 패널들)보다 먼저 만들어야 한다.
    m_pUiRenderer->BeginDockSpace();
    m_viewportSurfaceReady = false;
    m_pUiRenderer->BeginViewport([this](int width, int height) -> void*
    {
        m_viewportSurfaceReady = EnsureViewportSurface(width, height);
        return m_viewportSurfaceReady ? mpRenderer->GetWorldShaderResourceView() : nullptr;
    });
    m_pUiRenderer->DrawRegisteredPanels();
    SetUICurrentWindow();
}

void DearsGraphicsEngine::BeginRender()
{
    mpRenderer->BeginRender();
}


void DearsGraphicsEngine::EndRender()
{

	RenderImGui();
	mpRenderer->BindPresentationTarget();
	EndRenderImGui();
	mpRenderer->EndRender();
}

void DearsGraphicsEngine::Finalize()
{

}

void DearsGraphicsEngine::RenderFrame()
{
	// 프레임의 시작과 끝(Clear, ImGui 프레임, Present)은 그래픽스 엔진이 책임지고,
	// 그 사이에 무엇을 어떤 순서로 그릴지는 게임 엔진이 구성한 파이프라인이 결정한다.
	BeginRender();
	if (m_viewportSurfaceReady) m_renderPipeline.Execute(*this);
	EndRender();
}

RenderTargetHandle DearsGraphicsEngine::CreateRenderTarget(const RenderTargetDesc& desc)
{
	// 디바이스가 만들어진 뒤(Initialize 이후)에만 타깃을 만들 수 있다.
	return m_pRenderTargetPool ? m_pRenderTargetPool->Create(desc) : RenderTargetHandle();
}

RenderTargetHandle DearsGraphicsEngine::FindRenderTarget(const std::string& name) const
{
	return m_pRenderTargetPool ? m_pRenderTargetPool->Find(name) : RenderTargetHandle();
}

void DearsGraphicsEngine::ApplyRenderPassIO(const RenderPassIO& requestedIO)
{
	if (!m_pGBuffer || !mpRenderer || !m_pDeviceContext)
	{
		return;
	}

	const RenderPassIO io = NormalizeRenderPassIO(requestedIO);

	// 1. 이전 패스가 읽던 오프스크린 타깃을 먼저 푼다.
	//    이번 패스가 그 타깃에 쓰려고 할 수 있는데, 입력으로 묶인 채 출력으로 묶으면
	//    DX11이 경고와 함께 입력 쪽을 강제로 NULL로 만든다. 의도를 코드로 분명히 해 둔다.
	ID3D11ShaderResourceView* nullResource = nullptr;
	for (UINT slot : m_boundRenderTargetInputSlots)
	{
		m_pDeviceContext->PSSetShaderResources(slot, 1, &nullResource);
	}
	m_boundRenderTargetInputSlots.clear();

	// 2. G-Buffer를 입력으로 돌리거나 입력에서 뗀다.
	if (io.readGBuffer)
	{
		if (!m_isGBufferBoundAsInput)
		{
			// 이 호출은 현재 출력 대상을 모두 해제한 뒤 G-Buffer를 t20~t23에 연결한다.
			// 그래서 실제 출력 대상은 반드시 아래 3번에서 다시 묶어야 한다.
			m_pGBuffer->BindForLightingPass(GBuffer::LightingShaderSlot);
			m_isGBufferBoundAsInput = true;

			// Material 채널 분리와 Depth 기반 Position 복원 결과를 디버그용 텍스처에 생성한다.
			if (m_pGBufferDebugRenderer)
			{
				m_pGBufferDebugRenderer->Render();
			}
		}
	}
	else if (m_isGBufferBoundAsInput)
	{
		// G-Buffer 깊이가 SRV로 남아 있으면 같은 리소스를 깊이 버퍼로 묶을 수 없다.
		m_pGBuffer->UnbindShaderResources(GBuffer::LightingShaderSlot);
		m_isGBufferBoundAsInput = false;
	}

	// 3. 출력 대상을 묶는다.
	RenderTargetHandle boundOffscreenTarget;
	switch (io.color)
	{
	case PassColorOutput::GBuffer:
		// MRT 3장과 G-Buffer 깊이를 묶고 모두 지운다. 내부에서 G-Buffer 입력도 해제한다.
		m_pGBuffer->BindForGeometryPass();
		m_isGBufferBoundAsInput = false;
		// 직전 패스가 크기가 다른 오프스크린 타깃에 그렸더라도 메인 표시 영역으로 되돌린다.
		m_pDeviceContext->RSSetViewports(1, &mpRenderer->GetMainViewport());
		break;

	case PassColorOutput::BackBuffer:
	{
		if (io.depth == PassDepthOutput::None)
		{
			ID3D11RenderTargetView* backBuffer = mpRenderer->GetBackBufferRenderTargetView();
			m_pDeviceContext->OMSetRenderTargets(1, &backBuffer, nullptr);
			m_pDeviceContext->RSSetViewports(1, &mpRenderer->GetMainViewport());
		}
		else
		{
			// nullptr을 넘기면 Renderer가 메인 깊이를 사용한다.
			mpRenderer->BindMainRenderTarget(
				io.depth == PassDepthOutput::GBufferDepth ? m_pGBuffer->GetDepthStencilView() : nullptr);
		}

		if (io.clearColor)
		{
			m_pDeviceContext->ClearRenderTargetView(
				mpRenderer->GetBackBufferRenderTargetView(), io.clearColorValue);
		}
		if (io.clearDepth && io.depth != PassDepthOutput::None)
		{
			ID3D11DepthStencilView* depth = io.depth == PassDepthOutput::GBufferDepth
				? m_pGBuffer->GetDepthStencilView()
				: mpRenderer->GetMainDepthStencilView();
			m_pDeviceContext->ClearDepthStencilView(
				depth, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		}
		break;
	}

	case PassColorOutput::Offscreen:
	{
		const RenderTargetPool::Entry* target =
			m_pRenderTargetPool ? m_pRenderTargetPool->Get(io.offscreenTarget) : nullptr;
		if (!target)
		{
			// 핸들이 잘못됐으면 직전 출력 대상을 그대로 둔다.
			break;
		}

		// 백 버퍼와 같은 크기의 타깃만 G-Buffer 깊이를 공유할 수 있다.
		// DX11은 색상 타깃과 깊이 버퍼의 크기가 다르면 바인딩 자체를 거부한다.
		const bool isScreenSized =
			target->width == m_pGBuffer->GetWidth() && target->height == m_pGBuffer->GetHeight();

		ID3D11DepthStencilView* depth = nullptr;
		if (io.depth == PassDepthOutput::Default)
		{
			depth = target->depthStencilView.Get();
		}
		else if (io.depth == PassDepthOutput::GBufferDepth && isScreenSized)
		{
			depth = m_pGBuffer->GetDepthStencilView();
		}

		ID3D11RenderTargetView* renderTarget = target->renderTargetView.Get();
		m_pDeviceContext->OMSetRenderTargets(1, &renderTarget, depth);

		// 화면 크기 타깃은 메인 표시 영역과 같은 뷰포트를 써야 G-Buffer/백 버퍼와 픽셀 위치가 일치한다.
		// 크기가 다른 타깃(미니맵 등)은 타깃 전체를 채운다.
		m_pDeviceContext->RSSetViewports(
			1, isScreenSized ? &mpRenderer->GetMainViewport() : &target->viewport);

		if (io.clearColor)
		{
			m_pDeviceContext->ClearRenderTargetView(renderTarget, io.clearColorValue);
		}
		if (io.clearDepth && depth)
		{
			m_pDeviceContext->ClearDepthStencilView(
				depth, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		}
		boundOffscreenTarget = io.offscreenTarget;
		break;
	}

	case PassColorOutput::Keep:
	default:
		break;
	}

	// 4. 이 패스가 읽을 오프스크린 타깃을 픽셀 셰이더 슬롯에 연결한다.
	for (const RenderTargetInput& input : io.inputs)
	{
		// 지금 쓰고 있는 타깃을 동시에 읽을 수는 없다.
		if (boundOffscreenTarget.IsValid() && input.target == boundOffscreenTarget)
		{
			continue;
		}

		const RenderTargetPool::Entry* source =
			m_pRenderTargetPool ? m_pRenderTargetPool->Get(input.target) : nullptr;
		if (!source)
		{
			continue;
		}

		ID3D11ShaderResourceView* resource = source->shaderResourceView.Get();
		m_pDeviceContext->PSSetShaderResources(input.shaderSlot, 1, &resource);
		m_boundRenderTargetInputSlots.push_back(input.shaderSlot);
	}
}

void DearsGraphicsEngine::DrawMesh(ModelBuffer* modelBuffer, MeshDrawMode mode)
{
	if (!modelBuffer || !m_pMeshRenderer)
	{
		return;
	}

	// "그리기 종류 -> PSO/셰이더 경로" 매핑을 그래픽스 엔진 안 한 곳에 모은다.
	// 새 재질을 추가할 때는 MeshDrawMode 값과 이 분기만 늘리면 되고,
	// 인터페이스와 게임 엔진 쪽 호출 형태는 그대로 유지된다.
	switch (mode)
	{
	case MeshDrawMode::Skinned:
		m_pMeshRenderer->RenderAnimatedModel(modelBuffer);
		break;
	case MeshDrawMode::Equipment:
		m_pMeshRenderer->RenderEquipmentModel(modelBuffer);
		break;
	case MeshDrawMode::Pbr:
		m_pMeshRenderer->RenderPbrModel(modelBuffer);
		break;
	case MeshDrawMode::ThinFilm:
		m_pMeshRenderer->RenderThinFilmModel(modelBuffer);
		break;
	case MeshDrawMode::DeferredGeometry:
		m_pMeshRenderer->RenderDeferredGeometry(modelBuffer);
		break;
	case MeshDrawMode::CubeMap:
		m_pMeshRenderer->RenderCubeMap(modelBuffer);
		break;
	case MeshDrawMode::Billboard:
		m_pMeshRenderer->RenderBillboard(modelBuffer);
		break;
	case MeshDrawMode::Opacity:
		m_pMeshRenderer->RenderOpacityModel(modelBuffer);
		break;
	case MeshDrawMode::Water:
		m_pMeshRenderer->RenderWater(modelBuffer);
		break;
	case MeshDrawMode::Instanced:
		m_pMeshRenderer->RenderInstancedModels(modelBuffer);
		break;
	case MeshDrawMode::Outline:
		m_pMeshRenderer->RenderEdgeModel(modelBuffer);
		break;
	case MeshDrawMode::Static:
	default:
		m_pMeshRenderer->RenderStaticModel(modelBuffer);
		break;
	}
}

bool DearsGraphicsEngine::DrawMeshWithShaders(
	ModelBuffer* modelBuffer,
	const std::string& vertexShader,
	const std::string& pixelShader)
{
	if (!modelBuffer || !m_pMeshRenderer)
	{
		return false;
	}

	return m_pMeshRenderer->RenderWithShaders(modelBuffer, vertexShader, pixelShader);
}

void DearsGraphicsEngine::DrawShadowCaster(ModelBuffer* modelBuffer, ShadowCasterMode mode)
{
	if (!modelBuffer || !m_pMeshRenderer)
	{
		return;
	}

	switch (mode)
	{
	case ShadowCasterMode::Skinned:
		m_pMeshRenderer->RenderAnimatedDepthMap(modelBuffer);
		break;
	case ShadowCasterMode::Equipment:
		m_pMeshRenderer->RenderEquipmentDepthMap(modelBuffer);
		break;
	case ShadowCasterMode::Pbr:
		m_pMeshRenderer->RenderPbrDepthMap(modelBuffer);
		break;
	case ShadowCasterMode::Static:
	default:
		m_pMeshRenderer->RenderDepthMap(modelBuffer);
		break;
	}
}

void DearsGraphicsEngine::DrawDeferredLighting()
{
	RenderDeferredLighting();
}

void DearsGraphicsEngine::DrawParticles()
{
	RendParticle();
}

void DearsGraphicsEngine::DrawPostProcess()
{
	RendPostProcessing();
}

void DearsGraphicsEngine::DrawLightGizmo(const Light& light, bool drawShadowFrustum, float shadowAspect)
{
	Rend_DebugLightGizmo(light, drawShadowFrustum, shadowAspect);
}

void DearsGraphicsEngine::RendParticle()
{
	m_pParticleRenderer->Render();
}

void DearsGraphicsEngine::AddParticle(unsigned int particleNum, CSParticleData& particleData)
{
	// 씬은 "파티클을 몇 개 생성할지"와 "어떤 데이터로 생성할지"만 요청한다.
	// 실제 파티클 풀, StructuredBuffer, UAV/SRV 같은 DX11 세부 구현은 ParticleManager 안에 숨긴다.
	m_pParticleRenderer->AddParticle(particleNum, particleData);
}

void DearsGraphicsEngine::RendPostProcessing()
{
	m_pPostProcessRenderer->Render();
}

void DearsGraphicsEngine::RenderDeferredLighting()
{
	if (m_pDeferredLightingRenderer)
	{
		m_pDeferredLightingRenderer->Render();
	}
}

int DearsGraphicsEngine::GetScreenWidth() const
{
	return m_screenWidth;
}

int DearsGraphicsEngine::GetScreenHeight() const
{
	return m_screenHeight;
}

float DearsGraphicsEngine::GetAspectRatio()
{
	return 0;
}


void DearsGraphicsEngine::AddModel(std::string _basePath, std::string _fileName)
{
	m_pAssetManager->LoadModel(_basePath, _fileName);
}

void DearsGraphicsEngine::AddAnimation(std::string _basePath, std::string _fileName)
{
	m_pAssetManager->LoadAnimation(_basePath, _fileName);

}

void DearsGraphicsEngine::Add3DTexture(std::string _basePath, std::string _fileName)
{
	m_pAssetManager->Load3DTexture(_basePath, _fileName);

}

void DearsGraphicsEngine::Add2DTexture(std::string _basePath, std::string _fileName)
{
	m_pAssetManager->Load2DTexture(_basePath, _fileName);

}
void DearsGraphicsEngine::AddDDSTexture(std::string _basePath, std::string _fileName, bool isCubeMap)
{
	m_pAssetManager->LoadDDSTexture(_basePath, _fileName, isCubeMap);

}

void DearsGraphicsEngine::Add2DMipMapTexture(std::string _basePath, std::string _fileName)
{
	m_pAssetManager->Load2DMipMapTexture(_basePath, _fileName);
}

ComPtr<ID3D11Buffer> DearsGraphicsEngine::Get_VertexBuffer(std::string _modelName)
{
	return m_pResourceManager->Get_VertexBuffer(_modelName);
}

ComPtr<ID3D11Buffer> DearsGraphicsEngine::Get_IndexBuffer(std::string _modelName)
{
	return m_pResourceManager->Get_IndexBuffer(_modelName);
}

unsigned int DearsGraphicsEngine::Get_NumIndex(std::string _modelName)
{
	return m_pResourceManager->Get_NumIndex(_modelName);
}

Model* DearsGraphicsEngine::Get_ModelInfo(std::string _modelName)
{
	return m_pResourceManager->Get_ModelInfo(_modelName);
}

Animation* DearsGraphicsEngine::Get_Animation(std::string _animeName)
{
	return m_pResourceManager->Get_Animation(_animeName);
}

ComPtr<ID3D11ShaderResourceView> DearsGraphicsEngine::Get_Textures(std::string _textureName)
{
	return m_pResourceManager->Get_Textures(_textureName);
}

ImFont* DearsGraphicsEngine::Get_Font(std::string _fontName)
{
	return m_pResourceManager->Get_Font(_fontName);
}

int DearsGraphicsEngine::Get_TargetModelBoneIndex(std::string _modelName, std::string _boneName)
{
	return m_pResourceManager->Get_TargetModelBoneIndex(_modelName, _boneName);
}

Matrix DearsGraphicsEngine::GetTargetBoneMatrix(std::string _targetModel, std::string _targetBoneName)
{
	return m_pResourceManager->Get_TargetBoneMatrix(_targetModel, _targetBoneName);
}

Matrix DearsGraphicsEngine::GetTargetBoneAboveMatrix(std::string _targetModel, std::string _targetBoneName, float _scale /*= 1.f*/)
{
	return m_pResourceManager->Get_TargetBoneAboveMatrix(_targetModel, _targetBoneName)* Matrix::CreateScale(_scale);
}

Matrix DearsGraphicsEngine::GetTargetBoneAboveMatrix(std::string _targetModel, int _index, float _scale /*= 1.f*/)
{
	return m_pResourceManager->Get_TargetBoneAboveMatrix(_targetModel, _index) * Matrix::CreateScale(_scale);
}

AABB DearsGraphicsEngine::Get_AABB(std::string __targetModel)
{
	return m_pResourceManager->Get_ModelInfo(__targetModel)->mMeshData->mAABB;
}

void DearsGraphicsEngine::Erase_VertexBuffer(std::string _modelName)
{
	m_pResourceManager->Erase_VertexBuffer(_modelName);
}

void DearsGraphicsEngine::Erase_IndexBuffer(std::string _modelName)
{
	m_pResourceManager->Erase_IndexBuffer(_modelName);
}

void DearsGraphicsEngine::Erase_NumIndex(std::string _modelName)
{
	m_pResourceManager->Erase_NumIndex(_modelName);
}

void DearsGraphicsEngine::Erase_ModelInfo(std::string _modelName)
{
	m_pResourceManager->Erase_ModelInfo(_modelName);
}

void DearsGraphicsEngine::Erase_Animation(std::string _animName)
{
	m_pResourceManager->Erase_Animation(_animName);
}

void DearsGraphicsEngine::Erase_Textures(std::string _textureName)
{
	m_pResourceManager->Erase_Textures(_textureName);
}

void DearsGraphicsEngine::Erase_Font(std::string _fontName)
{
	m_pResourceManager->Erase_Font(_fontName);
}

ComPtr<ID3D11Buffer> DearsGraphicsEngine::CreateConstantBuffer(VSConstantBufferData& _VsConstantBufferData)
{
	return RendererHelper::CreateConstantBuffer(m_pDevice, _VsConstantBufferData);
}

ComPtr<ID3D11Buffer> DearsGraphicsEngine::CreateConstantBuffer(VSBoneConstantBufferData& _VsBoneConstantBufferData)
{
	return RendererHelper::CreateConstantBuffer(m_pDevice, _VsBoneConstantBufferData);
}

ComPtr<ID3D11Buffer> DearsGraphicsEngine::CreateConstantBuffer(VSTargetBoneConstantBufferData& _VsTargetBoneConstantBufferData)
{
	return RendererHelper::CreateConstantBuffer(m_pDevice, _VsTargetBoneConstantBufferData);
}

ComPtr<ID3D11Buffer> DearsGraphicsEngine::CreateConstantBuffer(PSConstantBufferData& _PsConstantBufferData)
{
	return RendererHelper::CreateConstantBuffer(m_pDevice, _PsConstantBufferData);
}


ComPtr<ID3D11Buffer> DearsGraphicsEngine::CreateConstantBuffer(VSShadowConstantBufferData& _VsShadowConstantBufferData)
{
	return RendererHelper::CreateConstantBuffer(m_pDevice, _VsShadowConstantBufferData);
}

ComPtr<ID3D11Buffer> DearsGraphicsEngine::CreateConstantBuffer(PsShadowConstantBufferData& _PsShadowConstantBufferData)
{
	return RendererHelper::CreateConstantBuffer(m_pDevice, _PsShadowConstantBufferData);
}

ComPtr<ID3D11Buffer> DearsGraphicsEngine::CreateConstantBuffer(CommonConstantBufferData& _CommonConstantBufferData)
{
	return RendererHelper::CreateConstantBuffer(m_pDevice, _CommonConstantBufferData);

}

ComPtr<ID3D11Buffer> DearsGraphicsEngine::CreateConstantBuffer(VSInstantConstantBufferData& _VSInstantConstantBufferData)
{
	return RendererHelper::CreateConstantBuffer(m_pDevice, _VSInstantConstantBufferData);
}

ComPtr<ID3D11Buffer> DearsGraphicsEngine::CreateConstantBuffer(PSEdgeConstantBufferData& _pPSEdgeConstantBufferData)
{
	return RendererHelper::CreateConstantBuffer(m_pDevice, _pPSEdgeConstantBufferData);
}

ComPtr<ID3D11Buffer> DearsGraphicsEngine::CreateStructuredBuffer(CSParticleData& _TestCSParticleData, unsigned int _count)
{
	return RendererHelper::CreateStructuredBuffer(m_pDevice, _TestCSParticleData, _count);
}

void DearsGraphicsEngine::UpdateConstantBuffer(ModelBuffer* _pModelBuffer, VSConstantBufferData& _VsConstantBufferData)
{
	RendererHelper::UpdateBuffer(m_pDeviceContext, _VsConstantBufferData, _pModelBuffer->m_pVSConstantBuffer);
}

void DearsGraphicsEngine::UpdateConstantBuffer(ModelBuffer* _pModelBuffer, PSConstantBufferData& _PsConstantBufferData)
{
	RendererHelper::UpdateBuffer(m_pDeviceContext, _PsConstantBufferData, _pModelBuffer->m_pPSConstantBuffer);
}

void DearsGraphicsEngine::UpdateVSEdgeConstantBuffer(ModelBuffer* _pModelBuffer, VSEdgeConstantBufferData& _pPSEdgeConstantBuffer)
{
	RendererHelper::UpdateBuffer(m_pDeviceContext, _pPSEdgeConstantBuffer, _pModelBuffer->m_pVSEdgeConstantBuffer);

}

void DearsGraphicsEngine::UpdatePSEdgeConstantBuffer(ModelBuffer* _pModelBuffer, PSEdgeConstantBufferData& _pPSEdgeConstantBuffer)
{
	RendererHelper::UpdateBuffer(m_pDeviceContext, _pPSEdgeConstantBuffer, _pModelBuffer->m_pPSEdgeConstantBuffer);
}

void DearsGraphicsEngine::UpdateVSWaterConstantBuffer(ModelBuffer* _pModelBuffer, VSWaterConstantBufferData& _pVSWaterConstantBuffer)
{
	RendererHelper::UpdateBuffer(m_pDeviceContext, _pVSWaterConstantBuffer, _pModelBuffer->m_pVSWaterConstantBuffer);

}

void DearsGraphicsEngine::UpdateVSPBRConstantBuffer(ModelBuffer* _pModelBuffer, PBRVertexShaderConstantData& _PBRConstantData)
{
	RendererHelper::UpdateBuffer(m_pDeviceContext, _PBRConstantData, _pModelBuffer->m_VSPBRConstantBuffer);

}

void DearsGraphicsEngine::UpdatePSPBRConstantBuffer(ModelBuffer* _pModelBuffer, PBRPixelShaderConstantData& _PBRConstantData)
{
	RendererHelper::UpdateBuffer(m_pDeviceContext, _PBRConstantData, _pModelBuffer->m_PSPBRConstantBuffer);

}

void DearsGraphicsEngine::UpdatePSThinFilmonstantBuffer(ModelBuffer* _pModelBuffer, ThinFilmPixelShaderConstantData& _ThinFilmConstantData)
{
	RendererHelper::UpdateBuffer(m_pDeviceContext, _ThinFilmConstantData, _pModelBuffer->m_PSThinFilmConstantBuffer);
}

void DearsGraphicsEngine::UpdateCommonConstantBuffer(CommonConstantBufferData& _CommonBufferData)
{
 	_CommonBufferData.view = m_pTargetCamera->GetViewRow().Transpose();		// 占쏙옙占쏙옙 占쏙옙환
 	_CommonBufferData.proj = m_pTargetCamera->GetProjRow().Transpose();
 	_CommonBufferData.viewProj = (m_pTargetCamera->GetViewRow() * m_pTargetCamera->GetProjRow()).Transpose();

	_CommonBufferData.invView = _CommonBufferData.view.Invert();
 	_CommonBufferData.invProj = _CommonBufferData.proj.Invert();
	
	// Depth에서 World Position을 복원할 때 사용할 역 View-Projection 행렬이다.
	// 과거에는 깨진 주석 뒤에 코드가 붙어 있어 실제 대입문 전체가 주석 처리되어 있었다.
	_CommonBufferData.invViewProj = _CommonBufferData.viewProj.Invert();

	_CommonBufferData.eyeWorld = m_pTargetCamera->GetmViewPos();

	mpRenderer->UpdateCommonConstantBuffer(_CommonBufferData);
	
}

void DearsGraphicsEngine::ApplyRenderContext(const RenderContext& renderContext)
{
	// 지금은 RenderContext가 SetCamera()와 UpdateCommonConstantBuffer()를 묶는 얇은 계층이다.
	// 하지만 이 함수 경계가 생기면 이후에는 viewport, render target, resource transition 같은
	// 패스별 상태도 같은 입구에서 적용할 수 있다.
	//
	// renderContext.passType은 현재 패스가 어떤 종류인지 코드가 알 수 있게 해준다.
	// 예를 들어 나중에 ShadowPass에서는 depth target만 바인딩하고,
	// ScenePass에서는 color/depth target과 shadow map shader resource를 바인딩하는 식으로
	// 이 함수 안에서 RHI별 준비 작업을 분기할 수 있다.
	m_currentRenderPassType = renderContext.passType;
	const char* passName = GetCurrentRenderPassName();
	(void)passName;

	if (renderContext.camera)
	{
		SetCamera(renderContext.camera);
	}

	if (renderContext.commonBuffer)
	{
		UpdateCommonConstantBuffer(*renderContext.commonBuffer);
	}

	// 패스의 출력/입력 리소스를 한 곳에서 바꾼다.
	// GameEngine은 "어디에 쓰고 무엇을 읽을지"를 RenderPassIO로 말하기만 하고,
	// DX11의 RTV/DSV/SRV 바인딩 순서와 충돌 규칙은 ApplyRenderPassIO()가 처리한다.
	//
	// 패스가 io를 직접 지정했으면 그것을 쓰고, 아니면 패스 종류의 기본 구성을 쓴다.
	//   Geometry : G-Buffer MRT + G-Buffer 깊이에 쓰기
	//   Lighting : G-Buffer를 t20~t23으로 읽고 백 버퍼 + 메인 깊이에 쓰기
	//   Forward  : G-Buffer 입력을 풀고 백 버퍼 + Geometry 깊이(공유)에 쓰기
	// 기본 구성이 없는 종류(Ui, Debug, Custom 등)는 직전 패스의 바인딩을 그대로 이어받는다.
	if (renderContext.io)
	{
		ApplyRenderPassIO(*renderContext.io);
		return;
	}

	RenderPassIO defaultIO;
	if (GetDefaultRenderPassIO(renderContext.passType, defaultIO))
	{
		ApplyRenderPassIO(defaultIO);
	}
}

void DearsGraphicsEngine::UpdateShadowConstantBuffer(ModelBuffer* _pModelBuffer, VSShadowConstantBufferData& _VsShadowConstantBufferData)
{
//	m_pResourceManager->UpdateBuffer(_VsShadowConstantBufferData, _pModelBuffer->m_pShadowConstantBuffer);
}

void DearsGraphicsEngine::UpdateShadowConstantBuffer(ModelBuffer* _pModelBuffer, PsShadowConstantBufferData& _PsShadowConstantBufferData)
{
//	m_pResourceManager->UpdateBuffer(_PsShadowConstantBufferData, _pModelBuffer->m_pShadowPSConstantBuffer);
}

void DearsGraphicsEngine::Set_CubeMap(std::string environmentTexture, std::string diffuseTextureName, std::string specularTextureName, std::string BRDFTextureName)
{
	mpRenderer->SetCommonShaderResource
	(
		m_pResourceManager->Get_Textures(environmentTexture),
		m_pResourceManager->Get_Textures(diffuseTextureName),
		m_pResourceManager->Get_Textures(specularTextureName),
		m_pResourceManager->Get_Textures(BRDFTextureName)
	);
}

///
void DearsGraphicsEngine::UpdateBoneConstantBuffer(ModelBuffer* _pModelBuffer, VSBoneConstantBufferData& _VsBoneConstantBufferData)
{
	mpAnimationHelper->UpdateBoneConstant(
		_pModelBuffer->mpTargetModel,
		_pModelBuffer->mpTargetAnimation,
		_VsBoneConstantBufferData,
		_pModelBuffer->mAnimationPlaytime
		);
	RendererHelper::UpdateBuffer(m_pDeviceContext, _VsBoneConstantBufferData, _pModelBuffer->m_BoneConstantBuffer);
}
///
bool DearsGraphicsEngine::UpdateTransitionBoneConstantBuffer(ModelBuffer* _pModelBuffer, VSBoneConstantBufferData& _VsBoneConstantBufferData)
{
	if (mpAnimationHelper->UpdateBoneConstant(
		_pModelBuffer->mpTargetModel,
		_pModelBuffer->mpTargetAnimation,
		_pModelBuffer->mpNextTargetAnimation,
		_VsBoneConstantBufferData,
		_pModelBuffer->mAnimationPlaytime,
		_pModelBuffer->mNextAnimationPlaytime)
		)
	{
		
		RendererHelper::UpdateBuffer(m_pDeviceContext, _VsBoneConstantBufferData, _pModelBuffer->m_BoneConstantBuffer);
		return true;
	}
	else
	{
	return false;
	}
}

void DearsGraphicsEngine::UpdateTargetBoneConstantBuffer(ModelBuffer* _pModelBuffer, VSTargetBoneConstantBufferData& _VsTargetBoneConstantBufferData)
{
	RendererHelper::UpdateBuffer(m_pDeviceContext, _VsTargetBoneConstantBufferData, _pModelBuffer->m_TargetBoneConstantBuffer);
}

void DearsGraphicsEngine::SetPipelineState(PipelineStateObject& _pso)
{
	mpRenderer->SetPipelineState(_pso);
}

void DearsGraphicsEngine::Rend_AnimateModel(ModelBuffer* _modelBuffer)
{
	m_pMeshRenderer->RenderAnimatedModel(_modelBuffer);
}

void DearsGraphicsEngine::Rend_Model(ModelBuffer* _modelBuffer)
{
	m_pMeshRenderer->RenderStaticModel(_modelBuffer);
}

void DearsGraphicsEngine::Rend_PBR(ModelBuffer* _modelBuffer)
{
	m_pMeshRenderer->RenderPbrModel(_modelBuffer);
}

void DearsGraphicsEngine::Rend_DeferredGeometry(ModelBuffer* _modelBuffer)
{
	m_pMeshRenderer->RenderDeferredGeometry(_modelBuffer);
}


void DearsGraphicsEngine::Rend_ThinFilm(ModelBuffer* _modelBuffer)
{
	m_pMeshRenderer->RenderThinFilmModel(_modelBuffer);
}

void DearsGraphicsEngine::Rend_EquipmentModel(ModelBuffer* _modelBuffer)
{
	m_pMeshRenderer->RenderEquipmentModel(_modelBuffer);
}

void DearsGraphicsEngine::SetOpacityFactor(float blendFactor[4])
{
	m_pMeshRenderer->SetOpacityFactor(blendFactor);
}

void DearsGraphicsEngine::Rend_OpacitiyModel(ModelBuffer* _modelBuffer)
{
	m_pMeshRenderer->RenderOpacityModel(_modelBuffer);
}

void DearsGraphicsEngine::Rend_EdgeModel(ModelBuffer* _modelBuffer)
{
	m_pMeshRenderer->RenderEdgeModel(_modelBuffer);
}

void DearsGraphicsEngine::Rend_Water(ModelBuffer* _modelBuffer)
{
	m_pMeshRenderer->RenderWater(_modelBuffer);
}

void DearsGraphicsEngine::RenderDepthMap(ModelBuffer* _modelbuffer)
{
	m_pMeshRenderer->RenderDepthMap(_modelbuffer);
}

void DearsGraphicsEngine::RenderPbrDepthMap(ModelBuffer* _modelbuffer)
{
	// PBR 메시는 메인 패스에서 Height Map으로 정점 위치가 바뀌므로
	// 일반 Depth 경로가 아니라 동일한 변형을 수행하는 전용 Shadow 경로로 전달한다.
	m_pMeshRenderer->RenderPbrDepthMap(_modelbuffer);
}

void DearsGraphicsEngine::RenderAniDepthMap(ModelBuffer* _modelbuffer)
{
	m_pMeshRenderer->RenderAnimatedDepthMap(_modelbuffer);
}

void DearsGraphicsEngine::RenderEquipDepthMap(ModelBuffer* _modelbuffer)
{
	m_pMeshRenderer->RenderEquipmentDepthMap(_modelbuffer);
}

void DearsGraphicsEngine::BeginShadowFrame()
{
	mpRenderer->BeginShadowFrame();
}

void DearsGraphicsEngine::BeginShadowProduction()
{
	mpRenderer->BeginShadowProduction();
}

void DearsGraphicsEngine::EndShadowProduction(bool success)
{
	mpRenderer->EndShadowProduction(success);
}

void DearsGraphicsEngine::BeginTwoDimensionalShadowPass(UINT lightIndex)
{
	// DX11 리소스 형식과 바인딩 슬롯은 Renderer 내부 구현이다.
	// 게임 계층에서는 어떤 라이트의 2D 그림자 패스를 시작할지만 전달한다.
	mpRenderer->BeginTwoDimensionalShadowPass(lightIndex);
}

void DearsGraphicsEngine::BeginPointShadowFace(UINT lightIndex, UINT faceIndex)
{
	mpRenderer->BeginPointShadowFace(lightIndex, faceIndex);
}

void DearsGraphicsEngine::EndShadowPass()
{
	mpRenderer->EndShadowPass();
}

void DearsGraphicsEngine::RenderShadowMapDebugPreview(
	UINT lightIndex,
	float nearPlane,
	float farPlane,
	bool isPerspective)
{
	if (!m_pShadowMapDebugRenderer || !mpRenderer)
	{
		return;
	}

	m_pShadowMapDebugRenderer->Render(
		mpRenderer->GetShadowMapShaderResourceView(),
		lightIndex,
		nearPlane,
		farPlane,
		isPerspective);
}

void DearsGraphicsEngine::RenderPointShadowMapDebugPreview(
	UINT lightIndex,
	float nearPlane,
	float farPlane)
{
	if (!m_pShadowMapDebugRenderer || !mpRenderer)
	{
		return;
	}

	m_pShadowMapDebugRenderer->RenderPointCube(
		mpRenderer->GetPointShadowCubeShaderResourceView(),
		lightIndex,
		nearPlane,
		farPlane);
}

bool DearsGraphicsEngine::UIDrawShadowMapDebugPreview(Vector2 size) const
{
	ID3D11ShaderResourceView* preview = m_pShadowMapDebugRenderer
		? m_pShadowMapDebugRenderer->GetShaderResourceView()
		: nullptr;
	if (!preview)
	{
		return false;
	}

	// ImTextureID로의 변환은 DX11 ImGui 백엔드를 소유한 그래픽스 계층 안에서만 수행한다.
	// 나중에 RHI 백엔드가 바뀌면 이 함수 내부의 Texture ID 생성 방식만 교체하면 된다.
	ImGui::Image(
		reinterpret_cast<ImTextureID>(preview),
		ImVec2(size.x, size.y));
	return true;
}

bool DearsGraphicsEngine::UIDrawRenderTarget(RenderTargetHandle target, Vector2 size) const
{
	const RenderTargetPool::Entry* entry = m_pRenderTargetPool ? m_pRenderTargetPool->Get(target) : nullptr;
	if (!entry || !entry->shaderResourceView)
	{
		return false;
	}

	// Shadow Map 미리보기와 같은 이유로, ImTextureID 변환은 그래픽스 계층 안에서만 수행한다.
	ImGui::Image(
		reinterpret_cast<ImTextureID>(entry->shaderResourceView.Get()),
		ImVec2(size.x, size.y));
	return true;
}

void DearsGraphicsEngine::Rend_InstancedModels(ModelBuffer* _modelbuffers)
{
	m_pMeshRenderer->RenderInstancedModels(_modelbuffers);
}

void DearsGraphicsEngine::Rend_BillBoard(ModelBuffer* _modelbuffers)
{
	m_pMeshRenderer->RenderBillboard(_modelbuffers);
}

void DearsGraphicsEngine::Rend_DebugBox(Vector3 _size, Vector3 _rotation, Vector3 _transpose)
{
	m_pDebugRenderer->RenderBox(_size, _rotation, _transpose);
}

void DearsGraphicsEngine::Rend_DebugBox(Matrix _size, Matrix _rotation, Matrix _transpose)
{
	m_pDebugRenderer->RenderBox(_size, _rotation, _transpose);
}

void DearsGraphicsEngine::Rend_DebugBox(Matrix _size, Matrix _rotation, Matrix _transpose, Matrix _tempMatrix = Matrix())
{
	m_pDebugRenderer->RenderBox(_size, _rotation, _transpose, _tempMatrix);
}

void DearsGraphicsEngine::Rend_DebugBox(AABB& _AABB, Matrix Scale, Matrix _rotation, Matrix _tempMatrix )
{
	m_pDebugRenderer->RenderBox(_AABB, Scale, _rotation, _tempMatrix);
}

void DearsGraphicsEngine::Rend_DebugSphere(Vector3 _size, Vector3 _rotation, Vector3 _transpose)
{
	m_pDebugRenderer->RenderSphere(_size, _rotation, _transpose);
}

void DearsGraphicsEngine::Rend_DebugCapsule(Vector3 _size, Vector3 _rotation, Vector3 _transpose)
{
	m_pDebugRenderer->RenderCapsule(_size, _rotation, _transpose);
}

void DearsGraphicsEngine::Rend_DebugLightGizmo(
	const Light& light,
	bool drawShadowFrustum,
	float shadowAspect)
{
	m_pDebugRenderer->RenderLightGizmo(
		light,
		drawShadowFrustum,
		shadowAspect);
}

void DearsGraphicsEngine::Rend_CubeMap(ModelBuffer* _modelBuffer)
{
	m_pMeshRenderer->RenderCubeMap(_modelBuffer);
}

void DearsGraphicsEngine::SetCamera(Camera* _pTargetCamera)
{
	m_pTargetCamera=_pTargetCamera;
}

void DearsGraphicsEngine::AddFont(std::string _basePath, std::string _fileName, float _size, bool _isKorean)
{
	m_pAssetManager->LoadFont(_basePath, _fileName, _size, _isKorean);
}

void DearsGraphicsEngine::FontSetFinish()
{
	m_pUiRenderer->BuildFonts();
}

void DearsGraphicsEngine::UIBegineRender()
{
	m_pUiRenderer->BeginFrame();
}

void DearsGraphicsEngine::UICanvasSet(Vector2 _posXY, Vector2 _sizeWH)
{
	m_pUiRenderer->BeginCanvas(_posXY, _sizeWH);
}

void DearsGraphicsEngine::UIDrawImageStart()
{
	m_pUiRenderer->DrawImageStart();
}

void DearsGraphicsEngine::UIDrawImage(Vector2 _posXY, Vector2 _sizeWH, std::string _textureName, Vector4 _rgba /*= Vector4(1.0f, 1.0f, 1.0f, 1.0f)*/)
{
	m_pUiRenderer->DrawImage(_posXY, _sizeWH, _textureName, _rgba);
}

void DearsGraphicsEngine::UIDrawImageFin()
{
	m_pUiRenderer->DrawImageEnd();
}

void DearsGraphicsEngine::UIStartFontID(std::string _fontName)
{
	m_pUiRenderer->StartFont(_fontName);
}

void DearsGraphicsEngine::UIDrawText(Vector2 _pos, std::u8string _text, Vector4 _rgba)
{
	m_pUiRenderer->DrawText(_pos, _text, _rgba);
}

void DearsGraphicsEngine::UIFinFontID()
{
	m_pUiRenderer->EndFont();
}

void DearsGraphicsEngine::SetUICurrentWindow()
{
	m_pUiRenderer->CacheCurrentWindow();
}

void DearsGraphicsEngine::UIDrawRect(Vector2 _posXY, Vector2 _sizeWH, Vector4 _rgba, float _rounding /*= 0.0f*/, float _thickness)
{
	m_pUiRenderer->DrawRect(_posXY, _sizeWH, _rgba, _rounding, _thickness);
}

void DearsGraphicsEngine::UIDrawRectFilled(Vector2 _posXY, Vector2 _sizeWH, Vector4 _rgba, float _rounding /*= 0.0f*/)
{
	m_pUiRenderer->DrawRectFilled(_posXY, _sizeWH, _rgba, _rounding);
}

void DearsGraphicsEngine::UIDrawRectwithBorder(Vector2 _posXY, Vector2 _sizeWH, Vector4 _rgba, float _rounding /*= 0.0f*/, float _thickness)
{
	m_pUiRenderer->DrawRectWithBorder(_posXY, _sizeWH, _rgba, _rounding, _thickness);
}

void DearsGraphicsEngine::UIFreeRect(Vector2 _posXY1, Vector2 _posXY2, Vector2 _posXY3, Vector2 _posXY4, Vector4 _rgba, float _thickness)
{
	m_pUiRenderer->DrawFreeRect(_posXY1, _posXY2, _posXY3, _posXY4, _rgba, _thickness);
}

void DearsGraphicsEngine::UIFreeRectFilled(Vector2 _posXY1, Vector2 _posXY2, Vector2 _posXY3, Vector2 _posXY4, Vector4 _rgba)
{
	m_pUiRenderer->DrawFreeRectFilled(_posXY1, _posXY2, _posXY3, _posXY4, _rgba);
}

void DearsGraphicsEngine::UIFreeRectwithBorder(Vector2 _posXY1, Vector2 _posXY2, Vector2 _posXY3, Vector2 _posXY4, Vector4 _rgba, float _thickness, Vector4 _borderRgba)
{
	m_pUiRenderer->DrawFreeRectWithBorder(_posXY1, _posXY2, _posXY3, _posXY4, _rgba, _thickness, _borderRgba);
}

void DearsGraphicsEngine::UIDrawLine(Vector2 _sPosXY, Vector2 _ePosXY, Vector4 _rgba)
{
	m_pUiRenderer->DrawLine(_sPosXY, _ePosXY, _rgba);
}

void DearsGraphicsEngine::UIDrawCir(Vector2 _posXY, float _radius, Vector4 _rgba)
{
	m_pUiRenderer->DrawCircle(_posXY, _radius, _rgba);
}

void DearsGraphicsEngine::RenderImGui()
{
	m_pUiRenderer->Render();
}

void DearsGraphicsEngine::EndRenderImGui()
{
	m_pUiRenderer->EndFrame();
}

void DearsGraphicsEngine::AddEditorPanel(IEditorPanel* panel)
{
	m_pUiRenderer->AddEditorPanel(panel);
}

void DearsGraphicsEngine::SetRenderViewportWidth(int viewportWidth)
{
	mpRenderer->SetViewportWidth(viewportWidth);

	// 에디터 오른쪽 패널은 3D Viewport에 포함되지 않지만, G-Buffer 리소스 자체는
	// Back Buffer와 같은 전체 창 크기를 유지해야 한다.
	//
	// Lighting 뒤의 Forward Pass는 Back Buffer RTV와 G-Buffer DSV를 동시에 묶는다.
	// DX11은 이때 RTV와 DSV의 Width/Height 및 Sample Count가 모두 같아야 하므로,
	// G-Buffer를 viewportWidth(현재 1600)로 줄이면 1920 너비의 Back Buffer와 함께
	// 바인딩할 수 없고 Forward 물체가 Deferred 결과를 올바르게 깊이 검사하지 못한다.
	//
	// 실제 3D 렌더 범위는 위의 SetViewportWidth()가 제한한다. 따라서 전체 크기의
	// G-Buffer 중 왼쪽 viewportWidth 영역만 사용되며 픽셀 좌표도 그대로 일치한다.
	if (m_pGBuffer && viewportWidth > 0)
	{
		m_pGBuffer->Resize(
			static_cast<UINT>(m_screenWidth),
			static_cast<UINT>(m_screenHeight));

		// Editor Panel이 ImGui DrawData를 만들기 전에 파생 디버그 텍스처도 같은 크기로 맞춘다.
		// 이전에는 Lighting Pass 안에서 처음 Resize되면서, 이미 ImGui에 전달된 SRV가
		// 해제되어 ImGui_ImplDX11_RenderDrawData()에서 접근 위반이 발생했다.
		if (m_pGBufferDebugRenderer)
		{
			m_pGBufferDebugRenderer->Resize(
				static_cast<UINT>(m_screenWidth),
				static_cast<UINT>(m_screenHeight));
		}
	}
}

void DearsGraphicsEngine::LightInitialize(CommonConstantBufferData* _psBufferData, UINT _num)
{
	mpLightHelper->Initialize(_psBufferData, _num);
}

void DearsGraphicsEngine::ChangeLightMaxNum(CommonConstantBufferData* _psBufferData, UINT _num)
{
	mpLightHelper->ChangeLightMaxNum(_psBufferData, _num);
}

void DearsGraphicsEngine::LightUpdate(CommonConstantBufferData* _psBufferData)
{
	//mpLightHelper->Update(_psBufferData);
	//TODO : 占쏙옙占썩서 占쌓몌옙占쏙옙 占쏙옙占쏙옙占쏙옙트占쏙옙占쏙옙 占쏙옙占쌍는곤옙 占쏙옙占싣븝옙占싸댐옙.
	mpRenderer->LightUpdate(_psBufferData);



}

void DearsGraphicsEngine::PrintLightInfo(CommonConstantBufferData* _psBufferData)
{
	mpLightHelper->PrintInfo(_psBufferData);
}

void DearsGraphicsEngine::SetLightSettingAll(CommonConstantBufferData* _psBufferData, UINT _index, LightEnum _lightType, float _strength, float _fallOffStart,
											 float _fallOffEnd, Vector3 _dir, Vector3 _pos, float _spotPower, Vector3 _color)
{
	mpLightHelper->SetLightSettingAll(_psBufferData, _index, _lightType, _strength, _fallOffStart, _fallOffEnd, _dir, _pos, _spotPower, _color);
}

void DearsGraphicsEngine::SetDirLight(CommonConstantBufferData* _psBufferData, UINT _index, float _strength, Vector3 _dir, Vector3 _color)
{
	_dir.Normalize();
	mpLightHelper->SetDirLight(_psBufferData, _index, _strength, _dir, _color);
}

void DearsGraphicsEngine::SetPointLight(CommonConstantBufferData* _psBufferData, UINT _index, float _strength, float _fallOffStart, float _fallOffEnd, Vector3 _pos, Vector3 _color)
{
	mpLightHelper->SetPointLight(_psBufferData, _index, _strength, _fallOffStart, _fallOffEnd, _pos, _color);
}

void DearsGraphicsEngine::SetSpotLight(CommonConstantBufferData* _psBufferData, UINT _index, float _strength, float _fallOffStart, float _fallOffEnd, Vector3 _dir, Vector3 _pos, float _spotPower, Vector3 _color)
{
	//_dir.Normalize();
	mpLightHelper->SetSpotLight(_psBufferData, _index, _strength, _fallOffStart, _fallOffEnd, _dir, _pos, _spotPower, _color);
}

void DearsGraphicsEngine::SetLightOff(CommonConstantBufferData* _psBufferData, UINT _index)
{
	mpLightHelper->SetLightOff(_psBufferData, _index);
}

void DearsGraphicsEngine::SetLightStrength(CommonConstantBufferData* _psBufferData, UINT _index, float _strength)
{
	mpLightHelper->SetStrength(_psBufferData, _index, _strength);
}

void DearsGraphicsEngine::SetLightType(CommonConstantBufferData* _psBufferData, UINT _index, LightEnum _lightType)
{
	mpLightHelper->SetLightType(_psBufferData, _index, _lightType);
}

void DearsGraphicsEngine::SetLightDir(CommonConstantBufferData* _psBufferData, UINT _index, Vector3 _dir)
{
	mpLightHelper->SetDirection(_psBufferData, _index, _dir);
}

void DearsGraphicsEngine::SetLightFallOffStart(CommonConstantBufferData* _psBufferData, UINT _index, float _distance)
{
	mpLightHelper->SetFallOffStart(_psBufferData, _index, _distance);
}

void DearsGraphicsEngine::SetLightFallOffEnd(CommonConstantBufferData* _psBufferData, UINT _index, float _length)
{
	mpLightHelper->SetFallOffEnd(_psBufferData, _index, _length);
}

void DearsGraphicsEngine::SetLightPos(CommonConstantBufferData* _psBufferData, UINT _index, Vector3 _pos)
{
	mpLightHelper->SetPosition(_psBufferData, _index, _pos);
}

void DearsGraphicsEngine::SetLightSpotPower(CommonConstantBufferData* _psBufferData, UINT _index, float _power)
{
	mpLightHelper->SetSpotPower(_psBufferData, _index, _power);
}

void DearsGraphicsEngine::SetLightColor(CommonConstantBufferData* _psBufferData, UINT _index, Vector3 _rgb)
{
	mpLightHelper->SetLightColor(_psBufferData, _index, _rgb);
}

void DearsGraphicsEngine::CopyLight(CommonConstantBufferData* _psBufferData, UINT _copy, UINT _origin)
{
	mpLightHelper->CopyLight(_psBufferData, _copy, _origin);
}

Matrix DearsGraphicsEngine::CreateShadowViewMatrix(const Light& light)
{
	Vector3 up = Vector3(0.0f, 1.0f, 0.0f);

	// 占쏙옙占쏙옙占쏙옙 占싣뤄옙占쏙옙占쏙옙 占쌕라보곤옙 占쌍댐옙 占쏙옙占? up 占쏙옙占싶몌옙 占쏙옙占쏙옙占쏙옙킵占싹댐옙.
	if (light.direction == up || light.direction == -up)
	{
		up = Vector3(0.0f, 0.0f, 1.0f);
	}

	// LookAt 占쌉쇽옙占쏙옙 占쏙옙占쏙옙占?shadowView 占쏙옙트占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙
	
	return Matrix::CreateLookAt(light.position, light.position + light.direction, up);
}

Matrix DearsGraphicsEngine::CreateShadowProjectionMatrix(const Light& light, float nearPlane, float farPlane, float fieldOfView, float aspectRatio)
{
	if (light.lightType == static_cast<int>(LightEnum::DIRECTIONAL_LIGHT))
	{
		// 占쏙옙占쏙옙 占쏙옙占쏙옙 占쏙옙트占쏙옙占쏙옙 (Orthographic Projection)
		return Matrix::CreateOrthographic(m_screenWidth, m_screenHeight, nearPlane, farPlane);
	}
	else
	{
		// 占쏙옙占쏙옙 占쏙옙占쏙옙 占쏙옙트占쏙옙占쏙옙 (Perspective Projection)
		return Matrix::CreatePerspectiveFieldOfView(fieldOfView, aspectRatio, nearPlane, farPlane);
	}
}

bool DearsGraphicsEngine::EnsureViewportSurface(int width, int height)
{
    if (width <= 0 || height <= 0) return false;
    // Resize happens before any current-frame panel can retain an SRV in DrawData.
    m_pDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
    ID3D11ShaderResourceView* empty[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
    m_pDeviceContext->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, empty);
    m_pDeviceContext->VSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, empty);
    m_isGBufferBoundAsInput = false;
    m_boundRenderTargetInputSlots.clear();
    return mpRenderer->SetWorldSize(width, height)
        && m_pGBuffer->Resize(UINT(width), UINT(height))
        && (!m_pGBufferDebugRenderer || m_pGBufferDebugRenderer->Resize(UINT(width), UINT(height)))
        && m_pRenderTargetPool->ResizeDefaultTargets(UINT(width), UINT(height));
}

bool DearsGraphicsEngine::ResizeClient(int width, int height)
{
    if (!mpRenderer->ResizeClient(width, height)) return false;
    m_screenWidth = width; m_screenHeight = height;
    return true;
}
