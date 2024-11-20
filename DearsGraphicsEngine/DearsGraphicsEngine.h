#pragma once
#include <windows.h>
#include <wrl.h>
#include <directxtk/SimpleMath.h>
#include <map>
#include <string>

#include "Renderer.h"
#include "GraphicsResourceManager.h"
#include "DearsImGui.h"
#include "AnimationHelper.h"
#include "LightHelper.h"
#include "ModelBuffer.h"
#include "ParticleManager.h"
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;
using DirectX::SimpleMath::Quaternion;

///제일 최상위에서 그래픽스의 모든것을 담당하는 클래스 
class DearsGraphicsEngine
{
public:
	DearsGraphicsEngine(HWND _hWnd, int screenWidth, int screenHeight);
	~DearsGraphicsEngine();

	/// Window관련 함수
private:
	HWND m_hWnd;
	int m_screenWidth;
	int m_screenHeight;

	//디바이스와 디바이스 컨텍스트는 중앙에서 관리하는게 좋다.
	ComPtr<ID3D11Device> m_pDevice;										//디바이스
	ComPtr<ID3D11DeviceContext> m_pDeviceContext;						//디바이스 컨텍스트

public:
	///렌더러를 가지고 있는 자료구조. 
	Renderer* mpRenderer;

	///UI 생성을 위한 2D 클래스
	DearsImGui* m_pDearsImGui;

	///그래픽스 리소스 매니저
	GraphicsResourceManager* m_pResourceManager;

	AnimationHelper* mpAnimationHelper;
	LightHelper* mpLightHelper;

	///카메라, 이후 게임엔진과 그래픽스 엔진 모두 아는 라이브러리로 빼서 수정에 용이함을 둔다.
	Camera* m_pTargetCamera;

	///파티클 매니저
	ParticleManager* m_pParticleManager;
public:
	//DX를 초기화한다.
	void Initialize();

	//업데이트.
	void Update();
	//렌더링을 준비한다.
	void BeginRender();
	//렌더링을 끝낸다.
	void EndRender();

	//엔진을 종료한다.
	void Finalize();

	void RendParticle();

	void RendPostProcessing();

	// 화면의 가로길이를 가져온다.
	int GetScreenWidth() const;
	// 화면의 세로길이를 가져온다.
	int GetScreenHeight() const;
	//화면비를 가져온다. -> Renderer로 빼는게 좋을 듯 하다.
	float GetAspectRatio();

	//모델을 추가, 모델의 VB, IB, p, Model*, boneNameVec
	void AddModel(std::string _basePath, std::string _fileName);
	//애니메이션을 추가
	void AddAnimation(std::string _basePath, std::string _fileName);
	//3D텍스쳐를 추가
	void Add3DTexture(std::string _basePath, std::string _fileName);
	//2D텍스쳐를 추가
	void Add2DTexture(std::string _basePath, std::string _fileName);
	void AddDDSTexture(std::string _basePath, std::string _fileName);

	//버텍스 버퍼를 가지고 온다.
	ComPtr<ID3D11Buffer> Get_VertexBuffer(std::string _modelName);
	//인덱스버퍼를 가지고 온다.
	ComPtr<ID3D11Buffer> Get_IndexBuffer(std::string _modelName);
	//인덱스 수를 가지고 온다.
	unsigned int Get_NumIndex(std::string _modelName);
	//모델의 정보를 가지고 온다. 메쉬의 이름이 아닌 파일명을 넣는다.
	Model* Get_ModelInfo(std::string _modelName);
	//애니메이션 정보를 가지고 온다.
	Animation* Get_Animation(std::string _animeName);
	//텍스쳐를 가지고 온다.
	ComPtr<ID3D11ShaderResourceView> Get_Textures(std::string _textureName);
	//폰트를 가지고 온다.
	ImFont* Get_Font(std::string _fontName);
	//
	int Get_TargetModelBoneIndex(std::string _modelName, std::string _boneName);

