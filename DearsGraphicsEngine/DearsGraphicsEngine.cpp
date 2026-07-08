#if DEBUG
#incldue<iostream>
#endif
#include "DearsGraphicsEngine.h"
#include "DebugRenderer.h"
#include "GraphicsCommon.h"
#include "MeshRenderer.h"
#include "PostProcessRenderer.h"


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

	mpAnimationHelper = std::make_unique<AnimationHelper>();

	m_pDearsImGui = std::make_unique<DearsImGui>(m_hWnd, m_pDevice, m_pDeviceContext, m_screenWidth, m_screenHeight, m_pResourceManager.get());

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

	m_pParticleManager = std::make_unique<ParticleManager>(m_pDevice, m_pDeviceContext, MAX_PARTICLE);
	m_pParticleManager->Initialize();
	m_pParticleManager->SetVertexBufferAndIndexBuffer(m_pResourceManager->Get_VertexBuffer("BillBoardSquare"), 
														m_pResourceManager->Get_IndexBuffer("BillBoardSquare"),
														m_pResourceManager->Get_NumIndex("BillBoardSquare"));

	m_pPostProcessRenderer = std::make_unique<PostProcessRenderer>(this);
	m_pPostProcessRenderer->Initialize();
}

void DearsGraphicsEngine::Update()
{
	m_pParticleManager->Update();
}

void DearsGraphicsEngine::BeginRender()
{
	mpRenderer->BeginRender();
	UIBegineRender();
	m_pDearsImGui->UISetting();
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
	m_pParticleManager->Render();
}

void DearsGraphicsEngine::AddParticle(unsigned int particleNum, CSParticleData& particleData)
{
	// 씬은 "파티클을 몇 개 생성할지"와 "어떤 데이터로 생성할지"만 요청한다.
	// 실제 파티클 풀, StructuredBuffer, UAV/SRV 같은 DX11 세부 구현은 ParticleManager 안에 숨긴다.
	m_pParticleManager->AddParticle(particleNum, particleData);
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
	m_pResourceManager->AddModel(_basePath, _fileName);
}

void DearsGraphicsEngine::AddAnimation(std::string _basePath, std::string _fileName)
{
	m_pResourceManager->AddAnimation(_basePath, _fileName);

}

void DearsGraphicsEngine::Add3DTexture(std::string _basePath, std::string _fileName)
{
	m_pResourceManager->Add3DTexture(_basePath, _fileName);

}

void DearsGraphicsEngine::Add2DTexture(std::string _basePath, std::string _fileName)
{
	m_pResourceManager->Add2DTexture(_basePath, _fileName);

}
void DearsGraphicsEngine::AddDDSTexture(std::string _basePath, std::string _fileName, bool isCubeMap)
{
	m_pResourceManager->AddDDSTexture(_basePath, _fileName, isCubeMap);

}

void DearsGraphicsEngine::Add2DMipMapTexture(std::string _basePath, std::string _fileName)
{
	m_pResourceManager->Add2DMipMapTexture(_basePath, _fileName);
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
	
	//占쌓몌옙占쏙옙 占쏙옙占쏙옙占쏙옙占쏙옙 占쏙옙占? 	_CommonBufferData.invViewProj = _CommonBufferData.viewProj.Invert();

	_CommonBufferData.eyeWorld = m_pTargetCamera->GetmViewPos();

	mpRenderer->UpdateCommonConstantBuffer(_CommonBufferData);
	
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
	m_pDearsImGui->UILoadFonts(_basePath, _fileName, _size, _isKorean);
}

void DearsGraphicsEngine::FontSetFinish()
{
	m_pDearsImGui->UIBuildFonts();
}

void DearsGraphicsEngine::UIBegineRender()
{
	m_pDearsImGui->UIBeginRender();
}

void DearsGraphicsEngine::UICanvasSet(Vector2 _posXY, Vector2 _sizeWH)
{
	m_pDearsImGui->UICanvasSet(_posXY, _sizeWH);
}

void DearsGraphicsEngine::UIDrawImageStart()
{
	m_pDearsImGui->UIDrawImageStart();
}

