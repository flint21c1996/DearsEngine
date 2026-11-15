#include "GameEngine.h"
#include "TimeManager.h"
#include "InputManager.h"
#include "FileManager.h"
#include "Camera.h"
#include "DemoScene.h"
#include "IScene.h"
#include "RenderDispatcher.h"
#include "RenderPipelinePanel.h"
#include <algorithm>
#include <array>
#include <imgui.h>
#include <math.h>
#include "Pool.h"

///占쏙옙占쏙옙占?占쌓쏙옙트
 
GameEngine::GameEngine(HWND _hWnd, const int _screenWidth, const int _screenHeight)
{
	m_hWnd = _hWnd;
	m_screenWidth = _screenWidth;
	m_screenHeight = _screenHeight;
	m_pTimeManager = nullptr;
	m_pInputManager = nullptr;
	m_pFileManager = std::make_unique<FileManager>();
	m_pDearsGraphicsEngine = std::make_unique<DearsGraphicsEngine>(m_hWnd, m_screenWidth, m_screenHeight);
	m_pRenderDispatcher = std::make_unique<RenderDispatcher>(m_pDearsGraphicsEngine.get());
	tempAStar = std::make_unique<AStar>();
	tempEasing = std::make_unique<EasingFunc>();
}

GameEngine::~GameEngine()
{
	// unique_ptr???먮룞?쇰줈 ?댁젣
}

void GameEngine::Initialize()
{
	// Initialize follows the same order every frame depends on:
	// managers and renderer first, then sample assets, cameras, scene content,
	// editor panels, and finally lighting state.
	InitializeManager();
	m_pDearsGraphicsEngine->Initialize();

	LoadDemoAssets();
	InitializeCameras();
	CreateSceneObjects();
	InitializeEditorPanels();
	InitializeLighting();
	SetupRenderPipeline();
}

void GameEngine::SetupRenderPipeline()
{
	// 프레임이 어떤 패스로, 어떤 순서로 구성되는지는 그래픽스 엔진이 아니라 여기서 정한다.
	// 그래픽스 엔진의 RenderFrame()은 이 목록을 위에서부터 실행하기만 한다.
	//
	// 각 패스가 지정하는 것:
	// - type        : 패스의 분류 (디버그 이름, io를 생략했을 때의 기본 입출력 구성)
	// - io          : 어디에 쓰고 무엇을 읽을지 (백 버퍼 / G-Buffer / 오프스크린 타깃, 깊이 공유 여부 등)
	//                 생략하면 직전 패스가 묶어 둔 출력 대상에 이어서 그린다.
	// - camera      : 이 패스의 View/Projection
	// - commonBuffer: 카메라/라이트 등 패스 전체가 공유하는 상수 버퍼
	// - execute     : 그 상태에서 실제로 무엇을 그릴지
	//
	// 새 패스를 넣고 싶으면 여기에 AddPass를 추가하거나, 실행 중에
	// pipeline.InsertPassAfter("Forward", {...}) / SetPassEnabled("Particle", false)처럼 바꾸면 된다.
	RenderPipeline& pipeline = m_pDearsGraphicsEngine->GetRenderPipeline();
	pipeline.Clear();

	Camera* mainCamera = tempCamera.get();
	CommonConstantBufferData* sceneBuffer = &tempCCConstantBuffer;

	// 데모/디버그 숫자를 ImGui 캔버스에 쌓는다. 실제 출력은 프레임 끝의 ImGui 렌더에서 이뤄진다.
	pipeline.AddPass({
		.name = "DemoOverlay",
		.type = RenderPassType::Ui,
		.execute = [this](IDearsRenderer&) { RenderDemoOverlay(); },
	});

	// 라이트마다(Point는 면마다) 카메라와 Depth 타깃이 바뀌므로 컨텍스트를 패스 안에서 직접 적용한다.
	pipeline.AddPass({
		.name = "Shadow",
		.type = RenderPassType::Shadow,
		.applyContext = false,
		.shouldExecute = [this]() { return m_hasShadowLight; },
		.execute = [this](IDearsRenderer& renderer) { RenderShadowPass(renderer); },
	});

	// Deferred 대상 메시의 표면 정보를 G-Buffer에 기록한다.
	pipeline.AddPass({
		.name = "Geometry",
		.type = RenderPassType::Geometry,
		.camera = mainCamera,
		.commonBuffer = sceneBuffer,
		.io = RenderPassIO{ .color = PassColorOutput::GBuffer },
		.execute = [this](IDearsRenderer&) { RenderGeometryPass(); },
	});

	// G-Buffer와 Shadow Map을 읽어 화면 전체 삼각형 하나로 최종 조명을 계산한다.
	pipeline.AddPass({
		.name = "Lighting",
		.type = RenderPassType::Lighting,
		.camera = mainCamera,
		.commonBuffer = sceneBuffer,
		.io = RenderPassIO{ .color = PassColorOutput::BackBuffer, .readGBuffer = true },
		.execute = [](IDearsRenderer& renderer) { renderer.DrawDeferredLighting(); },
	});

	// Forward로 지정한 메시, CubeMap, 투명/특수 셰이더를 Geometry 깊이를 공유하며 그린다.
	pipeline.AddPass({
		.name = "Forward",
		.type = RenderPassType::Forward,
		.camera = mainCamera,
		.commonBuffer = sceneBuffer,
		.io = RenderPassIO{ .color = PassColorOutput::BackBuffer, .depth = PassDepthOutput::GBufferDepth },
		.execute = [this](IDearsRenderer&) { RenderForwardPass(); },
	});

	pipeline.AddPass({
		.name = "SelectionOutline",
		.type = RenderPassType::SelectionOutline,
		.camera = mainCamera,
		.commonBuffer = sceneBuffer,
		.execute = [this](IDearsRenderer&) { RenderSelectedOutlinePass(); },
	});

	pipeline.AddPass({
		.name = "Particle",
		.type = RenderPassType::Particle,
		.camera = mainCamera,
		.commonBuffer = sceneBuffer,
		.execute = [](IDearsRenderer& renderer) { renderer.DrawParticles(); },
	});

	pipeline.AddPass({
		.name = "PostProcess",
		.type = RenderPassType::PostProcess,
		.execute = [this](IDearsRenderer& renderer) { RenderPostProcessPass(renderer); },
	});

	pipeline.AddPass({
		.name = "Debug",
		.type = RenderPassType::Debug,
		.camera = mainCamera,
		.commonBuffer = sceneBuffer,
		.execute = [this](IDearsRenderer& renderer) { RenderDebugPass(renderer); },
	});
}

