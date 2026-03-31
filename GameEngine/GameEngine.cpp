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

	// Rendering still lives in GameEngine, but scene-specific debug input
	// is applied by the scene before draw calls begin.
	m_pActiveScene->HandleRenderInput(*m_pInputManager);

	if (m_pInputManager->GetKeyState(KEY::Z) == KEY_STATE::TAP)
	{
		tempCamera->SetPerspective();
	}
	if (m_pInputManager->GetKeyState(KEY::X) == KEY_STATE::TAP)
	{
		tempCamera->SetOrthgraphic(0.1f);
	}

	m_pDearsGraphicsEngine->BeginRender();

	// These images are part of the current sample presentation layer.
	m_pDearsGraphicsEngine->UIDrawImageStart();
	m_pDearsGraphicsEngine->UIDrawImage(Vector2(1720, 20), Vector2(200, 200), "coco.jpg", Vector4(0.2f, 0.2f, 0.2f, 0.8f));
	m_pDearsGraphicsEngine->UIDrawImage(Vector2(0, 0), Vector2(200, 200), "coco.jpg");
	m_pDearsGraphicsEngine->UIDrawImage(m_pActiveScene->GetSecondaryUiPoint(), Vector2(200, 100), "startButton.png");
	m_pDearsGraphicsEngine->UIDrawImageFin();
	//m_pDearsGraphicsEngine->UIFreeRectFilled(Vector2(100, 100), Vector2(200, 100), Vector2(300, 500), Vector2(10, 500), Vector4(1.0f, 1.0f, 0.0f, 1.0f));		//占썹각占쏙옙占쏙옙 占쌓몌옙占쏙옙.
	//m_pDearsGraphicsEngine->UIFreeRectwithBorder(Vector2(500, 100), Vector2(600, 100), Vector2(700, 500), Vector2(400, 500), Vector4(1.0f, 1.0f, 0.0f, 1.0f), 10.0f, Vector4(0.0f, 1.0f, 0.0f, 1.0f));		//占썹각占쏙옙占쏙옙 占쌓몌옙占쏙옙.
	//m_pDearsGraphicsEngine->UIFreeRect(Vector2(100, 100), Vector2(200, 100), Vector2(300, 500), Vector2(10, 500), Vector4(0.0f, 1.0f, 0.0f, 1.0f), 10.0f);		//占썹각占쏙옙占쏙옙 占쌓몌옙占쏙옙.
	//m_pDearsGraphicsEngine->UIDrawLine(Vector2(110, 110), Vector2(160, 160), Vector4(1.0f, 0.0f, 0.0f, 1.0f));	//占쏙옙占쏙옙占쏙옙 占쌓몌옙占쏙옙.
	//m_pDearsGraphicsEngine->UIDrawCir(Vector2(150, 150), 20.0f, Vector4(0.0f, 0.0f, 1.0f, 1.0f));				//占쏙옙占쏙옙 占쌓몌옙占쏙옙.

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



	//m_pDearsGraphicsEngine->UIStartFontID("B.ttf");	//占쏙옙占쏙옙占쏙옙 占쏙옙트 占쏙옙占쏙옙
	//m_pDearsGraphicsEngine->UIDrawText(Vector2(100, 900), u8"占쏙옙占쏙옙占쌘뱄옙占쏙옙占쏙옙화占쏙옙占신э옙占쏙옙占폙.ttf / 40.0f", Vector4(0.1f, 0.3f, 0.0f, 1.0f));
	//m_pDearsGraphicsEngine->UIFinFontID();			//占쏙옙 占쏙옙占쏙옙占쏙옙占?호占쏙옙

	//m_pDearsGraphicsEngine->mpRenderer->RenderDepthMap(tempObject1->GetModelBuffer());
	//m_pDearsGraphicsEngine->mpRenderer->RenderDepthMap(tempObject2->GetModelBuffer());
	///m_pDearsGraphicsEngine->SetCamera(lightCamera); 占쏙옙占쏙옙트 카占쌨띰옙 Set, CCC占쏙옙占쏙옙占쏙옙트占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙占쌔댐옙.

	//혹占쏙옙 占쏙옙占쏙옙 카占쌨라에쇽옙 占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙占? 	m_pDearsGraphicsEngine->SetCamera(lightCamera.get());
 	m_pDearsGraphicsEngine->UpdateCommonConstantBuffer(tempLightCConstantBuffer);
	m_pDearsGraphicsEngine->RenderDepthMap(m_pActiveScene->GetTerrain()->GetModelBuffer());
	m_pDearsGraphicsEngine->RenderDepthMap(m_pActiveScene->GetFloor()->GetModelBuffer());
	m_pDearsGraphicsEngine->RenderDepthMap(m_pActiveScene->GetWater()->GetModelBuffer());
	m_pDearsGraphicsEngine->RenderAniDepthMap(m_pActiveScene->GetCharacter()->GetModelBuffer());
	m_pDearsGraphicsEngine->RenderDepthMap(m_pActiveScene->GetWeapon()->GetModelBuffer());
	m_pDearsGraphicsEngine->RenderDepthMap(m_pActiveScene->GetBillboard()->GetModelBuffer());
	m_pDearsGraphicsEngine->RenderEquipDepthMap(m_pActiveScene->GetPbrSphere()->GetModelBuffer());

	m_pDearsGraphicsEngine->SetCamera(tempCamera.get());
	m_pDearsGraphicsEngine->UpdateCommonConstantBuffer(tempCCConstantBuffer);

	//m_pDearsGraphicsEngine->Rend_InstancedModels(tempObject6->GetModelBuffer());
 	m_pDearsGraphicsEngine->Rend_CubeMap(m_pActiveScene->GetCubeMap()->GetModelBuffer());

	m_pDearsGraphicsEngine->Rend_BillBoard(m_pActiveScene->GetBillboard()->GetModelBuffer());

	m_pDearsGraphicsEngine->Rend_PBR(m_pActiveScene->GetPbrSphere()->GetModelBuffer());


 	//m_pDearsGraphicsEngine->Rend_Model(tempObject5->GetModelBuffer());
 //	m_pDearsGraphicsEngine->Rend_Water(tempObject6->GetModelBuffer());
 //	m_pDearsGraphicsEngine->Rend_Model(tempObject7->GetModelBuffer());