void DearsGraphicsEngine::UIDrawImage(Vector2 _posXY, Vector2 _sizeWH, std::string _textureName, Vector4 _rgba /*= Vector4(1.0f, 1.0f, 1.0f, 1.0f)*/)
{
	ComPtr<ID3D11ShaderResourceView> tempSRV = m_pResourceManager->Get_Textures(_textureName);
	m_pDearsImGui->UIDrawImage(_posXY, _sizeWH, tempSRV, _rgba);
}

void DearsGraphicsEngine::UIDrawImageFin()
{
	m_pDearsImGui->UIDrawImageFin();
}

void DearsGraphicsEngine::UIStartFontID(std::string _fontName)
{
	m_pDearsImGui->UIStartFontID(m_pResourceManager->Get_Font(_fontName));
}

void DearsGraphicsEngine::UIDrawText(Vector2 _pos, std::u8string _text, Vector4 _rgba)
{
	m_pDearsImGui->UIDrawText(_pos, m_pDearsImGui->ConvertUTF8String(_text), _rgba);
}

void DearsGraphicsEngine::UIFinFontID()
{
	m_pDearsImGui->UIEndFontID();
}

void DearsGraphicsEngine::SetUICurrentWindow()
{
	m_pDearsImGui->SetUICurrentWindow();
}

void DearsGraphicsEngine::UIDrawRect(Vector2 _posXY, Vector2 _sizeWH, Vector4 _rgba, float _rounding /*= 0.0f*/, float _thickness)
{
	m_pDearsImGui->UIDrawRect(_posXY, _sizeWH, _rgba, _rounding, _thickness);
}

void DearsGraphicsEngine::UIDrawRectFilled(Vector2 _posXY, Vector2 _sizeWH, Vector4 _rgba, float _rounding /*= 0.0f*/)
{
	m_pDearsImGui->UIDrawRectFilled(_posXY, _sizeWH, _rgba, _rounding);
}

void DearsGraphicsEngine::UIDrawRectwithBorder(Vector2 _posXY, Vector2 _sizeWH, Vector4 _rgba, float _rounding /*= 0.0f*/, float _thickness)
{
	m_pDearsImGui->UIDrawRectwithBorder(_posXY, _sizeWH, _rgba, _rounding, _thickness);
}

void DearsGraphicsEngine::UIFreeRect(Vector2 _posXY1, Vector2 _posXY2, Vector2 _posXY3, Vector2 _posXY4, Vector4 _rgba, float _thickness)
{
	m_pDearsImGui->UIFreeRect(_posXY1, _posXY2, _posXY3, _posXY4, _rgba, _thickness);
}

void DearsGraphicsEngine::UIFreeRectFilled(Vector2 _posXY1, Vector2 _posXY2, Vector2 _posXY3, Vector2 _posXY4, Vector4 _rgba)
{
	m_pDearsImGui->UIFreeRectFilled(_posXY1, _posXY2, _posXY3, _posXY4, _rgba);
}

void DearsGraphicsEngine::UIFreeRectwithBorder(Vector2 _posXY1, Vector2 _posXY2, Vector2 _posXY3, Vector2 _posXY4, Vector4 _rgba, float _thickness, Vector4 _borderRgba)
{
	m_pDearsImGui->UIFreeRectwithBorder(_posXY1, _posXY2, _posXY3, _posXY4, _rgba, _thickness, _borderRgba);
}

void DearsGraphicsEngine::UIDrawLine(Vector2 _sPosXY, Vector2 _ePosXY, Vector4 _rgba)
{
	m_pDearsImGui->UIDrawLine(_sPosXY, _ePosXY, _rgba);
}

void DearsGraphicsEngine::UIDrawCir(Vector2 _posXY, float _radius, Vector4 _rgba)
{
	m_pDearsImGui->UIDrawCircle(_posXY, _radius, _rgba);
}

void DearsGraphicsEngine::RenderImGui()
{
	m_pDearsImGui->UICanvasSetFin();
	m_pDearsImGui->UIRender();
}

void DearsGraphicsEngine::EndRenderImGui()
{
	m_pDearsImGui->UIEndRender();
}

void DearsGraphicsEngine::AddEditorPanel(IEditorPanel* panel)
{
	m_pDearsImGui->AddPanel(panel);
}

void DearsGraphicsEngine::SetRenderViewportWidth(int viewportWidth)
{
	mpRenderer->SetViewportWidth(viewportWidth);
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
