#include "GameEngine.h"
#include "TimeManager.h"
#include "InputManager.h"
#include "FileManager.h"
#include "Camera.h"
#include "DemoScene.h"
#include "IScene.h"
#include "RenderDispatcher.h"
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
	if (m_pActiveScene)
	{
		m_pActiveScene->RegisterEditorPanels(m_screenWidth);

		// The main gameplay camera should match the reduced render viewport.
		// Without this adjustment the 3D image is stretched after the editor
		// lane shrinks the visible render width.
		const int editorPanelWidth = GetEditorPanelWidth(static_cast<float>(m_screenWidth));
		const int renderViewportWidth = m_screenWidth - editorPanelWidth;
		tempCamera->SetAspectRatio(renderViewportWidth, m_screenHeight);
	}
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
	const bool blockEngineKeyboardInput = ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantTextInput;
	const bool viewportNavigationActive = IsViewportNavigationActive();
	if (!blockEngineKeyboardInput)
	{
		UpdateCameraControls(deltaTime);
		if (!viewportNavigationActive && m_pActiveScene)
		{
			m_pActiveScene->HandlePickingInput(*m_pInputManager, tempCamera.get());
		}
		if (!viewportNavigationActive)
		{
			UpdateDemoControls();
		}
	}

	UpdateLightingState();
	UpdateSceneObjects(deltaTime);
	if (!blockEngineKeyboardInput)
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
	tempCamera->mIsFirstPersonMode = IsViewportNavigationActive();

	if (!ImGui::GetIO().WantCaptureMouse)
	{
		tempCamera->OnMouseMove(static_cast<int>(m_pInputManager->GetMousePos().x), static_cast<int>(m_pInputManager->GetMousePos().y));
	}
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
	if (!m_pInputManager || ImGui::GetIO().WantCaptureMouse)
	{
		return false;
	}

	const KEY_STATE rightMouseState = m_pInputManager->GetKeyState(KEY::RBUTTON);
	return rightMouseState == KEY_STATE::TAP || rightMouseState == KEY_STATE::HOLD;
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
	if (!m_pActiveScene)
	{
		tempCCConstantBuffer.lightNum = 0;
		return;
	}

	m_pActiveScene->CollectLights(tempCCConstantBuffer);
	for (UINT index = 0; index < tempCCConstantBuffer.lightNum; ++index)
	{
		Light& light = tempCCConstantBuffer.light[index];
		if (light.lightType != static_cast<UINT>(LightEnum::DIRECTIONAL_LIGHT) &&
			light.lightType != static_cast<UINT>(LightEnum::SPOT_LIGHT))
		{
			continue;
		}

		// 현재 엔진은 Shadow Map이 한 장뿐이므로 첫 Directional/Spot Light만 그림자를 만든다.
		// 기존 셰이더가 shadowMaps[0]과 lights[0].viewProj를 한 쌍으로 사용하므로,
		// 그림자를 만드는 라이트를 배열의 0번으로 옮겨 두 데이터의 기준을 일치시킨다.
		if (index != 0)
		{
			Light firstLight = tempCCConstantBuffer.light[0];
			tempCCConstantBuffer.light[0] = light;
			tempCCConstantBuffer.light[index] = firstLight;
		}
		Light& shadowLight = tempCCConstantBuffer.light[0];
		lightCamera->SetEyePos(shadowLight.position);
		lightCamera->SetDirection(shadowLight.direction);
		shadowLight.viewProj = (lightCamera->GetViewRow() * lightCamera->GetProjRow()).Transpose();
		m_hasShadowLight = true;
		break;
	}
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

	// 이 함수는 한 프레임의 렌더링 순서를 보여주는 최상위 함수다.
	//
	// RHI 리팩터링에서 중요한 생각:
	// GameEngine은 나중에 "다음에 어떤 렌더 패스를 실행할지"만 결정하고,
	// 실제로 그 패스를 DX11, DX12, Vulkan 중 어떤 방식으로 실행할지는
	// 선택된 그래픽스 백엔드가 맡게 만드는 것이 목표다.
	// 지금은 아직 DearsGraphicsEngine을 직접 호출하지만, 패스 경계가
	// 보이도록 먼저 함수를 나눠둔다.
	HandleRenderControls();

	m_pDearsGraphicsEngine->BeginRender();
	RenderDemoOverlay();
	RenderShadowPass();
	RenderGeometryPass();
	RenderLightingPass();
	RenderForwardPass();
	RenderSelectedOutlinePass();
	RenderParticlePass();
	RenderPostProcessPass();
	RenderDebugPass();
	m_pDearsGraphicsEngine->EndRender();
}