///edge
// 	tempObject5->Update();
//	m_pDearsGraphicsEngine->Rend_EdgeModel(tempObject5->GetModelBuffer());
///Opacity
//m_pDearsGraphicsEngine->Rend_OpacitiyModel(tempObject5->GetModelBuffer());

//m_pDearsGraphicsEngine->Rend_Shadow(tempObject5->GetModelBuffer());
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempObject2->mpModelBuffer->mpTargetModel->mMeshData->mAABB,
// 		Matrix::CreateScale({ 1,1,1 }),
// 		Matrix::CreateRotationX(0) * Matrix::CreateRotationX(0) * Matrix::CreateRotationX(0),
// 		m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigRPalm", 0.1f));

	//m_pDearsGraphicsEngine->Rend_DebugBox(tempObject2->mpModelBuffer->mpTargetModel->mMeshData->mAABB, tempObject2->ObjectScl, tempObject2->ObjectRot);

	Vector3 _size = { 0.1f,0.1f,0.1f };
	//m_pDearsGraphicsEngine->Rend_DebugBox({30,10,50 }, { 0,0,0 }, {0,0,0 }, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigRPalm", 0.1f));

 	Matrix tempScl = Matrix::CreateScale({ 3,1,5 });
 	Matrix tempRot = Matrix::CreateRotationX(0) * Matrix::CreateRotationX(0) * Matrix::CreateRotationX(0);
 	Matrix tempPos = Matrix::CreateTranslation({ 0,0,0 });
	//m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos);
	m_pDearsGraphicsEngine->RendParticle();
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

	///占쏙옙占쏙옙 占쏙옙처占쏙옙 占쏙옙占싶몌옙 占쏙옙占쏙옙占쌀쇽옙 占쌍댐옙. DearsGame占쏙옙占쏙옙占쏙옙占쏙옙 占싱뤄옙占쏙옙 占싸븝옙占쏙옙 占쏙옙占쏙옙摸占?. EndRender占싫울옙 占쌍댐옙 占식쇽옙占쏙옙 占쏙옙占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙?

	

 //	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, Matrix::CreateTranslation({ 0,0,-35 }));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, Matrix::CreateTranslation({ 0,0,0 }));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigRThigh", 0.1f));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigPelvis", 0.1f));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigLThigh", 0.1f));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigRCollarbone", 0.1f));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigRibcage", 0.1f));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigHead", 0.1f));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigSpine2", 0.1f));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigRUpperarm", 0.1f));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigRForearm", 0.1f));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigSpine1", 0.1f));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigLFoot", 0.1f));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigLToes1", 0.1f));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigLCalf", 0.1f));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigLPalm", 0.1f));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigLCollarbone", 0.1f));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigLUpperarm", 0.1f));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigLForearm", 0.1f));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigPelvis", 0.1f));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigLThigh", 0.1f));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigLCalf", 0.1f));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigLFoot", 0.1f));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigLToes1", 0.1f));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigRThigh", 0.1f));
// 	m_pDearsGraphicsEngine->Rend_DebugBox(tempScl, tempRot, tempPos, m_pDearsGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", "RigRCalf", 0.1f));

	//m_pDearsGraphicsEngine->Rend_DebugBox({30,10,20 }, { 0,0,0 }, {0,0,0 }, Matrix::CreateTranslation({ 10,0,0 }));


	//m_pDearsGraphicsEngine->Rend_DebugSphere({ 1,1,1 }, { 0,0,0 }, Vector3(2.0f, 0.0f, 0.0f));
	m_pDearsGraphicsEngine->Rend_DebugSphere({ 1.f,1.f,1.f }, { 0,0,0 }, tempCCConstantBuffer.light[0].position);
// 	m_pDearsGraphicsEngine->Rend_DebugSphere({ 1.f,1.f,1.f }, { 0,0,0 }, tempCCConstantBuffer.light[1].position);
// 	m_pDearsGraphicsEngine->Rend_DebugSphere({ 1.f,1.f,1.f }, { 0,0,0 }, tempCCConstantBuffer.light[2].position);
	//m_pDearsGraphicsEngine->Rend_DebugCapsule({ 2,2,2 }, { 0,0,0 }, { -4,0,0 });
	m_pDearsGraphicsEngine->EndRender();
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