	// 무기의 렌더시 쓰는 함수 -> 본의 offsetMatrix가 곱해진 Matrix를 받아온다.
	Matrix GetTargetBoneMatrix(std::string _targetModel, std::string _targetBoneName);
	// 실제 본의 위치를 받아오는 함수 -> 본의 offsetMatrix가 곱해지지 않은 본의 위치값만 받아온다, 기본 스케일 = 1
	Matrix GetTargetBoneAboveMatrix(std::string _targetModel, std::string _targetBoneName, float _scale = 1.f);
	Matrix GetTargetBoneAboveMatrix(std::string _targetModel, int _index, float _scale = 1.f);

	AABB Get_AABB(std::string __targetModel);

	//리소스 컨테이너안의 특정 버텍스 버퍼를 지운다.
	void Erase_VertexBuffer(std::string _modelName);
	//리소스 컨테이너안의 특정 인덱스 버퍼를 지운다.
	void Erase_IndexBuffer(std::string _modelName);
	//리소스 컨테이너안의 특정 인덱스Num을 지운다.
	void Erase_NumIndex(std::string _modelName);
	//리소스 컨테이너안의 특정 모델정보를 지운다.
	void Erase_ModelInfo(std::string _modelName);
	//리소스 컨테이너안의 특정 애니메이션 정보를 지운다.
	void Erase_Animation(std::string _animName);
	//리소스 컨테이너 안의 특정 텍스쳐를 지운다.
	void Erase_Textures(std::string _textureName);
	//리소스 컨테이너 안의 특정 폰트를 지운다.
	void Erase_Font(std::string _fontName);

	//컨스턴트 버퍼를 생성한다. 아오 나중에 이 밑에 함수들 템플릿으로 바꾸자 못봐주겠다.. 
	ComPtr<ID3D11Buffer> CreateConstantBuffer(VSConstantBufferData& _VsConstantBufferData);
	ComPtr<ID3D11Buffer> CreateConstantBuffer(VSBoneConstantBufferData& _VsBoneConstantBufferData);
	ComPtr<ID3D11Buffer> CreateConstantBuffer(VSTargetBoneConstantBufferData& _PsConstantBufferData);
	ComPtr<ID3D11Buffer> CreateConstantBuffer(PSConstantBufferData& _PsConstantBufferData);
	ComPtr<ID3D11Buffer> CreateConstantBuffer(PSEdgeConstantBufferData& _pPSEdgeConstantBuffer);
	ComPtr<ID3D11Buffer> CreateConstantBuffer(VSShadowConstantBufferData& _VsShadowConstantBufferData);
	ComPtr<ID3D11Buffer> CreateConstantBuffer(PsShadowConstantBufferData& _VsShadowConstantBufferData);
	ComPtr<ID3D11Buffer> CreateConstantBuffer(CommonConstantBufferData& _CommonConstantBufferData);
	ComPtr<ID3D11Buffer> CreateConstantBuffer(VSInstantConstantBufferData& _VSInstantConstantBufferData);

	//Strucured Buffer을 생성한다. 
	ComPtr<ID3D11Buffer> CreateStructuredBuffer(CSParticleData& _TestCSParticleData, unsigned int _count);


	//갑자기 급 귀찮아져서 그냥 템플릿으로 짠다.
	template <typename T>
	ComPtr<ID3D11Buffer> CreateConstantBuffer(T& _bufferData)
	{
		return RendererHelper::CreateConstantBuffer(m_pDevice, _bufferData);
	}

	// 공통 컨스턴트 버퍼를 업데이트한다.
	void UpdateCommonConstantBuffer(CommonConstantBufferData& _CommonBufferData);

	//컨스턴트버퍼를 업데이트한다.
	void UpdateConstantBuffer(ModelBuffer* _pModelBuffer, VSConstantBufferData& _VsConstantBufferData);
	//본 컨스턴트 버퍼를 업데이트한다.
	void UpdateBoneConstantBuffer(ModelBuffer* _pModelBuffer, VSBoneConstantBufferData& _VsBoneConstantBufferData);
	
	//두 애니메이션의 보간을 하며 본 컨스턴트 버퍼를 업데이트한다.-> Trangision 사용 시
	bool UpdateTransitionBoneConstantBuffer(ModelBuffer* _pModelBuffer, VSBoneConstantBufferData& _VsBoneConstantBufferData);
	//타겟 본 컨스턴트 버퍼를 업데이트한다.
	void UpdateTargetBoneConstantBuffer(ModelBuffer* _pModelBuffer, VSTargetBoneConstantBufferData& _VsTargetBoneConstantBufferData);