void GameEngine::LoadDemoAssets()
{
	// Models
	m_pDearsGraphicsEngine->AddModel("../TestAsset/Test/", "Character 01.FBX");
	m_pDearsGraphicsEngine->AddModel("../TestAsset/Test/", "Armor 05.FBX");
	m_pDearsGraphicsEngine->AddModel("../TestAsset/Test/", "Armor 01.FBX");
	m_pDearsGraphicsEngine->AddModel("../TestAsset/Test/", "Hat 04.FBX");
	m_pDearsGraphicsEngine->AddModel("../TestAsset/Test/", "Wand 01.fbx");
	m_pDearsGraphicsEngine->AddModel("../TestAsset/Test/", "Axe 01.FBX");
	m_pDearsGraphicsEngine->AddModel("../TestAsset/Test/", "Bow 01.FBX");
	m_pDearsGraphicsEngine->AddModel("../TestAsset/Test/", "Scythe 02.FBX");
	m_pDearsGraphicsEngine->AddModel("../TestAsset/Test/", "Shoulder 12.FBX");
	m_pDearsGraphicsEngine->AddModel("../TestAsset/Test/", "Hair Female 04.FBX");
	m_pDearsGraphicsEngine->AddModel("../TestAsset/Test/", "Hood 02.FBX");
	m_pDearsGraphicsEngine->AddModel("../TestAsset/Test/", "Face Female 10.FBX");
	m_pDearsGraphicsEngine->AddModel("../TestAsset/Test/", "Mask 01.FBX");
	m_pDearsGraphicsEngine->AddModel("../TestAsset/Test/", "Cape 02.FBX");
	m_pDearsGraphicsEngine->AddModel("../TestAsset/Test/", "Bow 03.fbx");
	m_pDearsGraphicsEngine->AddModel("../TestAsset/Test/", "Vest 01.FBX");
	m_pDearsGraphicsEngine->AddModel("../TestAsset/Test/", "Shoulder 12.fbx");
	m_pDearsGraphicsEngine->AddModel("../TestAsset/Test/", "Rock_05.fbx");
	m_pDearsGraphicsEngine->AddModel("../TestAsset/Test/", "Sung.fbx");
	m_pDearsGraphicsEngine->AddModel("../TestAsset/", "Michelle.fbx");

	// Animations
	m_pDearsGraphicsEngine->AddAnimation("../TestAsset/", "CatwalkIdle.fbx");
	m_pDearsGraphicsEngine->AddAnimation("../TestAsset/", "CatwalkWalkForward.fbx");
	m_pDearsGraphicsEngine->AddAnimation("../TestAsset/Test/", "Character@Cast Spell 02.FBX");
	m_pDearsGraphicsEngine->AddAnimation("../TestAsset/Test/", "Character@Run Forward Bare Hands In Place.FBX");
	m_pDearsGraphicsEngine->AddAnimation("../TestAsset/Test/", "Character@Die.FBX");
	m_pDearsGraphicsEngine->AddAnimation("../TestAsset/Test/", "Character@Idle.FBX");
	m_pDearsGraphicsEngine->AddAnimation("../TestAsset/Test/", "Character@Mutant Run2.fbx");
	m_pDearsGraphicsEngine->AddAnimation("../TestAsset/Test/", "Character@Mutant Run.FBX");
	m_pDearsGraphicsEngine->AddAnimation("../TestAsset/Test/", "Character@Slash Attack.FBX");

	// Textures
	m_pDearsGraphicsEngine->Add3DTexture("../TestAsset/Test/", "Red.png");
	m_pDearsGraphicsEngine->Add3DTexture("../TestAsset/Test/", "Yellow.png");
	m_pDearsGraphicsEngine->Add3DTexture("../TestAsset/Test/", "Blue.png");
	m_pDearsGraphicsEngine->Add3DTexture("../TestAsset/Test/", "White.png");
	m_pDearsGraphicsEngine->Add3DTexture("../TestAsset/Test/", "BillBoardTest.png");
	m_pDearsGraphicsEngine->Add3DTexture("../TestAsset/Test/", "ss.png");
	m_pDearsGraphicsEngine->Add3DTexture("../TestAsset/Test/", "Water.png");
	m_pDearsGraphicsEngine->Add3DTexture("../TestAsset/Test/", "Hair-Orange.png");
	m_pDearsGraphicsEngine->Add3DTexture("../TestAsset/Test/", "Weapon-Black.png");
	m_pDearsGraphicsEngine->Add3DTexture("../TestAsset/Test/", "coco.jpg");

	m_pDearsGraphicsEngine->Add3DTexture("../TestAsset/metalball2/", "albedo.png");
	m_pDearsGraphicsEngine->Add3DTexture("../TestAsset/metalball2/", "ao.png");
	m_pDearsGraphicsEngine->Add3DTexture("../TestAsset/metalball2/", "metallic.png");
	m_pDearsGraphicsEngine->Add3DTexture("../TestAsset/metalball2/", "normal.png");
	m_pDearsGraphicsEngine->Add3DTexture("../TestAsset/metalball2/", "roughness.png");
	m_pDearsGraphicsEngine->Add3DTexture("../TestAsset/metalball2/", "height.png");
	m_pDearsGraphicsEngine->Add3DTexture("../TestAsset/", "ThinPattern.png");
	m_pDearsGraphicsEngine->Add3DTexture("../TestAsset/", "ThinPattern2.png");
	m_pDearsGraphicsEngine->Add3DTexture("../TestAsset/Test/", "M-Cos-16-Blue.png");

	// DDS textures
	m_pDearsGraphicsEngine->AddDDSTexture("../TestAsset/Test/", "saintpeters.dds");
	m_pDearsGraphicsEngine->AddDDSTexture("../TestAsset/Test/", "Atrium_diffuseIBL.dds");
	m_pDearsGraphicsEngine->AddDDSTexture("../TestAsset/Test/", "Atrium_specularIBL.dds");

	m_pDearsGraphicsEngine->AddDDSTexture("../TestAsset/MyCube1/", "MyCube1EnvHDR.dds");
	m_pDearsGraphicsEngine->AddDDSTexture("../TestAsset/MyCube1/", "MyCube1DiffuseHDR.dds");
	m_pDearsGraphicsEngine->AddDDSTexture("../TestAsset/MyCube1/", "MyCube1SpecularHDR.dds");
	m_pDearsGraphicsEngine->AddDDSTexture("../TestAsset/MyCube1/", "MyCube1Brdf.dds", false);
	m_pDearsGraphicsEngine->AddDDSTexture("../TestAsset/MyCube2/", "MyCube2EnvHDR.dds");
	m_pDearsGraphicsEngine->AddDDSTexture("../TestAsset/MyCube2/", "MyCube2DiffuseHDR.dds");
	m_pDearsGraphicsEngine->AddDDSTexture("../TestAsset/MyCube2/", "MyCube2SpecularHDR.dds");
	m_pDearsGraphicsEngine->AddDDSTexture("../TestAsset/MyCube2/", "MyCube2Brdf.dds", false);
	m_pDearsGraphicsEngine->AddDDSTexture("../TestAsset/MyCube3/", "MyCube3EnvHDR.dds");
	m_pDearsGraphicsEngine->AddDDSTexture("../TestAsset/MyCube3/", "MyCube3DiffuseHDR.dds");
	m_pDearsGraphicsEngine->AddDDSTexture("../TestAsset/MyCube3/", "MyCube3SpecularHDR.dds");
	m_pDearsGraphicsEngine->AddDDSTexture("../TestAsset/MyCube3/", "MyCube3Brdf.dds", false);

	// UI textures and fonts
	m_pDearsGraphicsEngine->Add2DTexture("../TestAsset/Test/", "startButton.png");
	m_pDearsGraphicsEngine->Add2DMipMapTexture("../TestAsset/Test/", "pngegg.png");
	m_pDearsGraphicsEngine->AddFont("../../Resources/Font/", "NotoSansKR-Regular(48).ttf", 48.f, true);
	m_pDearsGraphicsEngine->FontSetFinish();
}

