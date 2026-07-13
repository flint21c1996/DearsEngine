#if DEBUG
#incldue<iostream>
#endif
#include "DearsGraphicsEngine.h"
#include "DebugRenderer.h"
#include "GraphicsAssetManager.h"
#include "GraphicsCommon.h"
#include "GBuffer.h"
#include "GBufferDebugPanel.h"
#include "GBufferDebugRenderer.h"
#include "MeshRenderer.h"
#include "ParticleRenderer.h"
#include "PostProcessRenderer.h"
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

void DearsGraphicsEngine::BeginRender()
{
	mpRenderer->BeginRender();
	UIBegineRender();
	m_pUiRenderer->DrawRegisteredPanels();
	UICanvasSet(Vector2(0, 0), Vector2(static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight)));
	SetUICurrentWindow();
}


void DearsGraphicsEngine::EndRender()
{

	RenderImGui();
	EndRenderImGui();
	mpRenderer->EndRender();
}

void DearsGraphicsEngine::Finalize()
{

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

	// 패스 타입에 따라 출력/입력 리소스를 한 곳에서 바꾼다.
	// GameEngine은 순서만 결정하고 DX11의 RTV/SRV 바인딩 규칙은 몰라도 된다.
	if (!m_pGBuffer)
	{
		return;
	}

	switch (renderContext.passType)
	{
	case RenderPassType::Geometry:
		m_pGBuffer->BindForGeometryPass();
		break;
	case RenderPassType::Lighting:
		// Geometry Pass의 MRT 쓰기를 먼저 끝내고 G-Buffer를 t20~t23에 연결한다.
		// GBuffer가 기존 출력 타깃을 해제한 다음 백 버퍼를 묶어야,
		// Lighting 셰이더가 G-Buffer를 읽으면서 최종 색상을 백 버퍼에 쓸 수 있다.
		m_pGBuffer->BindForLightingPass(GBuffer::LightingShaderSlot);
		// Material 채널 분리와 Depth 기반 Position 복원 결과를 디버그용 텍스처에 생성한다.
		if (m_pGBufferDebugRenderer)
		{
			m_pGBufferDebugRenderer->Render();
		}
		mpRenderer->BindMainRenderTarget();
		break;
	case RenderPassType::Forward:
		// Lighting 이후의 포워드 물체는 Geometry Pass에서 만든 깊이를 공유해야 한다.
		// Depth 텍스처가 Lighting용 SRV로 남아 있으면 같은 리소스를 DSV로 묶을 수 없으므로,
		// G-Buffer SRV를 모두 해제한 다음 Geometry depth를 출력 병합 단계에 연결한다.
		m_pGBuffer->UnbindShaderResources(GBuffer::LightingShaderSlot);
		mpRenderer->BindMainRenderTarget(m_pGBuffer->GetDepthStencilView());
		break;
	default:
		break;
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

void DearsGraphicsEngine::RenderAniDepthMap(ModelBuffer* _modelbuffer)
{
	m_pMeshRenderer->RenderAnimatedDepthMap(_modelbuffer);
}

void DearsGraphicsEngine::RenderEquipDepthMap(ModelBuffer* _modelbuffer)
{
	m_pMeshRenderer->RenderEquipmentDepthMap(_modelbuffer);
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
	float shadowFovYDegrees,
	float shadowAspect,
	float shadowNear,
	float shadowFar)
{
	m_pDebugRenderer->RenderLightGizmo(
		light,
		drawShadowFrustum,
		shadowFovYDegrees,
		shadowAspect,
		shadowNear,
		shadowFar);
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

	// 에디터 오른쪽 패널은 3D 렌더 영역에 포함되지 않는다.
	// G-Buffer도 실제 렌더 뷰포트와 같은 크기로 만들어야 픽셀 좌표와 UV가 일치한다.
	if (m_pGBuffer && viewportWidth > 0)
	{
		m_pGBuffer->Resize(static_cast<UINT>(viewportWidth), static_cast<UINT>(m_screenHeight));

		// Editor Panel이 ImGui DrawData를 만들기 전에 파생 디버그 텍스처도 같은 크기로 맞춘다.
		// 이전에는 Lighting Pass 안에서 처음 Resize되면서, 이미 ImGui에 전달된 SRV가
		// 해제되어 ImGui_ImplDX11_RenderDrawData()에서 접근 위반이 발생했다.
		if (m_pGBufferDebugRenderer)
		{
			m_pGBufferDebugRenderer->Resize(
				static_cast<UINT>(viewportWidth),
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
