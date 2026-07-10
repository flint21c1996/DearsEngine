#pragma once

#include <windows.h>
#include <memory>

#include "AStar.h"
#include "DearsGraphicsEngine.h"
#include "EasingFunc.h"
#include "ObjectPool.h"

class TimeManager;
class InputManager;
class FileManager;
class IScene;
class RenderDispatcher;
struct SceneRenderItem;
struct ModelBuffer;
struct VSConstantBufferData;
struct VSBoneConstantBufferData;
struct PSConstantBufferData;
class Camera;
class AStar;

class GameEngine
{
public:
	GameEngine(HWND _hWnd, const int _screenWidth, const int _screenHeight);
	~GameEngine();

private:
	HWND m_hWnd;
	int m_screenWidth;
	int m_screenHeight;

	TimeManager* m_pTimeManager;
	InputManager* m_pInputManager;

	std::unique_ptr<FileManager> m_pFileManager;
	std::unique_ptr<DearsGraphicsEngine> m_pDearsGraphicsEngine;
	std::unique_ptr<RenderDispatcher> m_pRenderDispatcher;
	std::unique_ptr<EasingFunc> tempEasing;

	std::unique_ptr<Camera> tempCamera;
	std::unique_ptr<Camera> lightCamera;
	std::unique_ptr<IScene> m_pActiveScene;

	std::unique_ptr<AStar> tempAStar;
	std::vector<Vector2> tempRawPath;
	Vector3 nextPos;
	size_t pathIndex = 0;

public:
	CommonConstantBufferData tempCCConstantBuffer;
	CommonConstantBufferData tempLightCConstantBuffer;
	PsShadowConstantBufferData tempPSShadowConstantData;

public:
	void Initialize();
	void Update();
	void Render();
	void GameLoop();
	void Finalize();

private:
	void InitializeManager();
	void LoadDemoAssets();
	void InitializeCameras();
	void CreateSceneObjects();
	void InitializeEditorPanels();
	void InitializeLighting();
	void UpdateInputState();
	void UpdateCameraControls(float deltaTime);
	bool IsViewportNavigationActive() const;
	void UpdateDemoControls();
	void UpdateLightingState();
	void UpdateSceneObjects(float deltaTime);
	void UpdatePresentationControls();

	// RHI를 붙이기 전에 Render()를 작은 단계로 나눠둔다.
	// 아직 내부 구현은 DX11 기반 DearsGraphicsEngine을 그대로 쓰지만,
	// 함수 이름만 봐도 프레임 준비, 씬 렌더링, 디버그 렌더링,
	// 데모 전용 UI 출력이 어떤 순서로 실행되는지 알 수 있게 한다.
	void HandleRenderControls();
	void RenderDemoOverlay();
	void RenderShadowPass();
	void RenderScenePass();
	void RenderSelectedOutlinePass();
	void RenderParticlePass();
	void RenderPostProcessPass();
	void RenderDebugPass();
};