void GameEngine::InitializeCameras()
{
	tempCamera = std::make_unique<Camera>(m_screenWidth, m_screenHeight);
	tempCamera->SetSpeed(20.0f);
	tempCamera->SetEyePos(Vector3(0.f, 0.f, -30.f));
	tempCamera->ProjectionSettings(70.f, 0.1f, 10000.0f);
	tempCamera->SetAircraftAxes(0, 0, 0);
	m_pDearsGraphicsEngine->SetCamera(tempCamera.get());

	lightCamera = std::make_unique<Camera>(m_screenWidth, m_screenHeight);
	lightCamera->SetSpeed(20.0f);
	// 실제 위치와 방향은 Scene에 배치된 Directional/Spot Light에서 매 프레임 가져온다.
	lightCamera->SetEyePos(Vector3::Zero);
	lightCamera->SetDirection(Vector3::UnitZ);
	lightCamera->ProjectionSettings(70.f, 10.f, 1000.0f);
	lightCamera->SetAircraftAxes(0, 0, 0);
}

void GameEngine::CreateSceneObjects()
{
	// The engine owns the active scene object, but the scene itself is
	// responsible for deciding which sample objects exist in the world.
	m_pActiveScene = std::make_unique<DemoScene>(m_pDearsGraphicsEngine.get());
	m_pActiveScene->Initialize();
}

void GameEngine::InitializeEditorPanels()
{
	// 패스 목록 패널: Reset을 누르면 SetupRenderPipeline()이 등록하는 기본 구성으로 되돌린다.
	m_pRenderPipelinePanel = std::make_unique<RenderPipelinePanel>(
		m_pDearsGraphicsEngine.get(),
		[this]() { SetupRenderPipeline(); });
	m_pDearsGraphicsEngine->AddEditorPanel(m_pRenderPipelinePanel.get());

	if (m_pActiveScene)
	{
		m_pActiveScene->RegisterEditorPanels(m_screenWidth);

		// The main gameplay camera should match the reduced render viewport.
		// Without this adjustment the 3D image is stretched after the editor
		// lane shrinks the visible render width.
		// Main-camera aspect is set from the actual viewport image each frame.


	}

	// 기본 에디터 배치: 가운데 월드(Viewport), 오른쪽 위 목록, 오른쪽 아래 속성, 아래쪽 에셋.
	// 같은 자리에 등록한 창은 등록 순서대로 탭으로 묶인다.
	// 처음 실행과 Window > Reset Layout에만 쓰이고, 그 뒤에는 사용자가 옮긴 배치가 imgui.ini에 저장된다.
	m_pDearsGraphicsEngine->SetEditorDockSlot("Scene Hierarchy", EditorDockSlot::Right);
	m_pDearsGraphicsEngine->SetEditorDockSlot("Inspector", EditorDockSlot::RightBottom);
	m_pDearsGraphicsEngine->SetEditorDockSlot("Content Browser", EditorDockSlot::Bottom);
	m_pDearsGraphicsEngine->SetEditorDockSlot("Render Presets", EditorDockSlot::Bottom);
	m_pDearsGraphicsEngine->SetEditorDockSlot("Render Pipeline", EditorDockSlot::Bottom);
	// 디버그용 창들은 아래쪽 탭 뒤편에 모아 둔다. Inspector 자리에는 Inspector만 두어 항상 보이게 한다.
	m_pDearsGraphicsEngine->SetEditorDockSlot("Deferred G-Buffer", EditorDockSlot::Bottom);
	m_pDearsGraphicsEngine->SetEditorDockSlot("Light Shadow Map", EditorDockSlot::Bottom);
	m_pDearsGraphicsEngine->SetEditorDockSlot("Picking", EditorDockSlot::Bottom);
	m_pDearsGraphicsEngine->SetEditorDockSlot("A* Demo", EditorDockSlot::Floating);

	// 메뉴 바의 File/Edit 메뉴와 Play/Stop 버튼은 씬이 그린다. (씬 파일, 실행 취소 기록을 씬이 갖고 있다.)
	m_pDearsGraphicsEngine->SetEditorMenuBarCallback([this]()
	{
		if (m_pActiveScene)
		{
			m_pActiveScene->DrawEditorMenuBar();
		}
	});
}

