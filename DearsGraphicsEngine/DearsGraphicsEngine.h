#pragma once
#include <windows.h>
#include <wrl.h>
#include <directxtk/SimpleMath.h>
#include <map>
#include <memory>
#include <string>

#include "AnimationHelper.h"
#include "BufferData.h"
#include "GraphicsAssetManager.h"
#include "GraphicsResourceManager.h"
#include "LightHelper.h"
#include "ModelBuffer.h"
#include "Renderer.h"
#include "RenderContext.h"
#include "UiRenderer.h"
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;
using DirectX::SimpleMath::Quaternion;

class DebugRenderer;
class GraphicsAssetManager;
class MeshRenderer;
class ParticleRenderer;
class PostProcessRenderer;
struct RenderContext;
class UiRenderer;

class DearsGraphicsEngine
{
public:
	DearsGraphicsEngine(HWND _hWnd, int screenWidth, int screenHeight);
	~DearsGraphicsEngine();

	/// Window占쏙옙占쏙옙 占쌉쇽옙
private:
	HWND m_hWnd;
	int m_screenWidth;
	int m_screenHeight;

	ComPtr<ID3D11Device> m_pDevice;										
	ComPtr<ID3D11DeviceContext> m_pDeviceContext;						

private:
	// DearsGraphicsEngine이 소유하는 그래픽스 구현 객체들이다.
	//
	// 예전에는 public으로 열려 있어서 GameEngine/Scene/Object가
	// m_pResourceManager나 mpRenderer를 직접 타고 들어갈 수 있었다.
	// 그렇게 되면 외부 코드가 DX11 구현 클래스에 묶이기 때문에,
	// DX12/Vulkan 백엔드로 교체할 때 바깥 코드까지 같이 흔들린다.
	//
	// 그래서 외부 계층은 DearsGraphicsEngine의 public 함수만 사용하고,
	// 실제 DX11 구현 포인터는 그래픽스 엔진 내부에서만 다룬다.
	std::unique_ptr<Renderer> mpRenderer;
	std::unique_ptr<GraphicsAssetManager> m_pAssetManager;
	std::unique_ptr<GraphicsResourceManager> m_pResourceManager;
	std::unique_ptr<AnimationHelper> mpAnimationHelper;
	std::unique_ptr<LightHelper> mpLightHelper;

	// 그래픽스 엔진 내부에서 소유하고 관리하는 렌더링 하위 시스템들이다.
	// 외부 코드가 이 포인터들을 직접 타고 들어가면 DX11 기반 구현에 강하게 묶이므로,
	// 필요한 기능은 DearsGraphicsEngine의 public 함수로 한 번 감싸서 제공한다.
	std::unique_ptr<DebugRenderer> m_pDebugRenderer;
	std::unique_ptr<MeshRenderer> m_pMeshRenderer;
	std::unique_ptr<ParticleRenderer> m_pParticleRenderer;
	std::unique_ptr<PostProcessRenderer> m_pPostProcessRenderer;
	std::unique_ptr<UiRenderer> m_pUiRenderer;

	// 현재 적용된 렌더 패스 종류이다.
	// 지금은 디버깅용 상태에 가깝지만, 나중에 RHI를 붙이면
	// BeginRenderPass/EndRenderPass, resource transition, profiler marker가
	// 이 값을 기준으로 동작할 수 있다.
	RenderPassType m_currentRenderPassType = RenderPassType::Unknown;

	// 현재 공통 상수버퍼 계산에 사용할 카메라이다.
	// Camera의 소유권은 GameEngine/Scene 쪽에 있고, 그래픽스 엔진은 포인터만 빌려 쓴다.
	// 그래서 delete하지 않으며, 반드시 SetCamera() 또는 ApplyRenderContext()를 통해 갱신한다.
	Camera* m_pTargetCamera;

	// 하위 렌더러들은 아직 DX11 Renderer를 직접 호출한다.
	// 이 접근은 그래픽스 모듈 내부 구현으로만 허용하고, GameEngine/Scene 쪽에는 열지 않는다.
	// 다음 RHI 단계에서는 이 friend 접근도 IRenderDevice/IRenderCommandList 같은 인터페이스로 줄여간다.
	friend class DebugRenderer;
	friend class MeshRenderer;
	friend class PostProcessRenderer;

public:
	void Initialize();

	void Update();
	
	void BeginRender();
	
	void EndRender();

	void Finalize();

	void RendParticle();
	void AddParticle(unsigned int particleNum, CSParticleData& particleData);

	void RendPostProcessing();

	int GetScreenWidth() const;
	
	int GetScreenHeight() const;
	
	float GetAspectRatio();

