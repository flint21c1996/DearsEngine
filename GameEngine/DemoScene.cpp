#include "DemoScene.h"

#include "DearsGraphicsEngine.h"
#include "EasingFunc.h"
#include "InputManager.h"

namespace
{
constexpr size_t CharacterIndex = 0;
constexpr size_t WeaponIndex = 1;
constexpr size_t BillboardIndex = 2;
constexpr size_t PbrSphereIndex = 3;
constexpr size_t FloorIndex = 4;
constexpr size_t WaterIndex = 5;
constexpr size_t TerrainIndex = 6;
constexpr size_t CubeMapIndex = 7;
}

DemoScene::DemoScene(DearsGraphicsEngine* graphicsEngine)
	: m_pGraphicsEngine(graphicsEngine)
{
}

void DemoScene::Initialize()
{
	// Scene initialization only creates scene-local objects.
	// Engine-wide systems such as the renderer or cameras stay in GameEngine.
	CreateSceneObjects();
	CreateRenderItems();
}

void DemoScene::RegisterEditorPanels(int screenWidth)
{
	// The hierarchy and inspector panels belong to the demo world layout,
	// so they are registered from the scene instead of the engine core.
	// Panel width is derived from the window width so the render area keeps
	// a stable 5:1 proportion against the editor lane.
	m_selectedObjectIndex = 0;
	m_pScenePanel = std::make_unique<SceneHierarchyPanel>(m_objects, m_selectedObjectIndex, m_pGraphicsEngine);
	m_pScenePanel->RegisterName("Character");
	m_pScenePanel->RegisterName("Weapon");
	m_pScenePanel->RegisterName("Billboard");
	m_pScenePanel->RegisterName("PBR Sphere");
	m_pScenePanel->RegisterName("Floor");
	m_pScenePanel->RegisterName("Water");
	m_pScenePanel->RegisterName("Terrain");
	m_pScenePanel->RegisterName("CubeMap");

	m_pInspectorPanel = std::make_unique<ObjectInspectorPanel>(m_objects, m_selectedObjectIndex);
	m_pGraphicsEngine->AddEditorPanel(m_pScenePanel.get());
	m_pGraphicsEngine->AddEditorPanel(m_pInspectorPanel.get());

	const int editorPanelWidth = GetEditorPanelWidth(static_cast<float>(screenWidth));
	m_pGraphicsEngine->SetRenderViewportWidth(screenWidth - editorPanelWidth);
}

void DemoScene::Update(float deltaTime)
{
	// Character and attached equipment are updated together so the weapon
	// follows the animated character pose inside the scene.
	GetCharacter()->UpdateAnimationTime(deltaTime);
	GetCharacter()->Update();

	GetWeapon()->SetObjectPos(
		m_pGraphicsEngine->GetTargetBoneAboveMatrix("Character 01.FBX", GetWeapon()->mTargetBoneIndex, 0.1f));
	GetWeapon()->Update();

	GetBillboard()->Update();
	GetPbrSphere()->Update();
	GetFloor()->Update();

	GetWater()->mVSWaterConstantBufferData.time += deltaTime;
	GetWater()->Update();

	GetTerrain()->Update();
	GetCubeMap()->Update();
}

void DemoScene::HandleDemoInput(InputManager& inputManager)
{
	// These controls exist only to test the current sample scene.
	// They should not live in the engine core.
	if (inputManager.GetKeyState(KEY::W) == KEY_STATE::HOLD)
	{
		m_particleSpawnPos.z += 0.1f;
	}
	if (inputManager.GetKeyState(KEY::A) == KEY_STATE::HOLD)
	{
		m_particleSpawnPos.x -= 0.1f;
	}
	if (inputManager.GetKeyState(KEY::S) == KEY_STATE::HOLD)
	{
		m_particleSpawnPos.z -= 0.1f;
	}
	if (inputManager.GetKeyState(KEY::D) == KEY_STATE::HOLD)
	{
		m_particleSpawnPos.x += 0.1f;
	}

	if (inputManager.GetKeyState(KEY::F) == KEY_STATE::TAP)
	{
		SpawnDemoParticle();
	}

	if (inputManager.GetKeyState(KEY::_1) == KEY_STATE::HOLD)
	{
		m_pGraphicsEngine->Set_CubeMap("MyCube1EnvHDR.dds", "MyCube1DiffuseHDR.dds", "MyCube1SpecularHDR.dds", "MyCube1Brdf.dds");
	}
	if (inputManager.GetKeyState(KEY::_2) == KEY_STATE::HOLD)
	{
		m_pGraphicsEngine->Set_CubeMap("MyCube2EnvHDR.dds", "MyCube2DiffuseHDR.dds", "MyCube2SpecularHDR.dds", "MyCube2Brdf.dds");
	}
	if (inputManager.GetKeyState(KEY::_3) == KEY_STATE::HOLD)
	{
		m_pGraphicsEngine->Set_CubeMap("MyCube3EnvHDR.dds", "MyCube3DiffuseHDR.dds", "MyCube3SpecularHDR.dds", "MyCube3Brdf.dds");
	}
}