void GameEngine::InitializeLighting()
{
	// 라이트는 GameEngine에서 만들지 않는다. Scene의 Add Object로 생성하고
	// UpdateLightingState()가 매 프레임 Common Buffer에 수집한다.
	tempCCConstantBuffer.lightNum = 0;
	m_pDearsGraphicsEngine->Set_CubeMap("MyCube1EnvHDR.dds", "MyCube1DiffuseHDR.dds", "MyCube1SpecularHDR.dds", "MyCube1Brdf.dds");
}

void GameEngine::Update()
{
	// Update keeps engine-level flow in one place:
	// input, camera, demo controls, lighting, scene objects, presentation, renderer.
	m_pTimeManager->Tick();
	const float deltaTime = m_pTimeManager->DeltaTime();

	UpdateInputState();

	// ImGui가 텍스트 입력이나 콤보/팝업 조작 때문에 키보드를 잡고 있을 때는
	// 엔진 단축키를 처리하지 않는다.
	//
	// 예를 들어 Add Object의 Name 입력창에서 W/A/S/D를 누르는 동안
	// 파티클 위치나 카메라 이동이 같이 처리되면 에디터 입력과 게임 입력이 섞인다.
	const auto mouse = m_pInputManager->GetMousePos();
	const bool viewportInput = m_pDearsGraphicsEngine->GetEditorViewport().AllowsSceneInput(mouse.x, mouse.y);
	const bool blockEngineKeyboardInput = !viewportInput || ImGui::GetIO().WantTextInput;
	const bool viewportNavigationActive = IsViewportNavigationActive();
	if (!blockEngineKeyboardInput)
	{
		UpdateCameraControls(deltaTime);
		if (!viewportNavigationActive && m_pActiveScene)
		{
			m_pActiveScene->HandlePickingInput(*m_pInputManager, tempCamera.get());
		}
		// 데모용 키(파티클 위치 WASD, F 생성 등)는 Play 중에만 받는다.
		// 편집 중에는 W/E/R이 기즈모 전환이라서 같은 키가 두 가지 일을 하면 안 된다.
		if (!viewportNavigationActive && m_pActiveScene && m_pActiveScene->IsPlaying())
		{
			UpdateDemoControls();
		}
	}

	// 에디터 조작(기즈모, Ctrl+Z/Y, 뷰포트 드롭)은 뷰포트에 마우스가 없어도 처리해야 한다.
	// 예: Inspector에서 값을 바꾼 직후의 Ctrl+Z, 드래그가 끝난 뒤의 실행 취소 기록.
	// Viewport 창이 열려 있는 지금 호출해야 기즈모가 그 창 안에 그려진다.
	if (m_pActiveScene)
	{
		m_pActiveScene->UpdateEditor(*m_pInputManager, tempCamera.get());
	}

	UpdateLightingState();
	UpdateSceneObjects(deltaTime);
	if (!blockEngineKeyboardInput && m_pActiveScene && m_pActiveScene->IsPlaying())
	{
		UpdatePresentationControls();
	}
	m_pDearsGraphicsEngine->Update();
}

void GameEngine::UpdateInputState()
{
	m_pInputManager->Update();

	// 언리얼 에디터처럼 RMB를 누르고 있는 동안만 viewport navigation 모드로 둔다.
	// 예전 T 토글 방식은 한 번 켜면 계속 마우스 look 상태로 남아서,
	// 에디터 선택/피킹 입력과 카메라 입력이 서로 섞이기 쉬웠다.
    const auto& viewport = m_pDearsGraphicsEngine->GetEditorViewport();
    if (viewport.Valid()) tempCamera->SetAspectRatio(viewport.PixelWidth(), viewport.PixelHeight());
    const bool active = IsViewportNavigationActive();
    const auto mouse = m_pInputManager->GetMousePos();
    // Seed the look anchor on RMB entry; window movement must not jump the camera.
    if (active && !m_wasViewportNavigation)
    {
        tempCamera->mIsFirstPersonMode = false;
        tempCamera->OnMouseMove(int(mouse.x), int(mouse.y));
    }
    tempCamera->mIsFirstPersonMode = active;
    tempCamera->OnMouseMove(int(mouse.x), int(mouse.y));
    m_wasViewportNavigation = active;
}

void GameEngine::UpdateCameraControls(float deltaTime)
{
	if (!IsViewportNavigationActive())
	{
		return;
	}

	// Viewport navigation은 Unreal Editor에 가깝게 RMB + WASD/QE로 이동한다.
	// RMB를 떼면 즉시 선택/피킹 모드로 돌아가므로 LMB 피킹과 충돌하지 않는다.
	if (m_pInputManager->GetKeyState(KEY::W) == KEY_STATE::HOLD ||
		m_pInputManager->GetKeyState(KEY::UP) == KEY_STATE::HOLD)
	{
		tempCamera->MoveForward(deltaTime);
	}
	if (m_pInputManager->GetKeyState(KEY::S) == KEY_STATE::HOLD ||
		m_pInputManager->GetKeyState(KEY::DOWN) == KEY_STATE::HOLD)
	{
		tempCamera->MoveForward(-deltaTime);
	}
	if (m_pInputManager->GetKeyState(KEY::A) == KEY_STATE::HOLD ||
		m_pInputManager->GetKeyState(KEY::LEFT) == KEY_STATE::HOLD)
	{
		tempCamera->MoveRight(-deltaTime);
	}
	if (m_pInputManager->GetKeyState(KEY::D) == KEY_STATE::HOLD ||
		m_pInputManager->GetKeyState(KEY::RIGHT) == KEY_STATE::HOLD)
	{
		tempCamera->MoveRight(deltaTime);
	}
	if (m_pInputManager->GetKeyState(KEY::E) == KEY_STATE::HOLD ||
		m_pInputManager->GetKeyState(KEY::N) == KEY_STATE::HOLD)
	{
		tempCamera->MoveUp(deltaTime);
	}
	if (m_pInputManager->GetKeyState(KEY::Q) == KEY_STATE::HOLD ||
		m_pInputManager->GetKeyState(KEY::M) == KEY_STATE::HOLD)
	{
		tempCamera->MoveUp(-deltaTime);
	}
}

