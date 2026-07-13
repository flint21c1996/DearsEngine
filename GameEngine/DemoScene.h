#pragma once

#include <array>
#include <memory>
#include <vector>

#include "IScene.h"
#include "ObjectInspectorPanel.h"
#include "PickingManager.h"
#include "PickingPanel.h"
#include "RenderObject.h"
#include "SceneHierarchyPanel.h"

class DearsGraphicsEngine;
class InputManager;
class EasingFunc;
class Camera;

class DemoScene final : public IScene
{
public:
	explicit DemoScene(DearsGraphicsEngine* graphicsEngine);

	// Setup and update entry points used by GameEngine.
	void Initialize() override;
	void RegisterEditorPanels(int screenWidth) override;
	void Update(float deltaTime) override;

	// These handlers keep sample-scene controls out of GameEngine.
	void HandleDemoInput(InputManager& inputManager) override;
	void HandlePickingInput(InputManager& inputManager, Camera* camera) override;
	void HandlePresentationInput(InputManager& inputManager, EasingFunc& easingFunc, float deltaTime) override;
	void HandleRenderInput(InputManager& inputManager) override;

	// GameEngine은 이제 Character/Terrain 같은 DemoScene 전용 이름을 묻지 않고,
	// 각 렌더 패스에 필요한 목록만 받아간다.
	const std::vector<SceneRenderItem>& GetShadowRenderItems() const override;
	const std::vector<SceneRenderItem>& GetMainRenderItems() const override;
	RenderObject* GetSelectedObject() const override;
	void CollectLights(CommonConstantBufferData& buffer) const override;
	const Vector2& GetPrimaryUiPoint() const override;
	const Vector2& GetSecondaryUiPoint() const override;

private:
	void SpawnDemoParticle();
	RenderObject* GetObject(size_t index) const;
	void CreateObjectFromDesc(const SceneObjectCreateDesc& desc);
	void CreateObjectFromDesc(const SceneObjectCreateDesc& desc, Matrix position, Matrix rotation, Matrix scale, bool saveScene);
	void CreateSceneObjects();
	void CreateRenderItems();
	void SaveSceneToFile() const;
	void LoadSceneFromFile();
	void HandleSceneObjectEdited();
	void DeleteSelectedObject();

	RenderObject* GetCharacter() const;
	RenderObject* GetWeapon() const;
	RenderObject* GetBillboard() const;
	RenderObject* GetPbrSphere() const;
	RenderObject* GetFloor() const;
	RenderObject* GetWater() const;
	RenderObject* GetTerrain() const;
	RenderObject* GetCubeMap() const;

private:
	DearsGraphicsEngine* m_pGraphicsEngine = nullptr;
	std::vector<std::unique_ptr<RenderObject>> m_objects;
	std::vector<std::string> m_objectNames;
	std::vector<SceneObjectCreateDesc> m_objectCreateDescs;
	std::vector<SceneRenderItem> m_shadowRenderItems;
	std::vector<SceneRenderItem> m_mainRenderItems;
	int m_selectedObjectIndex = -1;
	int m_renderViewportWidth = 0;
	PickingManager m_pickingManager;
	std::unique_ptr<SceneHierarchyPanel> m_pScenePanel;
	std::unique_ptr<PickingPanel> m_pPickingPanel;
	std::unique_ptr<ObjectInspectorPanel> m_pInspectorPanel;

	// These values only exist to drive the sample scene presentation and tests,
	// so they stay with DemoScene instead of the engine core.
	Vector2 m_uiPoint = { 1720.f, 400.f };
	Vector2 m_uiPoint2 = { 1720.f, 700.f };
	Vector3 m_particleSpawnPos = { 0.f, 0.f, 0.f };
	float m_uiTweenTime = 0.f;
	float m_pbrSphereRotationY = 0.f;
	float m_pbrHeightScale = 0.f;
	std::array<float, 4> m_opacityValue = { 1.f, 1.f, 1.f, 1.f };
};