void DemoScene::HandlePresentationInput(InputManager& inputManager, EasingFunc& easingFunc, float deltaTime)
{
	// Presentation controls tweak rendering values for the sample scene.
	// They are intentionally separate from the engine-level camera controls.
	if (inputManager.GetKeyState(KEY::I) == KEY_STATE::HOLD)
	{
		GetBillboard()->mPSConstantBuffer.mipmapLevel += 0.1f;
	}
	if (inputManager.GetKeyState(KEY::O) == KEY_STATE::HOLD)
	{
		GetBillboard()->mPSConstantBuffer.mipmapLevel -= 0.1f;
	}

	if (inputManager.GetKeyState(KEY::_4) == KEY_STATE::HOLD)
	{
		m_uiTweenTime += deltaTime;
		easingFunc.EasingFuncMap["easeInOutBack"](1600, 300, m_uiTweenTime, &m_uiPoint.x);
		easingFunc.EasingFuncMap["easeOutBounce"](1600, 300, m_uiTweenTime, &m_uiPoint2.x);
	}
	if (inputManager.GetKeyState(KEY::_5) == KEY_STATE::HOLD)
	{
		m_uiTweenTime = 0.f;
		m_uiPoint.x = 1600.f;
		m_uiPoint2.x = 1600.f;
	}

	if (inputManager.GetKeyState(KEY::_9) == KEY_STATE::HOLD)
	{
		for (float& value : m_opacityValue)
		{
			value -= 0.01f;
		}
		m_pGraphicsEngine->SetOpacityFactor(m_opacityValue.data());
		GetPbrSphere()->mPSThinFilmConstantBufferData.time += 0.001f;
	}

	if (inputManager.GetKeyState(KEY::_0) == KEY_STATE::HOLD)
	{
		for (float& value : m_opacityValue)
		{
			value += 0.01f;
		}
		m_pGraphicsEngine->SetOpacityFactor(m_opacityValue.data());
		GetPbrSphere()->mPSThinFilmConstantBufferData.time -= 0.001f;
	}
}

void DemoScene::HandleRenderInput(InputManager& inputManager)
{
	// These keys directly manipulate the sample PBR object during rendering tests.
	if (inputManager.GetKeyState(KEY::U) == KEY_STATE::HOLD)
	{
		m_pbrSphereRotationY += 0.01f;
		GetPbrSphere()->SetObjectRot(Matrix::CreateRotationY(m_pbrSphereRotationY));
	}

	if (inputManager.GetKeyState(KEY::K) == KEY_STATE::HOLD)
	{
		m_pbrHeightScale += 0.01f;
		GetPbrSphere()->mVSPBRConstantBufferData.heightScale = m_pbrHeightScale;
	}
	if (inputManager.GetKeyState(KEY::L) == KEY_STATE::HOLD)
	{
		m_pbrHeightScale -= 0.01f;
		GetPbrSphere()->mVSPBRConstantBufferData.heightScale = m_pbrHeightScale;
	}
}

RenderObject* DemoScene::GetCharacter() const
{
	return GetObject(CharacterIndex);
}

RenderObject* DemoScene::GetWeapon() const
{
	return GetObject(WeaponIndex);
}

RenderObject* DemoScene::GetBillboard() const
{
	return GetObject(BillboardIndex);
}

RenderObject* DemoScene::GetPbrSphere() const
{
	return GetObject(PbrSphereIndex);
}

RenderObject* DemoScene::GetFloor() const
{
	return GetObject(FloorIndex);
}

RenderObject* DemoScene::GetWater() const
{
	return GetObject(WaterIndex);
}

RenderObject* DemoScene::GetTerrain() const
{
	return GetObject(TerrainIndex);
}

RenderObject* DemoScene::GetCubeMap() const
{
	return GetObject(CubeMapIndex);
}

const std::vector<SceneRenderItem>& DemoScene::GetShadowRenderItems() const
{
	return m_shadowRenderItems;
}

const std::vector<SceneRenderItem>& DemoScene::GetMainRenderItems() const
{
	return m_mainRenderItems;
}

const Vector2& DemoScene::GetPrimaryUiPoint() const
{
	return m_uiPoint;
}

const Vector2& DemoScene::GetSecondaryUiPoint() const
{
	return m_uiPoint2;
}