bool GameEngine::IsViewportNavigationActive() const
{
	if (!m_pInputManager)
	{
		return false;
	}

	const KEY_STATE rightMouseState = m_pInputManager->GetKeyState(KEY::RBUTTON);
	const auto mouse = m_pInputManager->GetMousePos();
	return m_pDearsGraphicsEngine->GetEditorViewport().AllowsSceneInput(mouse.x, mouse.y)
		&& (rightMouseState == KEY_STATE::TAP || rightMouseState == KEY_STATE::HOLD);
}

void GameEngine::UpdateDemoControls()
{
	// Sample-scene controls are delegated to the scene
	// so the engine loop only coordinates high-level flow.
	if (m_pActiveScene)
	{
		m_pActiveScene->HandleDemoInput(*m_pInputManager);
	}
}

void GameEngine::UpdateLightingState()
{
	m_hasShadowLight = false;
	m_selectedShadowLightIndex = -1;
	if (!m_pActiveScene)
	{
		tempCCConstantBuffer.lightNum = 0;
		return;
	}

	// Scene은 라이트를 Hierarchy 순서로 수집하고 선택된 라이트의 인덱스만 반환한다.
	// 선택 여부가 GPU 배열 순서를 바꾸지 않으므로 Shadow Map 배열과 lights[]의 대응이 안정적이다.
	m_selectedShadowLightIndex = m_pActiveScene->CollectLights(tempCCConstantBuffer);
	for (UINT index = 0; index < tempCCConstantBuffer.lightNum; ++index)
	{
		Light& light = tempCCConstantBuffer.light[index];
		if (light.lightType != static_cast<UINT>(LightEnum::DIRECTIONAL_LIGHT) &&
			light.lightType != static_cast<UINT>(LightEnum::POINT_LIGHT) &&
			light.lightType != static_cast<UINT>(LightEnum::SPOT_LIGHT))
		{
			continue;
		}

		// Directional/Spot은 메인 픽셀 셰이더에서 이 행렬로 월드 위치를 Shadow UV로 바꾼다.
		// Point는 여섯 면을 개별 카메라로 그리므로 이 행렬을 샘플링에 사용하지 않지만,
		// 구조체 값을 유효하게 유지하기 위해 기본 +X 면의 행렬을 기록한다.
		ConfigureShadowCamera(light);
		light.viewProj = (lightCamera->GetViewRow() * lightCamera->GetProjRow()).Transpose();
		m_hasShadowLight = true;
	}
}

void GameEngine::ConfigureShadowCamera(const Light& light)
{
	lightCamera->SetEyePos(light.position);

	if (light.lightType == static_cast<UINT>(LightEnum::DIRECTIONAL_LIGHT))
	{
		// Directional Light는 광선이 서로 평행하므로 직교 투영을 사용한다.
		// shadowWidth는 그림자 맵에 담을 월드 가로 길이이고, 세로는 화면비로 계산한다.
		lightCamera->SetAspectRatio(m_screenWidth, m_screenHeight);
		lightCamera->SetDirection(light.direction, light.shadowUp);
		const float shadowAspect = (std::max)(lightCamera->GetAspectRatio(), 0.001f);
		lightCamera->SetOrthographicSize(
			light.shadowWidth,
			light.shadowWidth / shadowAspect);
		lightCamera->ProjectionSettings(light.shadowFovY, light.shadowNear, light.shadowFar);
		return;
	}

	if (light.lightType == static_cast<UINT>(LightEnum::SPOT_LIGHT))
	{
		// Spot Light는 한 점에서 원뿔 형태로 퍼지므로 원근 투영을 사용한다.
		lightCamera->SetAspectRatio(m_screenWidth, m_screenHeight);
		lightCamera->SetDirection(light.direction, light.shadowUp);
		lightCamera->SetPerspective();
		lightCamera->ProjectionSettings(light.shadowFovY, light.shadowNear, light.shadowFar);
		return;
	}

	// Point Light는 정육면체 여섯 면을 채우기 위해 1:1 화면비와 90도 시야각을 사용한다.
	// 실제 면 방향은 RenderShadowPass에서 면마다 다시 설정한다.
	lightCamera->SetAspectRatio(1, 1);
	lightCamera->SetDirection(Vector3::UnitX, Vector3::UnitY);
	lightCamera->SetPerspective();
	lightCamera->ProjectionSettings(90.0f, light.shadowNear, light.shadowFar);
}

void GameEngine::UpdateSceneObjects(float deltaTime)
{
	if (m_pActiveScene)
	{
		m_pActiveScene->Update(deltaTime);
	}
}

void GameEngine::UpdatePresentationControls()
{
	// Visual test controls belong to the demo scene because they only tweak
	// sample content and do not describe engine-wide behavior.
	if (m_pActiveScene)
	{
		m_pActiveScene->HandlePresentationInput(*m_pInputManager, *tempEasing, m_pTimeManager->DeltaTime());
	}
}

void GameEngine::Render()
{
	if (!m_pActiveScene)
	{
		return;
	}

	// 한 프레임의 패스 종류와 순서는 SetupRenderPipeline()에서 등록한 목록이 결정한다.
	// 여기서는 입력에 따른 상태 변경만 처리하고, 프레임 시작 -> 등록된 패스 실행 -> Present는
	// 그래픽스 엔진의 RenderFrame()에 맡긴다.
	HandleRenderControls();

	m_pDearsGraphicsEngine->RenderFrame();
}