void GameEngine::HandleRenderControls()
{
	const bool blockEngineKeyboardInput = ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantTextInput;
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
	// 이 오버레이는 엔진 핵심 렌더링이 아니라 데모/디버그 표시용 UI다.
	// FPS, 마우스 좌표, 카메라 벡터, 라이트 view-projection 행렬처럼
	// GameEngine이 들고 있는 상태를 읽기 때문에 지금은 여기에 남겨둔다.
	//
	// 나중에 목표로 삼을 분리 방향:
	// - 사용자가 직접 조작하는 에디터 패널은 editor layer로 이동
	// - 임시 디버그 숫자는 DebugOverlay 같은 클래스로 이동
	// - DX11/DX12/Vulkan 백엔드는 이런 UI 내용 자체를 몰라도 되게 만들기
	RenderContext uiContext;
	uiContext.passType = RenderPassType::Ui;
	m_pDearsGraphicsEngine->ApplyRenderContext(uiContext);

	// 현재 샘플 화면을 보여주기 위한 데모 이미지들이다.
	m_pDearsGraphicsEngine->UIDrawImageStart();
	m_pDearsGraphicsEngine->UIDrawImage(Vector2(1720, 20), Vector2(200, 200), "coco.jpg", Vector4(0.2f, 0.2f, 0.2f, 0.8f));
	m_pDearsGraphicsEngine->UIDrawImage(Vector2(0, 0), Vector2(200, 200), "coco.jpg");
	m_pDearsGraphicsEngine->UIDrawImage(m_pActiveScene->GetSecondaryUiPoint(), Vector2(200, 100), "startButton.png");
	m_pDearsGraphicsEngine->UIDrawImageFin();

	m_pDearsGraphicsEngine->UIStartFontID("NotoSansKR-Regular(48).ttf");
	ImGui::SetWindowFontScale(0.5f);
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

void GameEngine::RenderShadowPass()
{
	if (!m_hasShadowLight)
	{
		return;
	}
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
	m_pDearsGraphicsEngine->ApplyRenderContext(shadowContext);

	m_pRenderDispatcher->RenderShadowItems(m_pActiveScene->GetShadowRenderItems());
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
	RenderContext geometryContext;
	geometryContext.passType = RenderPassType::Geometry;
	geometryContext.camera = tempCamera.get();
	geometryContext.commonBuffer = &tempCCConstantBuffer;
	m_pDearsGraphicsEngine->ApplyRenderContext(geometryContext);

	// 우선 PBR 불투명 메시만 Geometry Pass에서 G-Buffer에 기록한다.
	// Lighting 구현 전에는 Forward Pass에서도 다시 그려 기존 화면을 보존한다.
	m_pRenderDispatcher->RenderGeometryItems(m_pActiveScene->GetMainRenderItems());
}

void GameEngine::RenderLightingPass()
{
	// Deferred Lighting pass:
	// Geometry pass가 모든 오브젝트의 G-Buffer 기록을 끝낸 뒤 딱 한 번 실행된다.
	// 이 단계에서는 개별 모델을 다시 그리지 않고 화면 전체 삼각형을 그리며,
	// G-Buffer와 shadow map을 읽어 각 화면 픽셀의 최종 조명 색상을 계산한다.
	//
	// 현재는 읽을 G-Buffer와 Lighting 셰이더가 아직 없으므로 패스 경계만 적용한다.
	// 다음 단계에서 G-Buffer SRV 바인딩과 fullscreen triangle draw를 연결한다.
	RenderContext lightingContext;
	lightingContext.passType = RenderPassType::Lighting;
	lightingContext.camera = tempCamera.get();
	lightingContext.commonBuffer = &tempCCConstantBuffer;
	m_pDearsGraphicsEngine->ApplyRenderContext(lightingContext);
}

void GameEngine::RenderForwardPass()
{
	// Forward pass:
	// 현재 엔진의 메시 셰이더는 오브젝트를 그리는 순간 조명 계산까지 끝내므로,
	// 기존 화면을 보존하기 위해 모든 씬 오브젝트는 아직 이 패스에서 렌더링한다.
	// G-Buffer가 완성되면 불투명 PBR 메시는 Geometry pass로 이동하고,
	// 투명 오브젝트, 물, 파티클과 일부 특수 셰이더만 Forward 경로에 남는다.
	RenderContext forwardContext;
	forwardContext.passType = RenderPassType::Forward;
	forwardContext.camera = tempCamera.get();
	forwardContext.commonBuffer = &tempCCConstantBuffer;
	m_pDearsGraphicsEngine->ApplyRenderContext(forwardContext);

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
	RenderContext outlineContext;
	outlineContext.passType = RenderPassType::SelectionOutline;
	outlineContext.camera = tempCamera.get();
	outlineContext.commonBuffer = &tempCCConstantBuffer;
	m_pDearsGraphicsEngine->ApplyRenderContext(outlineContext);

	m_pRenderDispatcher->RenderSelectedOutline(m_pActiveScene->GetSelectedObject(), tempCamera.get());
}

void GameEngine::RenderParticlePass()
{
	// Particle pass:
	// 파티클은 DearsGraphicsEngine::Update()에서 갱신되고 여기서 렌더링된다.
	// 파티클은 보통 blending, depth write/read 규칙이 일반 mesh와 다르기 때문에
	// 현대 렌더러에서는 별도 패스로 분리되는 경우가 많다.
	RenderContext particleContext;
	particleContext.passType = RenderPassType::Particle;
	particleContext.camera = tempCamera.get();
	particleContext.commonBuffer = &tempCCConstantBuffer;
	m_pDearsGraphicsEngine->ApplyRenderContext(particleContext);

	m_pDearsGraphicsEngine->RendParticle();
}

void GameEngine::RenderPostProcessPass()
{
	// 포스트 프로세싱은 현재 꺼져 있지만, 기존 테스트 키 동작을 보존하려고
	// 토글 구조는 남겨둔다.
	// 나중에 bloom, tone mapping, SSAO 같은 후처리 효과를 다시 붙이면
	// 이 함수에서 관련 패스를 호출하면 된다.
	RenderContext postProcessContext;
	postProcessContext.passType = RenderPassType::PostProcess;
	m_pDearsGraphicsEngine->ApplyRenderContext(postProcessContext);

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
		//m_pDearsGraphicsEngine->RendPostProcessing();
	}
}