void DemoScene::SpawnDemoParticle()
{
	// The particle template is part of the demo scene setup.
	// Keeping it here avoids leaking sample-only effect data into GameEngine.
	CSParticleData particle;
	particle.position = m_particleSpawnPos;
	particle.playTime = 0;
	particle.color = { 1,1,1 };
	particle.lifeTime = 10;
	particle.direction = { 0,0,0 };
	particle.velocity = 20.0f;
	particle.Rotation = { 0, 0, 0 };
	particle.opacity = 1;
	particle.Scale = { 1,1,1 };
	particle.deltaOpacity = -0.1f;
	particle.deltaRotation = { 0,0,1 };
	particle.SclRandomFactor = 0;
	particle.deltaScale = { 0.f, 0.f, 0.f };
	particle.PosRandomFactor = { 0, 0, 0 };
	particle.RotRandomFactor = { 0, 0, 0 };
	particle.dirRandomFactor = { 0, 0, 0 };
	particle.deltaColor = { 0, 0, 0 };
	particle.gravity = 0;
	particle.colorRandomFactor = { 0, 0, 0 };

	m_pGraphicsEngine->m_pParticleManager->AddParticle(1, particle);
}

RenderObject* DemoScene::GetObject(size_t index) const
{
	if (index >= m_objects.size())
	{
		return nullptr;
	}

	return m_objects[index].get();
}

void DemoScene::CreateSceneObjects()
{
	// This method defines the sample scene layout.
	// The order is fixed because the rest of the scene refers to these objects by role.
	m_objects.clear();

	m_objects.push_back(std::make_unique<RenderObject>(m_pGraphicsEngine));
	GetCharacter()->Initialize();
	GetCharacter()->CreateVSConstantBuffer();
	GetCharacter()->CreateVSBoneConstantBuffer();
	GetCharacter()->CreatePSConstantBuffer();
	GetCharacter()->SetVIBuffer("Character 01");
	GetCharacter()->SetDiffuseTexture("M-Cos-16-Blue.png");
	GetCharacter()->SetModelInfo("Character 01.FBX");
	GetCharacter()->SetAnimation("Character@Slash Attack.FBX");
	GetCharacter()->SetObjectScl(Matrix::CreateScale(0.1));

	m_objects.push_back(std::make_unique<RenderObject>(m_pGraphicsEngine));
	GetWeapon()->Initialize();
	GetWeapon()->CreateVSConstantBuffer();
	GetWeapon()->CreatePSConstantBuffer();
	GetWeapon()->SetVIBuffer("Axe 01");
	GetWeapon()->SetModelInfo("Axe 01.fbx");
	GetWeapon()->SetDiffuseTexture("ss.png");
	GetWeapon()->SetTargetBoneIndex(m_pGraphicsEngine->Get_TargetModelBoneIndex("Character 01", "RigRPalm"));
	GetWeapon()->GetObjectTargetBoneMatrix(GetCharacter()->mpVSBoneConstantBufferData);

	m_objects.push_back(std::make_unique<RenderObject>(m_pGraphicsEngine));
	GetBillboard()->Initialize();
	GetBillboard()->CreateVSConstantBuffer();
	GetBillboard()->CreateVSBoneConstantBuffer();
	GetBillboard()->CreatePSConstantBuffer();
	GetBillboard()->SetVIBuffer("BillBoardSquare");
	GetBillboard()->SetDiffuseTexture("pngegg.png");
	GetBillboard()->SetObjectScl(Matrix::CreateScale(5));
	GetBillboard()->SetObjectPos(Matrix::CreateTranslation({ 10,3,-10 }));

	m_objects.push_back(std::make_unique<RenderObject>(m_pGraphicsEngine));
	GetPbrSphere()->Initialize();
	GetPbrSphere()->CreateVSPBRConstantBuffer();
	GetPbrSphere()->CreatePSPBRConstantBuffer();
	GetPbrSphere()->CreatePSThinFilmConstantBuffer();
	GetPbrSphere()->SetPBRTextures(
		"albedo.png",
		"normal.png",
		"ao.png",
		"metallic.png",
		"roughness.png",
		"height.png"
	);
	GetPbrSphere()->mPSThinFilmConstantBufferData.useAlbedoMap = 1;
	GetPbrSphere()->SetVIBuffer("MySphere");
	GetPbrSphere()->SetModelInfo("Hat 04.FBX");
	GetPbrSphere()->SetDiffuseTexture("albedo.png");
	GetPbrSphere()->SetTargetBoneIndex(m_pGraphicsEngine->Get_TargetModelBoneIndex("Character 01", "RigHead"));
	GetPbrSphere()->GetObjectTargetBoneMatrix(GetCharacter()->mpVSBoneConstantBufferData);
	GetPbrSphere()->SetObjectRot(Matrix::CreateRotationX(1.570));
	GetPbrSphere()->SetObjectScl(Matrix::CreateScale(5));
	GetPbrSphere()->SetObjectPos(Matrix::CreateTranslation({ 0,10, 0 }));

	m_objects.push_back(std::make_unique<RenderObject>(m_pGraphicsEngine));
	GetFloor()->Initialize();
	GetFloor()->CreateVSConstantBuffer();
	GetFloor()->CreatePSConstantBuffer();
	GetFloor()->SetVIBuffer("MySquare");
	GetFloor()->SetDiffuseTexture("White.png");
	GetFloor()->SetObjectScl(Matrix::CreateScale(5, 5, 5));
	GetFloor()->SetObjectPos(Matrix::CreateTranslation({ 0,0,0 }));
	GetFloor()->CreateVSEdgeConstantBuffer();
	GetFloor()->mVSEdgeConstantBufferData.EdgeScaleMatrix = Matrix::CreateScale(1.01);
	GetFloor()->mPSEdgeConstantBufferData.color = { 1,1,1 };
	GetFloor()->CreatePSEdgeConstantBuffer();

	m_objects.push_back(std::make_unique<RenderObject>(m_pGraphicsEngine));
	GetWater()->Initialize();
	GetWater()->CreateVSConstantBuffer();
	GetWater()->CreatePSConstantBuffer();
	GetWater()->CreateVSInstanceConstantBuffer();
	GetWater()->SetVIBuffer("MyBox");
	GetWater()->SetDiffuseTexture("Water.png");
	GetWater()->SetObjectScl(Matrix::CreateScale(5,5,5));
	GetWater()->SetObjectPos(Matrix::CreateTranslation({ 5,-20, 0 }));
	GetWater()->CreateVSWaterConstantBuffer();
	GetWater()->mVSWaterConstantBufferData.time = 0.0f;
	GetWater()->mVSWaterConstantBufferData.speed = 0.1f;

	m_objects.push_back(std::make_unique<RenderObject>(m_pGraphicsEngine));
	GetTerrain()->Initialize();
	GetTerrain()->CreateVSConstantBuffer();
	GetTerrain()->CreatePSConstantBuffer();
	GetTerrain()->SetVIBuffer("MySquare");
	GetTerrain()->SetDiffuseTexture("ss.png");
	GetTerrain()->SetObjectScl(Matrix::CreateScale(200));
	GetTerrain()->SetObjectPos(Matrix::CreateTranslation({ 0,-25, 0 }));

	m_objects.push_back(std::make_unique<RenderObject>(m_pGraphicsEngine));
	GetCubeMap()->Initialize();
	GetCubeMap()->CreateVSConstantBuffer();
	GetCubeMap()->CreatePSConstantBuffer();
	GetCubeMap()->SetVIBuffer("CubeMap");
	GetCubeMap()->SetObjectScl(Matrix::CreateScale(1));
	GetCubeMap()->SetObjectPos(Matrix::CreateTranslation({ 0,0,0 }));
}