void GameEngine::HandleRenderControls()
{
	const auto mouse = m_pInputManager->GetMousePos();
	const bool viewportInput = m_pDearsGraphicsEngine->GetEditorViewport().AllowsSceneInput(mouse.x, mouse.y);
	const bool blockEngineKeyboardInput = !viewportInput || ImGui::GetIO().WantTextInput;
	if (blockEngineKeyboardInput)
	{
		return;
	}

	// 씬 전용 렌더 테스트 입력은 아직 ActiveScene이 처리한다.
	// 예를 들어 현재 DemoScene은 여기서 PBR 구를 회전시키거나
	// height scale 값을 조절한다.
	// 나중에는 이런 테스트 입력을 에디터 툴이나 컴포넌트 쪽으로 옮길 수 있다.
	m_pActiveScene->HandleRenderInput(*m_pInputManager);

	// Z/X 키는 메인 카메라의 투영 방식을 바꾼다.
	// 이 처리를 RenderForwardPass()와 분리해두면,
	// 카메라 모드 변경은 draw call이 아니라 입력에 따른 상태 변경이라는 점이
	// 코드 흐름에서 분명해진다.
	if (m_pInputManager->GetKeyState(KEY::Z) == KEY_STATE::TAP)
	{
		tempCamera->SetPerspective();
	}
	if (m_pInputManager->GetKeyState(KEY::X) == KEY_STATE::TAP)
	{
		tempCamera->SetOrthgraphic(0.1f);
	}
}

void GameEngine::RenderDemoOverlay()
{
	if (!m_pDearsGraphicsEngine->ShowViewportOverlay()) return;
	// 이 오버레이는 엔진 핵심 렌더링이 아니라 데모/디버그 표시용 UI다.
	// FPS, 마우스 좌표, 카메라 벡터, 라이트 view-projection 행렬처럼
	// GameEngine이 들고 있는 상태를 읽기 때문에 지금은 여기에 남겨둔다.
	//
	// 나중에 목표로 삼을 분리 방향:
	// - 사용자가 직접 조작하는 에디터 패널은 editor layer로 이동
	// - 임시 디버그 숫자는 DebugOverlay 같은 클래스로 이동
	// - DX11/DX12/Vulkan 백엔드는 이런 UI 내용 자체를 몰라도 되게 만들기
	//
	// UI 함수들은 아직 IDearsRenderer 인터페이스 밖(DX11/ImGui 구현 쪽)에 있어서
	// 이 패스만 구체 클래스 포인터를 직접 사용한다.

	// 현재 샘플 화면을 보여주기 위한 데모 이미지들이다.
	m_pDearsGraphicsEngine->UIDrawImageStart();
	m_pDearsGraphicsEngine->UIDrawImage(Vector2(1720, 20), Vector2(200, 200), "coco.jpg", Vector4(0.2f, 0.2f, 0.2f, 0.8f));
	m_pDearsGraphicsEngine->UIDrawImage(Vector2(0, 0), Vector2(200, 200), "coco.jpg");
	m_pDearsGraphicsEngine->UIDrawImage(m_pActiveScene->GetSecondaryUiPoint(), Vector2(200, 100), "startButton.png");
	m_pDearsGraphicsEngine->UIDrawImageFin();

	m_pDearsGraphicsEngine->UIStartFontID("NotoSansKR-Regular(48).ttf");
	// Overlay is clipped to the world image; do not change the viewport window font scale.
	m_pDearsGraphicsEngine->UIDrawText(
		Vector2(150, 600),
		u8"1~3: 큐브맵 변경\nWASD: 파티클 위치 이동\nF: 파티클 생성",
		Vector4(1.0f));
	
	m_pDearsGraphicsEngine->UIDrawTextWithNum(Vector2(0, 100), u8"FPS : %.1f", Vector4(1.0f), m_pTimeManager->FPS());
	
	m_pDearsGraphicsEngine->UIDrawTextWithNum(
		Vector2(0, 120),
		u8"마우스 x좌표: %.2f \n마우스 y좌표: %.2f",
		Vector4(1.0f),
		m_pInputManager->GetMousePos().x,
		m_pInputManager->GetMousePos().y);
	
	m_pDearsGraphicsEngine->UIDrawTextWithNum(
		Vector2(0, 170),
		u8"카메라 좌표: %.2f, %.2f, %.2f",
		Vector4(0.5f),
		tempCamera->GetmViewPos().x,
		tempCamera->GetmViewPos().y,
		tempCamera->GetmViewPos().z);
	
	m_pDearsGraphicsEngine->UIDrawTextWithNum(
		Vector2(0, 190),
		u8"카메라 axis(RPY): %.1f, %.1f, %.1f",
		Vector4(1.0f),
		DirectX::XMConvertToDegrees(tempCamera->mRoll),
		(tempCamera->mPitch),
		(tempCamera->mYaw));
	
	m_pDearsGraphicsEngine->UIDrawTextWithNum(
		Vector2(0, 210),
		u8"Light 카메라 좌표: %.2f, %.2f, %.2f",
		Vector4(1.0f),
		lightCamera->GetmViewPos().x,
		lightCamera->GetmViewPos().y,
		lightCamera->GetmViewPos().z);
	Matrix e = tempCCConstantBuffer.light[0].viewProj;
	m_pDearsGraphicsEngine->UIDrawImage(m_pActiveScene->GetPrimaryUiPoint(), Vector2(200, 100), "startButton.png");

	m_pDearsGraphicsEngine->UIDrawTextWithNum(Vector2(0, 230), u8"light viewProj: %.2f, %.2f, %.2f, %.2f", Vector4(1.0f), e._11, e._12, e._13, e._14);
	m_pDearsGraphicsEngine->UIDrawTextWithNum(Vector2(0, 250), u8"light viewProj: %.2f, %.2f, %.2f, %.2f", Vector4(1.0f), e._21, e._22, e._23, e._24);
	m_pDearsGraphicsEngine->UIDrawTextWithNum(Vector2(0, 270), u8"light viewProj: %.2f, %.2f, %.2f, %.2f", Vector4(1.0f), e._31, e._32, e._33, e._34);
	m_pDearsGraphicsEngine->UIDrawTextWithNum(Vector2(0, 290), u8"light viewProj: %.2f, %.2f, %.2f, %.2f", Vector4(1.0f), e._41, e._42, e._43, e._44);
	Vector3 e1 = tempCamera->mViewDir;
	m_pDearsGraphicsEngine->UIDrawTextWithNum(Vector2(0, 310), u8"mViewDir: %.2f, %.2f, %.2f", Vector4(1.0f),e1.x, e1.y, e1.z);
	e1 = tempCamera->mRightDir;
	m_pDearsGraphicsEngine->UIDrawTextWithNum(Vector2(0, 330), u8"mRightVec: %.2f, %.2f, %.2f", Vector4(1.0f),e1.x, e1.y, e1.z);
	e1 = tempCamera->mViewUp;
	m_pDearsGraphicsEngine->UIDrawTextWithNum(Vector2(0, 350), u8"mUpVec: %.2f, %.2f, %.2f", Vector4(1.0f),e1.x, e1.y, e1.z);
	m_pDearsGraphicsEngine->UIFinFontID();
}