	void AddModel(std::string _basePath, std::string _fileName);
	void AddAnimation(std::string _basePath, std::string _fileName);
	void Add3DTexture(std::string _basePath, std::string _fileName);
	void Add2DTexture(std::string _basePath, std::string _fileName);
	void AddDDSTexture(std::string _basePath, std::string _fileName, bool isCubeMap = true);
	void Add2DMipMapTexture(std::string _basePath, std::string _fileName);
	ComPtr<ID3D11Buffer> Get_VertexBuffer(std::string _modelName);
	ComPtr<ID3D11Buffer> Get_IndexBuffer(std::string _modelName);
	unsigned int Get_NumIndex(std::string _modelName);
	Model* Get_ModelInfo(std::string _modelName);
	Animation* Get_Animation(std::string _animeName);
	ComPtr<ID3D11ShaderResourceView> Get_Textures(std::string _textureName);
	ImFont* Get_Font(std::string _fontName);
	//
	int Get_TargetModelBoneIndex(std::string _modelName, std::string _boneName);

	Matrix GetTargetBoneMatrix(std::string _targetModel, std::string _targetBoneName);
	Matrix GetTargetBoneAboveMatrix(std::string _targetModel, std::string _targetBoneName, float _scale = 1.f);
	Matrix GetTargetBoneAboveMatrix(std::string _targetModel, int _index, float _scale = 1.f);

	AABB Get_AABB(std::string __targetModel);

	void Erase_VertexBuffer(std::string _modelName);
	void Erase_IndexBuffer(std::string _modelName);
	void Erase_NumIndex(std::string _modelName);
	void Erase_ModelInfo(std::string _modelName);
	void Erase_Animation(std::string _animName);
	void Erase_Textures(std::string _textureName);
	void Erase_Font(std::string _fontName);

	ComPtr<ID3D11Buffer> CreateConstantBuffer(VSConstantBufferData& _VsConstantBufferData);
	ComPtr<ID3D11Buffer> CreateConstantBuffer(VSBoneConstantBufferData& _VsBoneConstantBufferData);
	ComPtr<ID3D11Buffer> CreateConstantBuffer(VSTargetBoneConstantBufferData& _PsConstantBufferData);
	ComPtr<ID3D11Buffer> CreateConstantBuffer(PSConstantBufferData& _PsConstantBufferData);
	ComPtr<ID3D11Buffer> CreateConstantBuffer(PSEdgeConstantBufferData& _pPSEdgeConstantBuffer);
	ComPtr<ID3D11Buffer> CreateConstantBuffer(VSShadowConstantBufferData& _VsShadowConstantBufferData);
	ComPtr<ID3D11Buffer> CreateConstantBuffer(PsShadowConstantBufferData& _VsShadowConstantBufferData);
	ComPtr<ID3D11Buffer> CreateConstantBuffer(CommonConstantBufferData& _CommonConstantBufferData);
	ComPtr<ID3D11Buffer> CreateConstantBuffer(VSInstantConstantBufferData& _VSInstantConstantBufferData);

	//Strucured Buffer. 
	ComPtr<ID3D11Buffer> CreateStructuredBuffer(CSParticleData& _TestCSParticleData, unsigned int _count);


	template <typename T>
	ComPtr<ID3D11Buffer> CreateConstantBuffer(T& _bufferData)
	{
		return RendererHelper::CreateConstantBuffer(m_pDevice, _bufferData);
	}

	void UpdateCommonConstantBuffer(CommonConstantBufferData& _CommonBufferData);
	void ApplyRenderContext(const RenderContext& renderContext);
	RenderPassType GetCurrentRenderPassType() const { return m_currentRenderPassType; }
	const char* GetCurrentRenderPassName() const { return ToRenderPassName(m_currentRenderPassType); }

	void UpdateConstantBuffer(ModelBuffer* _pModelBuffer, VSConstantBufferData& _VsConstantBufferData);
	void UpdateBoneConstantBuffer(ModelBuffer* _pModelBuffer, VSBoneConstantBufferData& _VsBoneConstantBufferData);
	
	bool UpdateTransitionBoneConstantBuffer(ModelBuffer* _pModelBuffer, VSBoneConstantBufferData& _VsBoneConstantBufferData);
	void UpdateTargetBoneConstantBuffer(ModelBuffer* _pModelBuffer, VSTargetBoneConstantBufferData& _VsTargetBoneConstantBufferData);


	void UpdateConstantBuffer(ModelBuffer* _pModelBuffer, PSConstantBufferData& _PsConstantBufferData);

	void UpdateVSEdgeConstantBuffer(ModelBuffer* _pModelBuffer, VSEdgeConstantBufferData& _pPSEdgeConstantBuffer);

