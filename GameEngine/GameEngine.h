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
class RenderPipelinePanel;
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
	// 패스를 켜고 끄고 순서를 바꾸는 에디터 패널이다. 패스 구성은 엔진 단위라서 Scene이 아니라 여기서 소유한다.
	std::unique_ptr<RenderPipelinePanel> m_pRenderPipelinePanel;
	std::unique_ptr<EasingFunc> tempEasing;

	std::unique_ptr<Camera> tempCamera;
	std::unique_ptr<Camera> lightCamera;
	bool m_hasShadowLight = false;
	bool m_wasViewportNavigation = false;
	// 에디터에서 선택한 라이트의 GPU 배열 인덱스다.
	// 모든 라이트의 그림자는 항상 만들고, 이 값은 미리보기와 선택 절두체에만 사용한다.
	int m_selectedShadowLightIndex = -1;
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
	bool ResizeClient(int width, int height);
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
	// Light 종류에 맞춰 공용 Shadow Camera의 위치, 방향, 투영 방식을 설정한다.
	void ConfigureShadowCamera(const Light& light);
	void UpdateSceneObjects(float deltaTime);
	void UpdatePresentationControls();

	void HandleRenderControls();

	// 한 프레임의 패스 구성(종류, 순서, 카메라, 출력 대상)을 그래픽스 엔진의 RenderPipeline에 등록한다.
	// 패스를 추가하거나 순서를 바꾸고 싶으면 이 함수만 고치면 되고,
	// 실행 중에는 GetRenderPipeline()의 InsertPass/SetPassEnabled 등으로 바꿀 수 있다.
	void SetupRenderPipeline();

	// 아래 함수들은 파이프라인에 등록된 각 패스의 "내용"이다.
	// 렌더 타깃 바인딩과 카메라/공통 버퍼 적용은 파이프라인이 패스 실행 직전에 처리하므로,
	// 여기서는 무엇을 그릴지만 다룬다. (Shadow는 라이트마다 타깃이 바뀌어서 직접 준비한다.)
	void RenderDemoOverlay();
	void RenderShadowPass(IDearsRenderer& renderer);
	void RenderGeometryPass();
	void RenderForwardPass();
	void RenderSelectedOutlinePass();
	void RenderPostProcessPass(IDearsRenderer& renderer);
	void RenderDebugPass(IDearsRenderer& renderer);
};