void GameEngine::RenderShadowPass(IDearsRenderer& renderer)
{
	// 그림자를 만드는 라이트가 없는 프레임은 파이프라인의 shouldExecute가 이 패스를 건너뛴다.
	//
	// Shadow pass:
	// 라이트 카메라 시점에서 씬을 depth-only target에 먼저 그린다.
	// 이후 메인 씬 패스에서 이 depth texture를 샘플링해서
	// 각 픽셀이 그림자 안에 있는지 판단한다.
	//
	// 이 경계가 RHI에서 중요한 이유:
	// DX11은 resource transition을 상당 부분 숨겨주지만,
	// DX12/Vulkan에서는 "depth write -> shader read" 같은 상태 전환을
	// 명시적으로 해줘야 한다.
	// 그래서 이 함수 경계는 나중에 실제 command list / render pass 경계가 된다.
	RenderContext shadowContext;
	shadowContext.passType = RenderPassType::Shadow;
	shadowContext.camera = lightCamera.get();
	shadowContext.commonBuffer = &tempLightCConstantBuffer;

	// Direct3D TextureCube의 표준 면 순서이다.
	// Y축 면은 Up 벡터까지 Y축을 쓰면 direction과 평행해지므로 각각 -Z/+Z를 사용한다.
	static const std::array<Vector3, 6> faceDirections =
	{
		Vector3::UnitX, -Vector3::UnitX,
		Vector3::UnitY, -Vector3::UnitY,
		Vector3::UnitZ, -Vector3::UnitZ
	};
	static const std::array<Vector3, 6> faceUps =
	{
		Vector3::UnitY, Vector3::UnitY,
		-Vector3::UnitZ, Vector3::UnitZ,
		Vector3::UnitY, Vector3::UnitY
	};

	// lights[]의 인덱스와 Shadow Map 배열의 인덱스를 동일하게 사용한다.
	// 선택 여부와 무관하게 등록된 모든 라이트의 Shadow Map을 매 프레임 갱신한다.
	for (UINT lightIndex = 0; lightIndex < tempCCConstantBuffer.lightNum; ++lightIndex)
	{
		const Light& shadowLight = tempCCConstantBuffer.light[lightIndex];
		const bool isDirectional =
			shadowLight.lightType == static_cast<UINT>(LightEnum::DIRECTIONAL_LIGHT);
		const bool isPoint =
			shadowLight.lightType == static_cast<UINT>(LightEnum::POINT_LIGHT);
		const bool isSpot =
			shadowLight.lightType == static_cast<UINT>(LightEnum::SPOT_LIGHT);
		if (!isDirectional && !isPoint && !isSpot)
		{
			continue;
		}

		ConfigureShadowCamera(shadowLight);
		if (isPoint)
		{
			// 같은 Scene을 여섯 번 그리지만 카메라 방향과 DSV 면만 바뀐다.
			// lightIndex * 6 + faceIndex가 TextureCubeArray의 실제 배열 슬라이스가 된다.
			for (UINT faceIndex = 0; faceIndex < faceDirections.size(); ++faceIndex)
			{
				lightCamera->SetDirection(faceDirections[faceIndex], faceUps[faceIndex]);
				renderer.ApplyRenderContext(shadowContext);
				renderer.BeginPointShadowFace(lightIndex, faceIndex);
				m_pRenderDispatcher->RenderShadowItems(m_pActiveScene->GetShadowRenderItems());
			}
		}
		else
		{
			// Directional과 Spot은 투영 방식은 다르지만 같은 Texture2DArray를 사용한다.
			// Begin 함수가 현재 lightIndex에 해당하는 DSV 슬라이스를 골라서 바인딩한다.
			renderer.ApplyRenderContext(shadowContext);
			renderer.BeginTwoDimensionalShadowPass(lightIndex);
			m_pRenderDispatcher->RenderShadowItems(m_pActiveScene->GetShadowRenderItems());
		}
	}

	// 모든 DSV 쓰기가 끝난 뒤 배열 SRV를 메인 픽셀 셰이더에 한 번만 연결한다.
	// DX12/Vulkan에서는 이 위치가 Depth Write에서 Shader Read로 전환하는 배리어가 된다.
	renderer.EndShadowPass();

	// 하이어라키 선택은 Shadow Map 생성 대상을 제한하지 않는다.
	// 여기서는 이미 만들어진 배열 중 어떤 슬라이스를 디버그 창에 보여줄지만 결정한다.
	if (m_selectedShadowLightIndex >= 0 &&
		static_cast<UINT>(m_selectedShadowLightIndex) < tempCCConstantBuffer.lightNum)
	{
		const UINT selectedLightIndex = static_cast<UINT>(m_selectedShadowLightIndex);
		const Light& selectedLight = tempCCConstantBuffer.light[selectedLightIndex];
		ConfigureShadowCamera(selectedLight);

		if (selectedLight.lightType == static_cast<UINT>(LightEnum::POINT_LIGHT))
		{
			m_pDearsGraphicsEngine->RenderPointShadowMapDebugPreview(
				selectedLightIndex,
				selectedLight.shadowNear,
				selectedLight.shadowFar);
		}
		else if (selectedLight.lightType == static_cast<UINT>(LightEnum::DIRECTIONAL_LIGHT) ||
			selectedLight.lightType == static_cast<UINT>(LightEnum::SPOT_LIGHT))
		{
			m_pDearsGraphicsEngine->RenderShadowMapDebugPreview(
				selectedLightIndex,
				selectedLight.shadowNear,
				selectedLight.shadowFar,
				selectedLight.lightType == static_cast<UINT>(LightEnum::SPOT_LIGHT));
		}
	}
}