void GameEngine::RenderDebugPass()
{
	// Debug pass:
	// 메인 씬을 그린 뒤에 엔진 시각화용 helper geometry를 추가로 그린다.
	// 디버그 기하도 일반 장면 렌더링과 같은 카메라를 사용해야 월드 좌표와 정확히 겹친다.
	// 아래 RenderContext를 적용한 뒤 라이트 기즈모를 그리면 현재 뷰/투영 행렬이
	// DebugLineVertexShader에도 전달되므로, 카메라가 움직여도 선이 라이트 위치를 따라간다.
	RenderContext debugContext;
	debugContext.passType = RenderPassType::Debug;
	debugContext.camera = tempCamera.get();
	debugContext.commonBuffer = &tempCCConstantBuffer;
	m_pDearsGraphicsEngine->ApplyRenderContext(debugContext);

	// Scene에 실제로 배치된 라이트만 기즈모로 표시한다.
	// 기즈모는 Debug Pass에서 그리므로 G-Buffer, Shadow Map, Picking 대상에는 포함되지 않는다.
	for (UINT lightIndex = 0; lightIndex < tempCCConstantBuffer.lightNum; ++lightIndex)
	{
		const Light& light = tempCCConstantBuffer.light[lightIndex];
		const bool drawDirectionalShadowFrustum =
			m_hasShadowLight &&
			lightIndex == 0 &&
			light.lightType == static_cast<UINT>(LightEnum::DIRECTIONAL_LIGHT);

		m_pDearsGraphicsEngine->Rend_DebugLightGizmo(
			light,
			drawDirectionalShadowFrustum,
			lightCamera->mProjFovAngleY,
			lightCamera->GetAspectRatio(),
			lightCamera->mNearZ,
			lightCamera->mFarZ);
	}
}

void GameEngine::GameLoop()
{
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
