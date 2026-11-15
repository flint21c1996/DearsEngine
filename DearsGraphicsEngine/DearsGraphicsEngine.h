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
#include "IDearsRenderer.h"
#include "LightHelper.h"
#include "ModelBuffer.h"
#include "Renderer.h"
#include "RenderContext.h"
#include "RenderPipeline.h"
#include "UiRenderer.h"
#include "EditorViewportState.h"
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;
using DirectX::SimpleMath::Quaternion;

class DebugRenderer;
class DeferredLightingRenderer;
class GraphicsAssetManager;
class GBuffer;
class GBufferDebugPanel;
class GBufferDebugRenderer;
class ShadowMapDebugRenderer;
class MeshRenderer;
class ParticleRenderer;
class PostProcessRenderer;
class RenderTargetPool;
struct RenderContext;
class UiRenderer;

// IDearsRenderer의 DX11 구현이다.
// 프레임/패스/그리기 계약은 IDearsRenderer에 있고, 이 클래스에만 있는 public 함수는
// 아직 DX11 타입을 노출하는 리소스/상수 버퍼/UI 계열이다.
class DearsGraphicsEngine : public IDearsRenderer
{
public:
	DearsGraphicsEngine(HWND _hWnd, int screenWidth, int screenHeight);
	~DearsGraphicsEngine() override;

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
	// G-Buffer를 읽고 Fullscreen Triangle으로 최종 PBR 조명을 계산한다.
	std::unique_ptr<DeferredLightingRenderer> m_pDeferredLightingRenderer;

	// 디퍼드 Geometry/Lighting 패스가 공유하는 화면 공간 표면 데이터이다.
	// DX11 리소스의 생성과 바인딩은 GBuffer 내부에 숨겨 GameEngine이 알지 못하게 한다.
	std::unique_ptr<GBuffer> m_pGBuffer;
	std::unique_ptr<GBufferDebugPanel> m_pGBufferDebugPanel;
	std::unique_ptr<GBufferDebugRenderer> m_pGBufferDebugRenderer;
	// 실제 Shadow Map을 선형 깊이 이미지로 변환한 에디터 전용 미리보기를 소유한다.
	std::unique_ptr<ShadowMapDebugRenderer> m_pShadowMapDebugRenderer;

	// 현재 적용된 렌더 패스 종류이다.
	// 지금은 디버깅용 상태에 가깝지만, 나중에 RHI를 붙이면
	// BeginRenderPass/EndRenderPass, resource transition, profiler marker가
	// 이 값을 기준으로 동작할 수 있다.
	RenderPassType m_currentRenderPassType = RenderPassType::Unknown;

	// 게임 엔진이 구성하는 프레임의 패스 목록이다.
	// 그래픽스 엔진은 목록의 내용과 순서를 정하지 않고 RenderFrame()에서 실행만 한다.
	RenderPipeline m_renderPipeline;

	// 게임 엔진이 CreateRenderTarget()으로 만든 오프스크린 타깃의 DX11 리소스를 소유한다.
	std::unique_ptr<RenderTargetPool> m_pRenderTargetPool;
	bool m_viewportSurfaceReady = false;
	bool EnsureViewportSurface(int width, int height);

	// RenderPassIO를 적용하면서 추적하는 바인딩 상태이다.
	// 같은 리소스를 출력과 입력으로 동시에 묶지 않도록, 다음 패스가 시작될 때 무엇을 풀어야 하는지 기억한다.
	bool m_isGBufferBoundAsInput = false;
	std::vector<UINT> m_boundRenderTargetInputSlots;

	// RenderPassIO 한 개를 실제 DX11 바인딩(OMSetRenderTargets, PSSetShaderResources 등)으로 옮긴다.
	void ApplyRenderPassIO(const RenderPassIO& requestedIO);

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
	void BeginEditorFrame();
	bool ResizeClient(int width, int height);
	const EditorViewportState& GetEditorViewport() const { return m_pUiRenderer->GetViewportState(); }
	bool ShowViewportOverlay() const { return m_pUiRenderer->ShowViewportOverlay(); }
	
	void EndRender();

	void Finalize();

	/// IDearsRenderer ---------------------------------------------------------------------------------
	// 게임 엔진과 렌더 패스는 가능하면 아래 함수들만 사용한다.
	// Rend_XXX / RenderXXXDepthMap 같은 종류별 함수는 기존 호출부 호환을 위해 남겨 두었다.
	RenderPipeline& GetRenderPipeline() override { return m_renderPipeline; }
	void RenderFrame() override;
	RenderTargetHandle CreateRenderTarget(const RenderTargetDesc& desc) override;
	RenderTargetHandle FindRenderTarget(const std::string& name) const override;
	void DrawMesh(ModelBuffer* modelBuffer, MeshDrawMode mode) override;
	bool DrawMeshWithShaders(
		ModelBuffer* modelBuffer,
		const std::string& vertexShader,
		const std::string& pixelShader) override;
	void DrawShadowCaster(ModelBuffer* modelBuffer, ShadowCasterMode mode) override;
	void DrawDeferredLighting() override;
	void DrawParticles() override;
	void DrawPostProcess() override;
	void DrawLightGizmo(const Light& light, bool drawShadowFrustum, float shadowAspect) override;

	void RendParticle();
	void AddParticle(unsigned int particleNum, CSParticleData& particleData);