	//컨스턴트버퍼를 업데이트한다.
	void UpdateConstantBuffer(ModelBuffer* _pModelBuffer, PSConstantBufferData& _PsConstantBufferData);

	//외곽선을 그리는데 관련된 VSconstantBuffer을 업데이트한다.
	void UpdateVSEdgeConstantBuffer(ModelBuffer* _pModelBuffer, VSEdgeConstantBufferData& _pPSEdgeConstantBuffer);

	//외곽선을 그리는데 관련된 PSconstantBuffer을 업데이트한다.
	void UpdatePSEdgeConstantBuffer(ModelBuffer* _pModelBuffer, PSEdgeConstantBufferData& _pPSEdgeConstantBuffer);

	//물을 표현할때 필요한 VSWaterConstnatBuffer을 업데이트한다.
	void UpdateVSWaterConstantBuffer(ModelBuffer* _pModelBuffer, VSWaterConstantBufferData& _pPSEdgeConstantBuffer);

	void UpdateShadowConstantBuffer(ModelBuffer* _pModelBuffer, VSShadowConstantBufferData& _VsShadowConstantBufferData);
	void UpdateShadowConstantBuffer(ModelBuffer* _pModelBuffer, PsShadowConstantBufferData& _VsShadowConstantBufferData);

	//큐브맵을 세팅한다.
	void Set_CubeMap(std::string diffuseTextureName, std::string specularTextureName);
	
	//PipelineState를 세팅한다.
	void SetPipelineState(PipelineStateObject& _pso);
	void Rend_AnimateModel(ModelBuffer* _modelBuffer);
	void Rend_Model(ModelBuffer* _modelBuffer);
	void Rend_EquipmentModel(ModelBuffer* _modelBuffer);
	void SetOpacityFactor(float blendFactor[4]);							//투명도를 설정한다.
	void Rend_OpacitiyModel(ModelBuffer* _modelBuffer);						//투명한 정적 오브젝트를 렌더한다.

	void Rend_EdgeModel(ModelBuffer* _modelBuffer);							//외곽선이 잇는 오브젝트를 렌더한다.
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
	//렌더링에 필요한 카메라를 세팅한다.
	void SetCamera(Camera* _pTargetCamera);

	ModelBuffer* Debug_ModelBuffer;		///*임시*
	ModelBuffer* PostProcessingBuffer;


	VSConstantBufferData m_VSConstantBufferData;
	PSConstantBufferData m_PSConstantBufferData;
	VSBoneConstantBufferData m_VSBoneConstantBufferData;

	/// UI 작업 -------------------------------------------------------------------------------------------------

	// [[[초기화에 설정]]]
	// [기본 경로, 파일 이름, 폰트 사이즈, 한글인가요?] 폰트를 추가 (파일이름은 폰트 ID로 자동 설정)
	void AddFont(std::string _basePath, std::string _fileName, float _size, bool _isKorean);

	// 폰트를 모두 추가했으면 셋팅 완료를 호출함
	void FontSetFinish();

	// [[[렌더러에 설정]]]
	// 프레임 설정
	void UIBegineRender();

	// ----- 이미지 및 폰트 -----------------------------------

	// [시작 위치xy, 가로세로] 캔버스 설정 
	void UICanvasSet(Vector2 _posXY, Vector2 _sizeWH = Vector2());

	// 이미지를 그리기 시작할 것임을 선언
	void UIDrawImageStart();

	// [시작 위치xy, 가로세로, UI이름, 텍스처 이름] 이미지를 그리기
	void UIDrawImage(Vector2 _posXY, Vector2 _sizeWH, std::string _textureName, Vector4 _rgba = Vector4(1.0f, 1.0f, 1.0f, 1.0f));

	// 더 이상 추가되는 이미지가 없음을 선언
	void UIDrawImageFin();

	// [font 리소스 이름] 설정한 폰트를 사용할 것임을 선언
	void UIStartFontID(std::string _fontName);

