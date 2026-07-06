#pragma once

#include <directxtk/SimpleMath.h>
#include <vector>

class InputManager;
class EasingFunc;
class RenderObject;

using DirectX::SimpleMath::Vector2;

enum class SceneRenderType
{
	StaticMesh,
	SkinnedMesh,
	EquipmentMesh,
	CubeMap,
	Billboard,
	PbrMesh
};

struct SceneRenderItem
{
	RenderObject* object = nullptr;
	SceneRenderType renderType = SceneRenderType::StaticMesh;
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
	virtual void HandlePresentationInput(InputManager& inputManager, EasingFunc& easingFunc, float deltaTime) = 0;
	virtual void HandleRenderInput(InputManager& inputManager) = 0;

	virtual const std::vector<SceneRenderItem>& GetShadowRenderItems() const = 0;
	virtual const std::vector<SceneRenderItem>& GetMainRenderItems() const = 0;

	virtual const Vector2& GetPrimaryUiPoint() const = 0;
	virtual const Vector2& GetSecondaryUiPoint() const = 0;
};
