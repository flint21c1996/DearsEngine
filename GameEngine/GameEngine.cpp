#include "GameEngine.h"
#include "TimeManager.h"
#include "InputManager.h"
#include "FileManager.h"
#include "Camera.h"
#include "DemoScene.h"
#include "IScene.h"
#include <math.h>
#include "Pool.h"

///占쏙옙占쏙옙占?占쌓쏙옙트
 
Vector3 dir = { 1.f, -1.f, 1.f };

GameEngine::GameEngine(HWND _hWnd, const int _screenWidth, const int _screenHeight)
{
	m_hWnd = _hWnd;
	m_screenWidth = _screenWidth;
	m_screenHeight = _screenHeight;
	m_pTimeManager = nullptr;
	m_pInputManager = nullptr;
	m_pFileManager = std::make_unique<FileManager>();
	m_pDearsGraphicsEngine = std::make_unique<DearsGraphicsEngine>(m_hWnd, m_screenWidth, m_screenHeight);
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
	lightCamera->SetEyePos(Vector3(-50.f, 50.0f, -50.f));
	lightCamera->SetDirection(Vector3(1.0f, -1.0f, 1.0f));
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
	// Lighting stays in GameEngine because it is part of the renderer setup.
	// Scene-local UI positions now live inside DemoScene instead of here.
	m_pDearsGraphicsEngine->LightInitialize(&tempCCConstantBuffer, 3);

	Vector3 lightDirection = { 1.f, -1.f, 1.f };
	lightDirection.Normalize();

	m_pDearsGraphicsEngine->SetDirLight(&tempCCConstantBuffer, 0, 1.f, lightDirection);
	tempCCConstantBuffer.light[0].position = lightCamera->GetmViewPos();
	lightCamera->SetDirection(lightDirection);
	lightCamera->SetEyePos(tempCCConstantBuffer.light[0].position);

	m_pDearsGraphicsEngine->Set_CubeMap("MyCube1EnvHDR.dds", "MyCube1DiffuseHDR.dds", "MyCube1SpecularHDR.dds", "MyCube1Brdf.dds");
}

void GameEngine::Update()
{
	// Update keeps engine-level flow in one place:
	// input, camera, demo controls, lighting, scene objects, presentation, renderer.
	m_pTimeManager->Tick();
	const float deltaTime = m_pTimeManager->DeltaTime();

	UpdateInputState();
	UpdateCameraControls(deltaTime);
	UpdateDemoControls();
	UpdateLightingState();
	UpdateSceneObjects(deltaTime);
	UpdatePresentationControls();
	m_pDearsGraphicsEngine->Update();
}

void GameEngine::UpdateInputState()
{
	m_pInputManager->Update();
	tempCamera->OnMouseMove(static_cast<int>(m_pInputManager->GetMousePos().x), static_cast<int>(m_pInputManager->GetMousePos().y));
	lightCamera->OnMouseMove(static_cast<int>(m_pInputManager->GetMousePos().x), static_cast<int>(m_pInputManager->GetMousePos().y));
}

void GameEngine::UpdateCameraControls(float deltaTime)
{
	if (m_pInputManager->GetKeyState(KEY::T) == KEY_STATE::TAP)
	{
		tempCamera->mIsFirstPersonMode = !tempCamera->mIsFirstPersonMode;
	}

	if (m_pInputManager->GetKeyState(KEY::Y) == KEY_STATE::TAP)
	{
		lightCamera->mIsFirstPersonMode = !lightCamera->mIsFirstPersonMode;
	}

	if (m_pInputManager->GetKeyState(KEY::UP) == KEY_STATE::HOLD)
	{
		tempCamera->MoveForward(deltaTime);
	}
	if (m_pInputManager->GetKeyState(KEY::DOWN) == KEY_STATE::HOLD)
	{
		tempCamera->MoveForward(-deltaTime);
	}
	if (m_pInputManager->GetKeyState(KEY::LEFT) == KEY_STATE::HOLD)
	{
		tempCamera->MoveRight(-deltaTime);
	}
	if (m_pInputManager->GetKeyState(KEY::RIGHT) == KEY_STATE::HOLD)
	{
		tempCamera->MoveRight(deltaTime);
	}
	if (m_pInputManager->GetKeyState(KEY::N) == KEY_STATE::HOLD)
	{
		tempCamera->MoveUp(deltaTime);
	}
	if (m_pInputManager->GetKeyState(KEY::M) == KEY_STATE::HOLD)
	{
		tempCamera->MoveUp(-deltaTime);
	}
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
	tempCCConstantBuffer.light[0].direction = lightCamera->mViewDir;
	m_pDearsGraphicsEngine->LightUpdate(&tempCCConstantBuffer);
	m_pDearsGraphicsEngine->LightUpdate(&tempLightCConstantBuffer);
	m_pDearsGraphicsEngine->SetCamera(lightCamera.get());
	tempCCConstantBuffer.light[0].viewProj = (lightCamera->GetViewRow() * lightCamera->GetProjRow()).Transpose();
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
	RenderScenePass();
	RenderParticleAndPostProcessPass();
	RenderDebugPass();
	m_pDearsGraphicsEngine->EndRender();
}