	// [위치xy, 띄울 문장, RGBA값] 텍스트를 그리는 함수	//C++
	void UIDrawText(Vector2 _pos, std::u8string _text, Vector4 _rgba = Vector4(0.0f, 0.0f, 0.0f, 1.0f));

	// [위치xy, 띄울 문장, RGBA값, 인수들...] 변하는 숫자를 포함한 텍스트를 그리는 함수	//C++
	template<typename ...Args>
	void UIDrawTextWithNum(Vector2 _posXY, const std::u8string _formatText, Vector4 _rgba = Vector4(0.0f, 0.0f, 0.0f, 1.0f), Args&& ...args);

	// 이전까지 설정한 폰트를 더 사용하지 않을 것임을 선언
	void UIFinFontID();

	// ----- 기본 도형 그리기 -----------------------------------

	// 현재 윈도우 가져오기, 기본 그리기 함수들의 바로 위에 호출
	void SetUICurrentWindow();

	// [위치xy, 가로세로, RGBA] 사각형의 테두리만 그리는 함수 
	void UIDrawRect(Vector2 _posXY, Vector2 _sizeWH, Vector4 _rgba, float _rounding = 0.0f, float _thickness = 1.0f);

	// [시작 위치xy, 가로세로, RGBA, 모서리 둥근 정도] 채워진 사각형을 그리는 함수
	void UIDrawRectFilled(Vector2 _posXY, Vector2 _sizeWH, Vector4 _rgba, float _rounding = 0.0f);							

	// [시작 위치xy, 가로세로, RGBA, 모서리 둥근 정도, 두께] 사각형과 테두리를 그리는 함수
	void UIDrawRectwithBorder(Vector2 _posXY, Vector2 _sizeWH, Vector4 _rgba, float _rounding = 0.0f, float _thickness = 1.0f);
	
	// [point 시계방향으로 4개, RGBA, 두께] 자유형 사각형의 테두리만 그리는 함수
	void UIFreeRect(Vector2 _posXY1, Vector2 _posXY2, Vector2 _posXY3, Vector2 _posXY4, Vector4 _rgba, float _thickness);
	
	// [point 시계방향으로 4개, RGBA] 채워진 자유형 사각형을 그리는 함수
	void UIFreeRectFilled(Vector2 _posXY1, Vector2 _posXY2, Vector2 _posXY3, Vector2 _posXY4, Vector4 _rgba);
	
	// [point 시계방향으로 4개, RGBA, 두께] 자유형 사각형과 테두리를 그리는 함수
	void UIFreeRectwithBorder(Vector2 _posXY1, Vector2 _posXY2, Vector2 _posXY3, Vector2 _posXY4, Vector4 _rgba, float _thickness, Vector4 _borderRgba);

	// [시작점 위치xy, 끝점 위치xy, RGBA] 선을 그리는 함수 
	void UIDrawLine(Vector2 _sPosXY, Vector2 _ePosXY, Vector4 _rgba);

	// [시작 위치xy, 반지름, RGBA] 원을 그리는 함수 
	void UIDrawCir(Vector2 _posXY, float _radius, Vector4 _rgba);
	// -----------------------------------------------------

	void RenderImGui();

	void EndRenderImGui();

	/// 라이트 작업 -------------------------------------------------------------------------------------------------
	// [PS버퍼구조, 만드려는 빛의 개수] 원하는 Light의 개수를 만들고, PSConstantBufferData에 전해준다.
	void LightInitialize(CommonConstantBufferData* _psBufferData, UINT _num);

	// [PS버퍼구조, 만드려는 빛의 개수] PS 버퍼의 Max Light의 개수 조정
	void ChangeLightMaxNum(CommonConstantBufferData* _psBufferData, UINT _num);

	// [PS버퍼구조] Light Update, PS 버퍼 업데이트 전에 호출
	void LightUpdate(CommonConstantBufferData* _psBufferData);

	// [PS버퍼구조] 현재 컨스턴트 버퍼의 빛의 개수와 정보 출력
	void PrintLightInfo(CommonConstantBufferData* _psBufferData);