void GameEngine::RenderGeometryPass()
{
	// Deferred Geometry pass:
	// 디퍼드 렌더링에서는 씬의 모든 불투명 오브젝트를 먼저 순회하면서
	// 최종 조명 색상이 아니라 Albedo, Normal, Material, Depth 같은 표면 정보를
	// 화면 크기의 여러 렌더 타깃(G-Buffer)에 기록한다.
	//
	// G-Buffer와 Geometry 전용 Pixel Shader가 준비되었으므로 이 패스에서
	// PBR 불투명 메시의 표면 정보를 실제 MRT에 기록한다.
	// 다만 Lighting Pass가 아직 최종 색상을 만들지 못하므로, 같은 PBR 메시는
	// 아래 Forward Pass에서도 임시로 한 번 더 그려 기존 화면 출력을 유지한다.
	//
	// G-Buffer 바인딩과 카메라/공통 버퍼 적용은 파이프라인이 이 함수 직전에
	// { Geometry, 메인 카메라, 씬 공통 버퍼 } 컨텍스트로 처리한다.

	// 우선 PBR 불투명 메시만 Geometry Pass에서 G-Buffer에 기록한다.
	// Lighting 구현 전에는 Forward Pass에서도 다시 그려 기존 화면을 보존한다.
	m_pRenderDispatcher->RenderGeometryItems(m_pActiveScene->GetMainRenderItems());
}

void GameEngine::RenderForwardPass()
{
	// Forward pass:
	// Deferred Default Lit PBR은 앞의 Geometry + Lighting Pass에서 이미 최종 색상이 완성됐다.
	// 여기서는 Forward로 지정한 메시와 CubeMap, 투명/물/특수 셰이더만 추가로 렌더링한다.
	m_pRenderDispatcher->RenderForwardItems(m_pActiveScene->GetMainRenderItems());
}

void GameEngine::RenderSelectedOutlinePass()
{
	// Selection outline pass:
	// 피킹이나 Hierarchy 패널을 통해 선택된 오브젝트를 기존 외곽선 셰이더로 강조한다.
	// 선택 자체는 Scene이 관리하고, 실제 외곽선 렌더 경로 선택은 RenderDispatcher가 맡는다.
	//
	// 지금은 DX11 stencil 기반 외곽선 렌더를 직접 호출하지만,
	// 나중에 에디터 전용 overlay pass나 post-process outline으로 바꾸더라도
	// GameEngine은 "선택 강조 패스를 실행한다"는 순서만 유지하면 된다.
	m_pRenderDispatcher->RenderSelectedOutline(m_pActiveScene->GetSelectedObject(), tempCamera.get());
}

void GameEngine::RenderPostProcessPass(IDearsRenderer& renderer)
{
	// 포스트 프로세싱은 현재 꺼져 있지만, 기존 테스트 키 동작을 보존하려고
	// 토글 구조는 남겨둔다.
	// 나중에 bloom, tone mapping, SSAO 같은 후처리 효과를 다시 붙이면
	// 이 함수에서 관련 패스를 호출하면 된다.
	(void)renderer;

	static bool a = true;
	if (m_pInputManager->GetKeyState(KEY::_7) == KEY_STATE::HOLD)
	{
		a = true;
	}
	if (m_pInputManager->GetKeyState(KEY::_8) == KEY_STATE::HOLD)
	{
		a = false;
	}
	if (a == true)
	{
		//renderer.DrawPostProcess();
	}
}

void GameEngine::RenderDebugPass(IDearsRenderer& renderer)
{
	// Debug pass:
	// 메인 씬을 그린 뒤에 엔진 시각화용 helper geometry를 추가로 그린다.
	// 디버그 기하도 일반 장면 렌더링과 같은 카메라를 사용해야 월드 좌표와 정확히 겹친다.
	// 아래 RenderContext를 적용한 뒤 라이트 기즈모를 그리면 현재 뷰/투영 행렬이
	// DebugLineVertexShader에도 전달되므로, 카메라가 움직여도 선이 라이트 위치를 따라간다.
	// (이 컨텍스트는 파이프라인이 Debug 패스 실행 직전에 적용한다.)

	// Scene에 실제로 배치된 라이트만 기즈모로 표시한다.
	// 기즈모는 Debug Pass에서 그리므로 G-Buffer, Shadow Map, Picking 대상에는 포함되지 않는다.
	for (UINT lightIndex = 0; lightIndex < tempCCConstantBuffer.lightNum; ++lightIndex)
	{
		const Light& light = tempCCConstantBuffer.light[lightIndex];
		const bool drawShadowFrustum =
			m_hasShadowLight &&
			m_selectedShadowLightIndex >= 0 &&
			lightIndex == static_cast<UINT>(m_selectedShadowLightIndex) &&
			(light.lightType == static_cast<UINT>(LightEnum::DIRECTIONAL_LIGHT) ||
			 light.lightType == static_cast<UINT>(LightEnum::SPOT_LIGHT));

		renderer.DrawLightGizmo(
			light,
			drawShadowFrustum,
			lightCamera->GetAspectRatio());
	}
}

void GameEngine::GameLoop()
{
	m_pDearsGraphicsEngine->BeginEditorFrame();
	Update();
	Render();
}

void GameEngine::Finalize()
{
	m_pActiveScene.reset();

	if (m_pInputManager)
	{
		InputManager::destroyInstance();
		m_pInputManager = nullptr;
	}

	if (m_pTimeManager)
	{
		TimeManager::destroyInstance();
		m_pTimeManager = nullptr;
	}
}

/// 占쏙옙占쏙옙占쏙옙-----------------------------------------------------------------------------------
void GameEngine::InitializeManager()
{
	m_pTimeManager = TimeManager::getInstance();
	m_pTimeManager->Reset();

	m_pInputManager = InputManager::getInstance();
	m_pInputManager->SetHwnd(m_hWnd);

	m_pFileManager->Initialize();
	//m_pFileManager->SearchOneFile("Michelle", "Michelle.fbx");
}

bool GameEngine::ResizeClient(int width, int height)
{
    // The fixed shadow-map extents keep their original projection. Main-camera
    // aspect follows the image, independently of the native presentation extent.
    const bool ready = m_pDearsGraphicsEngine->ResizeClient(width, height);
    m_pTimeManager->Tick(); // Discard time spent minimized or in the native resize modal loop.
    m_wasViewportNavigation = false;
    return ready;
}