void GameEngine::HandleRenderControls()
{
	// 씬 전용 렌더 테스트 입력은 아직 ActiveScene이 처리한다.
	// 예를 들어 현재 DemoScene은 여기서 PBR 구를 회전시키거나
	// height scale 값을 조절한다.
	// 나중에는 이런 테스트 입력을 에디터 툴이나 컴포넌트 쪽으로 옮길 수 있다.
	m_pActiveScene->HandleRenderInput(*m_pInputManager);

	// Z/X 키는 메인 카메라의 투영 방식을 바꾼다.
	// 이 처리를 RenderScenePass()와 분리해두면,
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
	m_pDearsGraphicsEngine->SetCamera(lightCamera.get());
	m_pDearsGraphicsEngine->UpdateCommonConstantBuffer(tempLightCConstantBuffer);

	for (const SceneRenderItem& item : m_pActiveScene->GetShadowRenderItems())
	{
		RenderShadowItem(item);
	}
}

void GameEngine::RenderScenePass()
{
	// Main scene pass:
	// 라이트 카메라에서 다시 플레이어/에디터 카메라로 돌아온 뒤,
	// 공통 view/projection constant buffer를 갱신하고 실제 씬 오브젝트를 그린다.
	//
	// 이제 GameEngine은 Terrain, PBR Sphere 같은 DemoScene 전용 이름을
	// 직접 묻지 않고, 씬이 넘겨준 render item 목록만 순서대로 처리한다.
	// 아직은 renderType을 현재 DX11 렌더 함수로 매핑하지만,
	// 그 매핑도 나중에는 RHI 계층으로 내려가는 것이 목표다.
	m_pDearsGraphicsEngine->SetCamera(tempCamera.get());
	m_pDearsGraphicsEngine->UpdateCommonConstantBuffer(tempCCConstantBuffer);

	for (const SceneRenderItem& item : m_pActiveScene->GetMainRenderItems())
	{
		RenderMainItem(item);
	}
}

void GameEngine::RenderShadowItem(const SceneRenderItem& item)
{
	if (!item.object)
	{
		return;
	}

	// Shadow pass에서는 오브젝트의 "역할 이름"보다
	// 어떤 셰이더 경로로 depth를 그려야 하는지가 중요하다.
	// StaticMesh는 일반 depth, SkinnedMesh는 bone buffer가 필요한 depth,
	// EquipmentMesh는 target bone matrix가 필요한 depth 경로를 탄다.
	//
	// 지금은 여기서 DX11용 DearsGraphicsEngine 함수를 직접 부르지만,
	// RHI를 도입하면 이 함수가 "공통 shadow draw command"를 만드는
	// 위치로 바뀔 가능성이 높다.
	switch (item.renderType)
	{
	case SceneRenderType::SkinnedMesh:
		m_pDearsGraphicsEngine->RenderAniDepthMap(item.object->GetModelBuffer());
		break;
	case SceneRenderType::EquipmentMesh:
		m_pDearsGraphicsEngine->RenderEquipDepthMap(item.object->GetModelBuffer());
		break;
	default:
		m_pDearsGraphicsEngine->RenderDepthMap(item.object->GetModelBuffer());
		break;
	}
}

void GameEngine::RenderMainItem(const SceneRenderItem& item)
{
	if (!item.object)
	{
		return;
	}

	// Main pass에서는 씬이 넘긴 renderType에 따라 현재 DX11 렌더 함수를 고른다.
	// 이 switch는 지금 당장은 "렌더 타입 -> DX11 렌더 함수" 매핑이지만,
	// 나중에는 "렌더 타입 -> RHI pipeline / draw command" 매핑으로 옮겨갈 부분이다.
	switch (item.renderType)
	{
	case SceneRenderType::CubeMap:
		m_pDearsGraphicsEngine->Rend_CubeMap(item.object->GetModelBuffer());
		break;
	case SceneRenderType::Billboard:
		m_pDearsGraphicsEngine->Rend_BillBoard(item.object->GetModelBuffer());
		break;
	case SceneRenderType::PbrMesh:
		m_pDearsGraphicsEngine->Rend_PBR(item.object->GetModelBuffer());
		break;
	case SceneRenderType::SkinnedMesh:
		m_pDearsGraphicsEngine->Rend_AnimateModel(item.object->GetModelBuffer());
		break;
	case SceneRenderType::EquipmentMesh:
		m_pDearsGraphicsEngine->Rend_EquipmentModel(item.object->GetModelBuffer());
		break;
	default:
		m_pDearsGraphicsEngine->Rend_Model(item.object->GetModelBuffer());
		break;
	}
}

void GameEngine::RenderParticleAndPostProcessPass()
{
	// Particle pass:
	// 파티클은 DearsGraphicsEngine::Update()에서 갱신되고 여기서 렌더링된다.
	// 파티클은 보통 blending, depth write/read 규칙이 일반 mesh와 다르기 때문에
	// 현대 렌더러에서는 별도 패스로 분리되는 경우가 많다.
	m_pDearsGraphicsEngine->RendParticle();

	// 포스트 프로세싱은 현재 꺼져 있지만, 기존 테스트 키 동작을 보존하려고
	// 토글 구조는 남겨둔다.
	// 나중에 bloom, tone mapping, SSAO 같은 후처리 효과를 다시 붙이면
	// 이 함수에서 관련 패스를 호출하면 된다.
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
	// 현재는 첫 번째 라이트 위치만 구체로 표시한다.
	// 나중에는 DebugRenderer로 분리해서 bounding box, skeleton, physics shape,
	// nav path, picking ray 같은 디버그 표시를 토글할 수 있게 만들면 좋다.
	m_pDearsGraphicsEngine->Rend_DebugSphere({ 1.f,1.f,1.f }, { 0,0,0 }, tempCCConstantBuffer.light[0].position);
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
