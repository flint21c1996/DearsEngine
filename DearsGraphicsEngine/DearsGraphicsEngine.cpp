#if DEBUG
#incldue<iostream>
#endif
#include "DearsGraphicsEngine.h"
#include "GraphicsCommon.h"


DearsGraphicsEngine::DearsGraphicsEngine(HWND _hWnd, int screenWidth, int screenHeight)
{
	m_hWnd = _hWnd;
	m_screenHeight = screenHeight;
	m_screenWidth = screenWidth;
	m_pResourceManager = nullptr;
	mpRenderer = nullptr;
	m_pDearsImGui = nullptr;
	m_pTargetCamera = nullptr;
	mpAnimationHelper = nullptr;
	mpLightHelper = nullptr;
}

DearsGraphicsEngine::~DearsGraphicsEngine()
{

	delete m_pDearsImGui;
	delete mpRenderer;
	delete m_pResourceManager;
	delete m_pTargetCamera;
	delete mpAnimationHelper;

}
//명시적으로 초기화를 시킨다.
void DearsGraphicsEngine::Initialize()
{
	//우선 디바이스 생성을 먼저한다. 	//  스왑 체인, 뷰 포트, 깊이 스텐실 , 렌더 타겟 뷰 생성등은 Renderer안의 InitalizeD3D함수 안에서한다.

	RendererHelper::CreateDevice(m_pDevice, m_pDeviceContext);
	
	//렌더러 생성
	mpRenderer = new Renderer(m_hWnd, 0, 0, m_screenWidth, m_screenHeight, m_pDevice, m_pDeviceContext);
	mpRenderer->Initialize(m_pResourceManager);

	//리소스 매니저 생성
	m_pResourceManager = new GraphicsResourceManager(m_pDevice, m_pDeviceContext);

	//애니메이션 헬퍼클래스 생성
	mpAnimationHelper = new AnimationHelper();

	//ImGUI 생성
	m_pDearsImGui = new DearsImGui(m_hWnd, m_pDevice, m_pDeviceContext, m_screenWidth, m_screenHeight, m_pResourceManager);
	

	//라이트 헬퍼생성
	mpLightHelper = new LightHelper;

 	Debug_ModelBuffer = new ModelBuffer;
	AddModel("../TestAsset/", "Debug_Box.fbx");
	AddModel("../TestAsset/", "Debug_Capsule.fbx");
	AddModel("../TestAsset/", "Debug_Sphere.fbx");

 	VSConstantBufferData temp;
 	Debug_ModelBuffer->m_pVSConstantBuffer = CreateConstantBuffer(temp);

	Dears::Graphics::InitCommonStates(m_pDevice);

	MeshData cubeMeshData = GeometryGenerator::MakeBox(1000);
	std::reverse(cubeMeshData.indices.begin(), cubeMeshData.indices.end());
	m_pResourceManager->AddModel(cubeMeshData, "CubeMap");

	MeshData SphereMeshData = GeometryGenerator::MakeSphere(1.f, 300, 300);
	m_pResourceManager->AddModel(SphereMeshData, "MySphere");

	MeshData SquareMeshData = GeometryGenerator::MakeSquare(1);
	m_pResourceManager->AddModel(SquareMeshData, "MySquare");

	MeshData BillBoardSquareMeshData = GeometryGenerator::BillboradSquare(1);
	m_pResourceManager->AddModel(BillBoardSquareMeshData, "BillBoardSquare");

	MeshData boxMeshData = GeometryGenerator::MakeBox(1);
	m_pResourceManager->AddModel(boxMeshData, "MyBox");

	m_pParticleManager = new ParticleManager(m_pDevice, m_pDeviceContext, MAX_PARTICLE);
	m_pParticleManager->Initialize();
	m_pParticleManager->SetVertexBufferAndIndexBuffer(m_pResourceManager->Get_VertexBuffer("BillBoardSquare"), 
														m_pResourceManager->Get_IndexBuffer("BillBoardSquare"),
														m_pResourceManager->Get_NumIndex("BillBoardSquare"));
	PostProcessingBuffer = new ModelBuffer;
	PostProcessingBuffer->m_pVertexBuffer = Get_VertexBuffer("BillBoardSquare");
	PostProcessingBuffer->m_pIndexBuffer = Get_IndexBuffer("BillBoardSquare");
	PostProcessingBuffer->mNumIndices = Get_NumIndex("BillBoardSquare");
}