	void UpdatePSEdgeConstantBuffer(ModelBuffer* _pModelBuffer, PSEdgeConstantBufferData& _pPSEdgeConstantBuffer);

	void UpdateVSWaterConstantBuffer(ModelBuffer* _pModelBuffer, VSWaterConstantBufferData& _pPSEdgeConstantBuffer);
	void UpdateVSPBRConstantBuffer(ModelBuffer* _pModelBuffer, PBRVertexShaderConstantData& _PBRConstantData);
	void UpdatePSPBRConstantBuffer(ModelBuffer* _pModelBuffer, PBRPixelShaderConstantData& _PBRConstantData);
	void UpdatePSThinFilmonstantBuffer(ModelBuffer* _pModelBuffer, ThinFilmPixelShaderConstantData& _ThinFilmConstantData);

	void UpdateShadowConstantBuffer(ModelBuffer* _pModelBuffer, VSShadowConstantBufferData& _VsShadowConstantBufferData);
	void UpdateShadowConstantBuffer(ModelBuffer* _pModelBuffer, PsShadowConstantBufferData& _VsShadowConstantBufferData);

	void Set_CubeMap(std::string environmentTexture, std::string diffuseTextureName, std::string specularTextureName, std::string BRDFTextureName);
	
	void SetPipelineState(PipelineStateObject& _pso);
	void Rend_AnimateModel(ModelBuffer* _modelBuffer);
	void Rend_Model(ModelBuffer* _modelBuffer);

	void Rend_PBR(ModelBuffer* _modelBuffer);
	void Rend_ThinFilm(ModelBuffer* _modelBuffer);

	void Rend_EquipmentModel(ModelBuffer* _modelBuffer);
	void SetOpacityFactor(float blendFactor[4]);							
	void Rend_OpacitiyModel(ModelBuffer* _modelBuffer);						

	void Rend_EdgeModel(ModelBuffer* _modelBuffer);							
	void Rend_Water(ModelBuffer* _modelBuffer);


	void RenderDepthMap(ModelBuffer* _modelbuffer);
	void RenderAniDepthMap(ModelBuffer* _modelbuffer);
	void RenderEquipDepthMap(ModelBuffer* _modelbuffer);

	void Rend_InstancedModels(ModelBuffer* _modelbuffers);
	void Rend_BillBoard(ModelBuffer* _modelbuffers);

	void Rend_DebugBox(Vector3 _size, Vector3 _rotation, Vector3 _transpose);
	void Rend_DebugBox(Matrix _size, Matrix _rotation, Matrix _transpose);
	void Rend_DebugBox(Matrix _size, Matrix _rotation, Matrix _transpose, Matrix _tempMatrix);
	void Rend_DebugBox(AABB& _AABB, Matrix Scale, Matrix _rotation, Matrix _tempMatrix = Matrix());

	void Rend_DebugSphere(Vector3 _size, Vector3 _rotation, Vector3 _transpose);
	void Rend_DebugCapsule(Vector3 _size, Vector3 _rotation, Vector3 _transpose);

	void Rend_CubeMap(ModelBuffer* _modelBuffer);
	//占쏙옙占쏙옙占쏙옙占쏙옙 占십울옙占쏙옙 카占쌨띰옙 占쏙옙占쏙옙占싼댐옙.
	void SetCamera(Camera* _pTargetCamera);

	VSConstantBufferData m_VSConstantBufferData;
	PSConstantBufferData m_PSConstantBufferData;
	VSBoneConstantBufferData m_VSBoneConstantBufferData;

	/// UI -------------------------------------------------------------------------------------------------

	void AddEditorPanel(IEditorPanel* panel);
	void SetRenderViewportWidth(int viewportWidth);

	void AddFont(std::string _basePath, std::string _fileName, float _size, bool _isKorean);

	void FontSetFinish();

	void UIBegineRender();


	void UICanvasSet(Vector2 _posXY, Vector2 _sizeWH = Vector2());

	void UIDrawImageStart();

	
	void UIDrawImage(Vector2 _posXY, Vector2 _sizeWH, std::string _textureName, Vector4 _rgba = Vector4(1.0f, 1.0f, 1.0f, 1.0f));

	void UIDrawImageFin();

	void UIStartFontID(std::string _fontName);

	void UIDrawText(Vector2 _pos, std::u8string _text, Vector4 _rgba = Vector4(0.0f, 0.0f, 0.0f, 1.0f));

	template<typename ...Args>
	void UIDrawTextWithNum(Vector2 _posXY, const std::u8string _formatText, Vector4 _rgba = Vector4(0.0f, 0.0f, 0.0f, 1.0f), Args&& ...args);

