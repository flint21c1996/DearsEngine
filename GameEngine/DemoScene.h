#pragma once

#include <array>
#include <memory>
#include <vector>

#include "IScene.h"
#include "ObjectInspectorPanel.h"
#include "RenderObject.h"
#include "SceneHierarchyPanel.h"

class DearsGraphicsEngine;
class InputManager;
class EasingFunc;

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
	void HandlePresentationInput(InputManager& inputManager, EasingFunc& easingFunc, float deltaTime) override;
	void HandleRenderInput(InputManager& inputManager) override;

	// GameEngine still renders objects directly, so it queries scene roles through
	// these accessors instead of touching the raw object container.
	RenderObject* GetCharacter() const override;
	RenderObject* GetWeapon() const override;
	RenderObject* GetBillboard() const override;
	RenderObject* GetPbrSphere() const override;
	RenderObject* GetFloor() const override;
	RenderObject* GetWater() const override;
	RenderObject* GetTerrain() const override;
	RenderObject* GetCubeMap() const override;
	const Vector2& GetPrimaryUiPoint() const override;
	const Vector2& GetSecondaryUiPoint() const override;

private:
	void SpawnDemoParticle();
	RenderObject* GetObject(size_t index) const;
	void CreateSceneObjects();

private:
	DearsGraphicsEngine* m_pGraphicsEngine = nullptr;
	std::vector<std::unique_ptr<RenderObject>> m_objects;
	int m_selectedObjectIndex = -1;
	std::unique_ptr<SceneHierarchyPanel> m_pScenePanel;
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