	void RendPostProcessing();
	// ApplyRenderContext(Lighting)으로 G-Buffer가 SRV에 연결된 뒤 실제 조명 Draw를 실행한다.
	void RenderDeferredLighting();

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
	void ApplyRenderContext(const RenderContext& renderContext) override;
	void BeginShadowFrame() override;
	void BeginShadowProduction() override;
	void EndShadowProduction(bool success) override;
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
	void Rend_DeferredGeometry(ModelBuffer* _modelBuffer);
	void Rend_ThinFilm(ModelBuffer* _modelBuffer);

	void Rend_EquipmentModel(ModelBuffer* _modelBuffer);
	void SetOpacityFactor(float blendFactor[4]);							
	void Rend_OpacitiyModel(ModelBuffer* _modelBuffer);						

	void Rend_EdgeModel(ModelBuffer* _modelBuffer);							
	void Rend_Water(ModelBuffer* _modelBuffer);


	void RenderDepthMap(ModelBuffer* _modelbuffer);
	void RenderPbrDepthMap(ModelBuffer* _modelbuffer);
	void RenderAniDepthMap(ModelBuffer* _modelbuffer);
	void RenderEquipDepthMap(ModelBuffer* _modelbuffer);
	// GameEngine은 라이트/면 인덱스만 전달하고 DX11 Array DSV 선택은 그래픽스 계층에 맡긴다.
	// lightIndex는 CommonConstantBuffer의 lights[] 인덱스와 같고,
	// Point의 faceIndex 순서는 +X, -X, +Y, -Y, +Z, -Z이다.
	void BeginTwoDimensionalShadowPass(UINT lightIndex) override;
	void BeginPointShadowFace(UINT lightIndex, UINT faceIndex) override;
	void EndShadowPass() override;
	void RenderShadowMapDebugPreview(UINT lightIndex, float nearPlane, float farPlane, bool isPerspective);
	void RenderPointShadowMapDebugPreview(UINT lightIndex, float nearPlane, float farPlane);

	void Rend_InstancedModels(ModelBuffer* _modelbuffers);
	void Rend_BillBoard(ModelBuffer* _modelbuffers);

	void Rend_DebugBox(Vector3 _size, Vector3 _rotation, Vector3 _transpose);
	void Rend_DebugBox(Matrix _size, Matrix _rotation, Matrix _transpose);
	void Rend_DebugBox(Matrix _size, Matrix _rotation, Matrix _transpose, Matrix _tempMatrix);
	void Rend_DebugBox(AABB& _AABB, Matrix Scale, Matrix _rotation, Matrix _tempMatrix = Matrix());

	void Rend_DebugSphere(Vector3 _size, Vector3 _rotation, Vector3 _transpose);
	void Rend_DebugCapsule(Vector3 _size, Vector3 _rotation, Vector3 _transpose);
	void Rend_DebugLightGizmo(
		const Light& light,
		bool drawShadowFrustum = false,
		float shadowAspect = 1.0f);

	void Rend_CubeMap(ModelBuffer* _modelBuffer);
	//占쏙옙占쏙옙占쏙옙占쏙옙 占십울옙占쏙옙 카占쌨띰옙 占쏙옙占쏙옙占싼댐옙.
	void SetCamera(Camera* _pTargetCamera);

	VSConstantBufferData m_VSConstantBufferData;
	PSConstantBufferData m_PSConstantBufferData;
	VSBoneConstantBufferData m_VSBoneConstantBufferData;

	/// UI -------------------------------------------------------------------------------------------------

	void AddEditorPanel(IEditorPanel* panel);
	// 에디터 창의 기본 도킹 자리를 정한다. windowName은 그 창이 ImGui::Begin에 넘기는 이름이다.
	void SetEditorDockSlot(const std::string& windowName, EditorDockSlot slot) { m_pUiRenderer->SetDockSlot(windowName, slot); }
	// 메인 메뉴 바의 Window 메뉴 앞에 File/Edit/Play 같은 항목을 그릴 콜백을 등록한다.
	void SetEditorMenuBarCallback(std::function<void()> callback) { m_pUiRenderer->SetMenuBarCallback(std::move(callback)); }
	// 월드(Viewport) 이미지에 끌어다 놓기: 받을 페이로드 종류를 등록하고, 놓인 것을 프레임마다 꺼내 간다.
	// x/y는 Viewport 이미지 안의 픽셀 위치라서 피킹에 그대로 쓸 수 있다.
	void SetViewportDropTypes(std::vector<std::string> types) { m_pUiRenderer->SetViewportDropTypes(std::move(types)); }
	bool ConsumeViewportDrop(std::string& type, std::string& data, float& x, float& y) { return m_pUiRenderer->ConsumeViewportDrop(type, data, x, y); }
	void SetRenderViewportWidth(int viewportWidth);
	// 에디터 계층이 DX11 SRV 타입을 직접 알지 않도록 Shadow Map 이미지를 그래픽스 UI 경계에서 그린다.
	bool UIDrawShadowMapDebugPreview(Vector2 size) const;
	// 오프스크린 렌더 타깃의 내용을 현재 ImGui 창에 이미지로 그린다. (미니맵, 타깃 디버그 확인용)
	// 타깃에 쓰는 패스가 끝난 뒤의 내용이 프레임 끝 ImGui 렌더에서 표시된다.
	bool UIDrawRenderTarget(RenderTargetHandle target, Vector2 size) const;

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