	// 라이트 내의 성분 전체 설정 [PS버퍼구조, 종류, 세기, 빛 강도 하락의 시작, 빛 강도 하락의 끝, 방향, 위치, spotPower]
	void SetLightSettingAll(CommonConstantBufferData* _psBufferData, UINT _index, LightEnum _lightType, float _strength, float _fallOffStart,
							float _fallOffEnd, Vector3 _dir, Vector3 _pos, float _spotPower, Vector3 _color = Vector3(1.0f, 1.0f, 1.0f));
	// Directional Light 설정
	void SetDirLight(CommonConstantBufferData* _psBufferData, UINT _index, float _strength, Vector3 _dir, Vector3 _color = Vector3(1.0f, 1.0f, 1.0f));

	// Point Light 설정
	void SetPointLight(CommonConstantBufferData* _psBufferData, UINT _index, float _strength, float _fallOffStart,
		float _fallOffEnd, Vector3 _pos, Vector3 _color = Vector3(1.0f, 1.0f, 1.0f));

	// Spot Light 설정
	void SetSpotLight(CommonConstantBufferData* _psBufferData, UINT _index, float _strength, float _fallOffStart,
		float _fallOffEnd, Vector3 _dir, Vector3 _pos, float _spotPower, Vector3 _color = Vector3(1.0f, 1.0f, 1.0f));

	// 현재 인덱스의 라이트를 NONE으로 바꿉니다. -> Light 연산 안함
	void SetLightOff(CommonConstantBufferData* _psBufferData, UINT _index);

	// [PS버퍼구조, index, 빛의 세기(0~1)] 빛의 세기를 설정, 0~1의 값만 넣는다.
	void SetLightStrength(CommonConstantBufferData* _psBufferData, UINT _index, float _strength);
	// [PS버퍼구조, index, 빛의 종류] 빛의 종류를 설정
	void SetLightType(CommonConstantBufferData* _psBufferData, UINT _index, LightEnum _lightType);
	// [PS버퍼구조, index, 빛의 방향] 빛의 방향 설정, _dir은 단위 벡터로 넣을 것.
	void SetLightDir(CommonConstantBufferData* _psBufferData, UINT _index, Vector3 _dir);
	// [PS버퍼구조, index, 빛의 강도가 줄어드는 지점의 시작 거리] 빛의 범위 설정 : 5이하로 설정시 오류날 수 있음
	void SetLightFallOffStart(CommonConstantBufferData* _psBufferData, UINT _index, float _distance);
	// [PS버퍼구조, index, 빛 범위의 끝] 빛의 범위 설정 : 5이하로 설정시 오류날 수 있음
	void SetLightFallOffEnd(CommonConstantBufferData* _psBufferData, UINT _index, float _length);
	// [PS버퍼구조, index, 빛의 위치] 빛의 위치를 설정
	void SetLightPos(CommonConstantBufferData* _psBufferData, UINT _index, Vector3 _pos);
	// [PS버퍼구조, index, 빛의 spotPower(0~100)] 빛의 spotPower를 설정 : 클수록 빛이 모아짐
	void SetLightSpotPower(CommonConstantBufferData* _psBufferData, UINT _index, float _power);
	// [PS버퍼구조, index, 빛의 방향] 빛의 색깔 설정
	void SetLightColor(CommonConstantBufferData* _psBufferData, UINT _index, Vector3 _rgb);

	// [PS버퍼구조, 복사할 빛, 복사될 인덱스] 특정 인덱스의 빛 복사하기
	void CopyLight(CommonConstantBufferData* _psBufferData, UINT _copy, UINT _origin);

	Matrix CreateShadowViewMatrix(const Light& light);
	Matrix CreateShadowProjectionMatrix(const Light& light, float nearPlane, float farPlane, float fieldOfView, float aspectRatio);
};

template<typename ...Args>
void DearsGraphicsEngine::UIDrawTextWithNum(Vector2 _posXY, const std::u8string _formatText, Vector4 _rgba, Args&& ...args)
{
	m_pDearsImGui->UIDrawTextWithNum(_posXY, m_pDearsImGui->ConvertUTF8String(_formatText), _rgba, std::forward<Args>(args)...);
}

