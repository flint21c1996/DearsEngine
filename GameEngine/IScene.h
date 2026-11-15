#pragma once

#include <directxtk/SimpleMath.h>
#include <vector>

class InputManager;
class EasingFunc;
class RenderObject;
class Camera;
struct CommonConstantBufferData;

using DirectX::SimpleMath::Vector2;

enum class SceneRenderType
{
	StaticMesh,
	SkinnedMesh,
	EquipmentMesh,
	CubeMap,
	Billboard,
	PbrMesh,
	DirectionalLight,
	PointLight,
	SpotLight
};

// 같은 PBR 재질이라도 어떤 렌더링 경로로 제출할지 선택한다.
// Forward는 오브젝트를 그릴 때 즉시 조명을 계산하고,
// Deferred는 Geometry Pass에 표면 정보를 기록한 뒤 Lighting Pass에서 조명을 계산한다.
enum class SceneRenderPath
{
	Forward,
	Deferred,
};

// 같은 PBR 메시라도 표면의 최종 색상을 계산하는 방법은 재질마다 다를 수 있다.
// Render Path가 "언제 그릴지"를 정한다면, Shading Model은 "어떤 조명 공식으로 그릴지"를 정한다.
// Thin Film은 현재 전용 Pixel Shader가 최종 색상을 계산하므로 Forward Path에서 처리한다.
enum class MaterialShadingModel
{
	DefaultLit,
	ThinFilm,
};

struct SceneRenderItem
{
	RenderObject* object = nullptr;
	SceneRenderType renderType = SceneRenderType::StaticMesh;
	SceneRenderPath renderPath = SceneRenderPath::Forward;
};

// GameEngine이 씬에 대해 알아야 하는 최소 계약이다.
// 구체 씬은 자신이 가진 오브젝트, 에디터 패널, 테스트 입력을 직접 관리하고,
// GameEngine에는 렌더링에 필요한 목록만 넘겨준다.
class IScene
{
public:
	virtual ~IScene() = default;

	virtual void Initialize() = 0;
	virtual void RegisterEditorPanels(int screenWidth) = 0;
	virtual void Update(float deltaTime) = 0;

	virtual void HandleDemoInput(InputManager& inputManager) = 0;
	virtual void HandlePickingInput(InputManager& inputManager, Camera* camera) = 0;
	virtual void HandlePresentationInput(InputManager& inputManager, EasingFunc& easingFunc, float deltaTime) = 0;
	virtual void HandleRenderInput(InputManager& inputManager) = 0;

	// 에디터 기능: 매 프레임 호출되어 단축키(Ctrl+Z/Y/S/D, W/E/R), 이동/회전/크기 기즈모,
	// 뷰포트로 끌어다 놓은 에셋, 실행 취소 기록을 처리한다.
	// Viewport 창이 열려 있는 동안(BeginEditorFrame 이후, Render 이전) 호출해야 기즈모가 그 창에 그려진다.
	virtual void UpdateEditor(InputManager& inputManager, Camera* camera) = 0;
	// 메인 메뉴 바에 File/Edit 메뉴와 Play/Stop 버튼을 그린다.
	virtual void DrawEditorMenuBar() = 0;
	// Play 중이면 true. Play 중에 바뀐 씬 상태는 Stop할 때 버려지고 편집 원본으로 돌아간다.
	virtual bool IsPlaying() const = 0;

	virtual const std::vector<SceneRenderItem>& GetShadowRenderItems() const = 0;
	virtual const std::vector<SceneRenderItem>& GetMainRenderItems() const = 0;
	virtual RenderObject* GetSelectedObject() const = 0;
	// Scene에 배치된 라이트를 GPU 공통 버퍼 형식으로 수집한다.
	// 반환값은 현재 선택된 라이트의 GPU 배열 인덱스이며, 선택된 라이트가 없으면 -1이다.
	// 그림자 생성 여부와 에디터의 미리보기 선택을 분리하기 위해 인덱스를 별도로 돌려준다.
	virtual int CollectLights(CommonConstantBufferData& buffer) const = 0;

	virtual const Vector2& GetPrimaryUiPoint() const = 0;
	virtual const Vector2& GetSecondaryUiPoint() const = 0;
};