void DearsGraphicsEngine::Update()
{
	m_pParticleManager->Update();
}

void DearsGraphicsEngine::BeginRender()
{
	mpRenderer->BeginRender();
	UIBegineRender();
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

void DearsGraphicsEngine::RendPostProcessing()
{
	mpRenderer->SetPipelineState(Dears::Graphics::samplerPSO);
	for (int i = 0; i < 10; i++)
	{
	mpRenderer->RenderSampler(PostProcessingBuffer);

	mpRenderer->SetPipelineState(Dears::Graphics::postEffectPSO);
	mpRenderer->RenderPostProcessing(PostProcessingBuffer);
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
void DearsGraphicsEngine::AddDDSTexture(std::string _basePath, std::string _fileName)
{
	m_pResourceManager->AddDDSTexture(_basePath, _fileName);

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

void DearsGraphicsEngine::UpdateCommonConstantBuffer(CommonConstantBufferData& _CommonBufferData)
{
 	_CommonBufferData.view = m_pTargetCamera->GetViewRow().Transpose();		// 시점 변환
 	_CommonBufferData.proj = m_pTargetCamera->GetProjRow().Transpose();
 	_CommonBufferData.viewProj = (m_pTargetCamera->GetViewRow() * m_pTargetCamera->GetProjRow()).Transpose();

	_CommonBufferData.invView = _CommonBufferData.view.Invert();
 	_CommonBufferData.invProj = _CommonBufferData.proj.Invert();
	
	//그림자 렌더링에 사용
 	_CommonBufferData.invViewProj = _CommonBufferData.viewProj.Invert();

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

void DearsGraphicsEngine::Set_CubeMap(std::string diffuseTextureName, std::string specularTextureName)
{
	mpRenderer->SetCommonShaderResource
	(
		m_pResourceManager->Get_Textures(diffuseTextureName),
		m_pResourceManager->Get_Textures(specularTextureName)
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
	SetPipelineState(Dears::Graphics::AnimeGeometryPSO);
	mpRenderer->Render(_modelBuffer);
}

void DearsGraphicsEngine::Rend_Model(ModelBuffer* _modelBuffer)
{
	SetPipelineState(Dears::Graphics::BasicGeometryPSO);
	mpRenderer->Render(_modelBuffer);
}

void DearsGraphicsEngine::Rend_EquipmentModel(ModelBuffer* _modelBuffer)
{
	SetPipelineState(Dears::Graphics::EquipmentGeometryPSO);
	mpRenderer->Render(_modelBuffer);
}

void DearsGraphicsEngine::SetOpacityFactor(float blendFactor[4])
{
	for (int i = 0; i < 4; i++)
	{
	Dears::Graphics::OpacityPSO.m_blendFactor[i] = blendFactor[i];
	}
}

void DearsGraphicsEngine::Rend_OpacitiyModel(ModelBuffer* _modelBuffer)
{
	SetPipelineState(Dears::Graphics::OpacityPSO);
	mpRenderer->Render(_modelBuffer);
}

void DearsGraphicsEngine::Rend_EdgeModel(ModelBuffer* _modelBuffer)
{
	SetPipelineState(Dears::Graphics::PunchingPSO);
	mpRenderer->Render(_modelBuffer);
	SetPipelineState(Dears::Graphics::EdgePSO);
	mpRenderer->RenderEdge(_modelBuffer);

}

void DearsGraphicsEngine::Rend_Water(ModelBuffer* _modelBuffer)
{
	SetPipelineState(Dears::Graphics::WaterPSO);
	mpRenderer->Render(_modelBuffer);
}

void DearsGraphicsEngine::RenderDepthMap(ModelBuffer* _modelbuffer)
{
	mpRenderer->RenderDepthMap(_modelbuffer);
}

void DearsGraphicsEngine::RenderAniDepthMap(ModelBuffer* _modelbuffer)
{
	mpRenderer->RenderAniDepthMap(_modelbuffer);
}

void DearsGraphicsEngine::RenderEquipDepthMap(ModelBuffer* _modelbuffer)
{
	mpRenderer->RenderEquipDepthMap(_modelbuffer);
}

void DearsGraphicsEngine::Rend_InstancedModels(ModelBuffer* _modelbuffers)
{
	SetPipelineState(Dears::Graphics::BasicInstancingPSO);
	///여기서 렌더러를 거쳐 렌더링이 가능하도록
	mpRenderer->Rend_InstancedModels(_modelbuffers);
}

void DearsGraphicsEngine::Rend_BillBoard(ModelBuffer* _modelbuffers)
{
	SetPipelineState(Dears::Graphics::TestPSO);
	mpRenderer->Render(_modelbuffers);

}

void DearsGraphicsEngine::Rend_DebugBox(Vector3 _size, Vector3 _rotation, Vector3 _transpose)
{
	SetPipelineState(Dears::Graphics::DebugGeometryPSO);
	Debug_ModelBuffer->m_pVertexBuffer = Get_VertexBuffer("Box");
	Debug_ModelBuffer->m_pIndexBuffer = Get_IndexBuffer("Box");
	Debug_ModelBuffer->mNumIndices = Get_NumIndex("Box");
	VSConstantBufferData temp;
	temp.world = (Matrix::CreateScale(_size) * Matrix::CreateRotationX(_rotation.x) * Matrix::CreateRotationY(_rotation.y) *
		Matrix::CreateRotationZ(_rotation.z) * Matrix::CreateTranslation(_transpose)).Transpose();
	UpdateConstantBuffer(Debug_ModelBuffer, temp);
	mpRenderer->Render(Debug_ModelBuffer);

}

void DearsGraphicsEngine::Rend_DebugBox(Matrix _size, Matrix _rotation, Matrix _transpose)
{
	SetPipelineState(Dears::Graphics::DebugGeometryPSO);
	Debug_ModelBuffer->m_pVertexBuffer = Get_VertexBuffer("Box");
	Debug_ModelBuffer->m_pIndexBuffer = Get_IndexBuffer("Box");
	Debug_ModelBuffer->mNumIndices = Get_NumIndex("Box");

	VSConstantBufferData temp;
 	temp.world = (_size * _rotation * _transpose).Transpose();

	UpdateConstantBuffer(Debug_ModelBuffer, temp);

	mpRenderer->Render(Debug_ModelBuffer);
}

void DearsGraphicsEngine::Rend_DebugBox(Matrix _size, Matrix _rotation, Matrix _transpose, Matrix _tempMatrix = Matrix())
{
	SetPipelineState(Dears::Graphics::DebugGeometryPSO);
	Debug_ModelBuffer->m_pVertexBuffer = Get_VertexBuffer("Box");
	Debug_ModelBuffer->m_pIndexBuffer = Get_IndexBuffer("Box");
	Debug_ModelBuffer->mNumIndices = Get_NumIndex("Box");
	VSConstantBufferData temp;
	temp.world = _size * _rotation * _transpose* _tempMatrix;
	temp.world = temp.world.Transpose();
	
	UpdateConstantBuffer(Debug_ModelBuffer, temp);

	mpRenderer->Render(Debug_ModelBuffer);
}

void DearsGraphicsEngine::Rend_DebugBox(AABB& _AABB, Matrix Scale, Matrix _rotation, Matrix _tempMatrix )
{
	Vector3 _tempSize = { _AABB.mMax.x - _AABB.mMin.x,
		_AABB.mMax.y - _AABB.mMin.y,
		_AABB.mMax.z - _AABB.mMin.z,
	};
	Matrix tempSizeMatrix = Matrix::CreateScale(_tempSize);
	tempSizeMatrix *= Scale;

 	Vector3	centerAABB = {
 	(_AABB.mMax.x + _AABB.mMin.x) / 2.0f,
 	(_AABB.mMax.y + _AABB.mMin.y) / 2.0f,
 	(_AABB.mMax.z + _AABB.mMin.z) / 2.0f
 	};
	//centerAABB *= Scale;
	Matrix tempTranslationMatrix = Matrix::CreateTranslation(centerAABB);
	Rend_DebugBox(tempSizeMatrix, _rotation, tempTranslationMatrix, _tempMatrix);
}

void DearsGraphicsEngine::Rend_DebugSphere(Vector3 _size, Vector3 _rotation, Vector3 _transpose)
{
	SetPipelineState(Dears::Graphics::DebugGeometryPSO);
	Debug_ModelBuffer->m_pVertexBuffer = Get_VertexBuffer("Sphere");
	Debug_ModelBuffer->m_pIndexBuffer = Get_IndexBuffer("Sphere");
	Debug_ModelBuffer->mNumIndices = Get_NumIndex("Sphere");

	VSConstantBufferData temp;
	temp.world = (Matrix::CreateScale(_size) * Matrix::CreateRotationX(_rotation.x) * Matrix::CreateRotationY(_rotation.y) *
		Matrix::CreateRotationZ(_rotation.z) * Matrix::CreateTranslation(_transpose)).Transpose();

	UpdateConstantBuffer(Debug_ModelBuffer, temp);

	mpRenderer->Render(Debug_ModelBuffer);
}

void DearsGraphicsEngine::Rend_DebugCapsule(Vector3 _size, Vector3 _rotation, Vector3 _transpose)
{
	SetPipelineState(Dears::Graphics::DebugGeometryPSO);
	Debug_ModelBuffer->m_pVertexBuffer = Get_VertexBuffer("Capsule");
	Debug_ModelBuffer->m_pIndexBuffer = Get_IndexBuffer("Capsule");
	Debug_ModelBuffer->mNumIndices = Get_NumIndex("Capsule");

	VSConstantBufferData temp;
	temp.world = (Matrix::CreateScale(_size) * Matrix::CreateRotationX(_rotation.x) * Matrix::CreateRotationY(_rotation.y) *
		Matrix::CreateRotationZ(_rotation.z) * Matrix::CreateTranslation(_transpose)).Transpose();

	UpdateConstantBuffer(Debug_ModelBuffer, temp);

	mpRenderer->Render(Debug_ModelBuffer);
}

void DearsGraphicsEngine::Rend_CubeMap(ModelBuffer* _modelBuffer)
{
	SetPipelineState(Dears::Graphics::CubeMapGeometryPSO);
	mpRenderer->Render_CubeMap(_modelBuffer);
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
	//TODO : 여기서 그림자 업데이트까지 해주는게 좋아보인다.
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

	// 조명이 아래쪽을 바라보고 있는 경우, up 벡터를 반전시킵니다.
	if (light.direction == up || light.direction == -up)
	{
		up = Vector3(0.0f, 0.0f, 1.0f);
	}

	// LookAt 함수를 사용해 shadowView 매트릭스를 생성
	
	return Matrix::CreateLookAt(light.position, light.position + light.direction, up);
}

Matrix DearsGraphicsEngine::CreateShadowProjectionMatrix(const Light& light, float nearPlane, float farPlane, float fieldOfView, float aspectRatio)
{
	if (light.lightType == static_cast<int>(LightEnum::DIRECTIONAL_LIGHT))
	{
		// 직교 투영 매트릭스 (Orthographic Projection)
		return Matrix::CreateOrthographic(m_screenWidth, m_screenHeight, nearPlane, farPlane);
	}
	else
	{
		// 원근 투영 매트릭스 (Perspective Projection)
		return Matrix::CreatePerspectiveFieldOfView(fieldOfView, aspectRatio, nearPlane, farPlane);
	}
}