void DemoScene::CreateRenderItems()
{
	// 렌더 아이템 목록은 "이 씬의 어떤 오브젝트를 어떤 방식으로 그릴지"를
	// GameEngine에 알려주는 얇은 명세다.
	// GameEngine은 이제 Terrain, Floor 같은 데모 전용 이름을 몰라도 되고,
	// 각 패스에 들어갈 RenderObject와 렌더 타입만 보고 처리한다.
	m_shadowRenderItems.clear();
	m_mainRenderItems.clear();

	// 기존 shadow pass 순서를 그대로 보존한다.
	// Terrain은 이름만 지형이고 실제로는 크게 키운 MySquare지만,
	// GameEngine 입장에서는 그냥 StaticMesh shadow caster일 뿐이다.
	m_shadowRenderItems.push_back({ GetTerrain(), SceneRenderType::StaticMesh });
	m_shadowRenderItems.push_back({ GetFloor(), SceneRenderType::StaticMesh });
	m_shadowRenderItems.push_back({ GetWater(), SceneRenderType::StaticMesh });
	m_shadowRenderItems.push_back({ GetCharacter(), SceneRenderType::SkinnedMesh });
	m_shadowRenderItems.push_back({ GetWeapon(), SceneRenderType::StaticMesh });
	m_shadowRenderItems.push_back({ GetBillboard(), SceneRenderType::StaticMesh });
	m_shadowRenderItems.push_back({ GetPbrSphere(), SceneRenderType::EquipmentMesh });

	// 기존 main scene pass 순서를 그대로 보존한다.
	m_mainRenderItems.push_back({ GetCubeMap(), SceneRenderType::CubeMap });
	m_mainRenderItems.push_back({ GetBillboard(), SceneRenderType::Billboard });
	m_mainRenderItems.push_back({ GetPbrSphere(), SceneRenderType::PbrMesh });
}