	void UIFinFontID();

	// -----  -----------------------------------

	void SetUICurrentWindow();

	void UIDrawRect(Vector2 _posXY, Vector2 _sizeWH, Vector4 _rgba, float _rounding = 0.0f, float _thickness = 1.0f);

	void UIDrawRectFilled(Vector2 _posXY, Vector2 _sizeWH, Vector4 _rgba, float _rounding = 0.0f);							

	void UIDrawRectwithBorder(Vector2 _posXY, Vector2 _sizeWH, Vector4 _rgba, float _rounding = 0.0f, float _thickness = 1.0f);
	
	void UIFreeRect(Vector2 _posXY1, Vector2 _posXY2, Vector2 _posXY3, Vector2 _posXY4, Vector4 _rgba, float _thickness);
	
	void UIFreeRectFilled(Vector2 _posXY1, Vector2 _posXY2, Vector2 _posXY3, Vector2 _posXY4, Vector4 _rgba);
	
	void UIFreeRectwithBorder(Vector2 _posXY1, Vector2 _posXY2, Vector2 _posXY3, Vector2 _posXY4, Vector4 _rgba, float _thickness, Vector4 _borderRgba);

	void UIDrawLine(Vector2 _sPosXY, Vector2 _ePosXY, Vector4 _rgba);

	void UIDrawCir(Vector2 _posXY, float _radius, Vector4 _rgba);
	// -----------------------------------------------------

	void RenderImGui();

	void EndRenderImGui();

	/// 占쏙옙占쏙옙트 占쌜억옙 -------------------------------------------------------------------------------------------------
	void LightInitialize(CommonConstantBufferData* _psBufferData, UINT _num);

	void ChangeLightMaxNum(CommonConstantBufferData* _psBufferData, UINT _num);

	void LightUpdate(CommonConstantBufferData* _psBufferData);

	void PrintLightInfo(CommonConstantBufferData* _psBufferData);

	void SetLightSettingAll(CommonConstantBufferData* _psBufferData, UINT _index, LightEnum _lightType, float _strength, float _fallOffStart,
							float _fallOffEnd, Vector3 _dir, Vector3 _pos, float _spotPower, Vector3 _color = Vector3(1.0f, 1.0f, 1.0f));
	// Directional Light 
	void SetDirLight(CommonConstantBufferData* _psBufferData, UINT _index, float _strength, Vector3 _dir, Vector3 _color = Vector3(1.0f, 1.0f, 1.0f));

	// Point Light 
	void SetPointLight(CommonConstantBufferData* _psBufferData, UINT _index, float _strength, float _fallOffStart,
		float _fallOffEnd, Vector3 _pos, Vector3 _color = Vector3(1.0f, 1.0f, 1.0f));

	// Spot Light 
	void SetSpotLight(CommonConstantBufferData* _psBufferData, UINT _index, float _strength, float _fallOffStart,
		float _fallOffEnd, Vector3 _dir, Vector3 _pos, float _spotPower, Vector3 _color = Vector3(1.0f, 1.0f, 1.0f));

	void SetLightOff(CommonConstantBufferData* _psBufferData, UINT _index);

	void SetLightStrength(CommonConstantBufferData* _psBufferData, UINT _index, float _strength);
	void SetLightType(CommonConstantBufferData* _psBufferData, UINT _index, LightEnum _lightType);
	void SetLightDir(CommonConstantBufferData* _psBufferData, UINT _index, Vector3 _dir);
	void SetLightFallOffStart(CommonConstantBufferData* _psBufferData, UINT _index, float _distance);
	void SetLightFallOffEnd(CommonConstantBufferData* _psBufferData, UINT _index, float _length);
	void SetLightPos(CommonConstantBufferData* _psBufferData, UINT _index, Vector3 _pos);
	void SetLightSpotPower(CommonConstantBufferData* _psBufferData, UINT _index, float _power);
	void SetLightColor(CommonConstantBufferData* _psBufferData, UINT _index, Vector3 _rgb);

	void CopyLight(CommonConstantBufferData* _psBufferData, UINT _copy, UINT _origin);

	Matrix CreateShadowViewMatrix(const Light& light);
	Matrix CreateShadowProjectionMatrix(const Light& light, float nearPlane, float farPlane, float fieldOfView, float aspectRatio);
};

template<typename ...Args>
void DearsGraphicsEngine::UIDrawTextWithNum(Vector2 _posXY, const std::u8string _formatText, Vector4 _rgba, Args&& ...args)
{
	m_pUiRenderer->DrawTextWithNum(_posXY, _formatText, _rgba, std::forward<Args>(args)...);
}

