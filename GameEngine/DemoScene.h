#pragma once

#include <array>
#include <memory>
#include <vector>

#include "IScene.h"
#include "AStarDemoPanel.h"
#include "ContentBrowserPanel.h"
#include "SceneHistory.h"
#include "LightShadowMapPanel.h"
#include "ObjectInspectorPanel.h"
#include "PickingManager.h"
#include "PickingPanel.h"
#include "RenderObject.h"
#include "RenderPreset.h"
#include "RenderPresetPanel.h"
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

	// 에디터 기능 (구현은 DemoSceneEditor.cpp)
	void UpdateEditor(InputManager& inputManager, Camera* camera) override;
	void DrawEditorMenuBar() override;
	bool IsPlaying() const override { return m_isPlaying; }

	// GameEngine은 이제 Character/Terrain 같은 DemoScene 전용 이름을 묻지 않고,
	// 각 렌더 패스에 필요한 목록만 받아간다.
	const std::vector<SceneRenderItem>& GetShadowRenderItems() const override;
	const std::vector<SceneRenderItem>& GetMainRenderItems() const override;
	RenderObject* GetSelectedObject() const override;
	int CollectLights(CommonConstantBufferData& buffer) const override;
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
	bool LoadSceneFromFile(const std::string& scenePath);
	void HandleSceneObjectEdited();
	void DeleteSelectedObject();

	// 렌더 프리셋: 프리셋 에셋 목록을 Scene이 소유하고, 오브젝트는 이름으로 참조한다.
	// 프리셋의 내용(셰이더, 렌더 경로)을 오브젝트의 렌더 설정과 렌더 목록으로 풀어 낸다.
	void ApplyRenderPresetToObject(size_t objectIndex);
	// 드래그 앤 드롭이나 Inspector에서 오브젝트에 프리셋을 지정한다. 빈 이름은 프리셋 해제이다.
	void AssignRenderPreset(int objectIndex, const std::string& presetName);
	// 프리셋을 만들거나 고치거나 지웠을 때: 모든 오브젝트에 다시 적용하고 파일로 저장한다.
	void HandleRenderPresetLibraryEdited();
	void HandleRenderPresetRenamed(const std::string& oldName, const std::string& newName);

	// 씬 직렬화: 파일 저장, 실행 취소 스냅샷, Play 백업이 같은 텍스트 형식을 함께 쓴다.
	void WriteScene(std::ostream& stream) const;
	bool ReadScene(std::istream& stream);
	std::string SerializeScene() const;
	void ClearSceneObjects();
	bool RestoreScene(const std::string& serializedScene);

	// 실행 취소 / 다시 실행
	void Undo();
	void Redo();
	// 진행 중인 조작(드래그, 기즈모)이 끝났을 때만 기록을 남긴다. 드래그 한 번이 한 단계가 되게 하기 위해서다.
	void CommitHistoryIfIdle();

	// 씬 파일
	void NewScene(const std::string& sceneName);
	void OpenScene(const std::string& scenePath);
	void SaveSceneAs(const std::string& sceneName);
	std::string GetSceneDisplayName() const;

	// Play / Stop
	void SetPlaying(bool playing);

	// 뷰포트 조작
	void DrawTransformGizmo(Camera* camera);
	void HandleViewportDrop(Camera* camera);
	void DrawScenePopups();
	void DuplicateSelectedObject();
	// 콘텐츠 브라우저에서 끌어 온 메시로 오브젝트를 만든다.
	void SpawnMeshAsset(const std::string& vertexBufferName, const Vector3& position);
	void AssignDiffuseTexture(int objectIndex, const std::string& textureName);

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
	std::unique_ptr<LightShadowMapPanel> m_pLightShadowMapPanel;
	std::unique_ptr<AStarDemoPanel> m_pAStarDemoPanel;
	RenderPresetLibrary m_renderPresets;
	std::unique_ptr<RenderPresetPanel> m_pRenderPresetPanel;
	std::unique_ptr<ContentBrowserPanel> m_pContentBrowserPanel;

	// 현재 열려 있는 씬 파일의 경로이다. 편집할 때마다 이 파일에 저장된다.
	std::string m_scenePath;
	std::string m_sceneFileError;

	// 실행 취소 기록. m_historyDirty는 "저장은 했지만 아직 기록에 남기지 않은 변경이 있다"는 표시이고,
	// SaveSceneToFile() const 안에서 세우기 때문에 mutable이다.
	SceneHistory m_history;
	mutable bool m_historyDirty = false;

	// Play를 누른 순간의 씬이다. Stop하면 이 상태로 되돌린다.
	bool m_isPlaying = false;
	std::string m_playSnapshot;

	// 기즈모 상태: 0 이동, 1 회전, 2 크기. (ImGuizmo 타입을 헤더에 노출하지 않으려고 정수로 둔다.)
	int m_gizmoOperation = 0;
	bool m_gizmoLocalSpace = false;
	bool m_gizmoWasUsing = false;

	// File 메뉴에서 연 이름 입력 창의 상태이다. 0 없음, 1 New Scene, 2 Save As.
	int m_pendingScenePopup = 0;
	char m_sceneNameBuffer[64] = {};

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
