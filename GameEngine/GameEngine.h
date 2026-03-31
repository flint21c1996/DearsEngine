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
	void UpdateDemoControls();
	void UpdateLightingState();
	void UpdateSceneObjects(float deltaTime);
	void UpdatePresentationControls();
};
