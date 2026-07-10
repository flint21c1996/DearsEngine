#include "DemoScene.h"

#include "DearsGraphicsEngine.h"
#include "EasingFunc.h"
#include "InputManager.h"

#include <imgui.h>
#include <algorithm>

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

std::string MakeUniqueObjectName(const std::vector<std::string>& objectNames, const std::string& requestedName)
{
	// UI에서 같은 이름을 여러 번 입력해도 씬 계층 창에서는 각각의 오브젝트를
	// 구분할 수 있어야 한다. 그래서 실제 등록 직전에 "Name", "Name (1)",
	// "Name (2)" 형태로 빈 이름을 찾는다.
	const std::string baseName = requestedName.empty() ? "New Object" : requestedName;
	const auto nameExists = [&objectNames](const std::string& name)
		{
			return std::find(objectNames.begin(), objectNames.end(), name) != objectNames.end();
		};

	if (!nameExists(baseName))
	{
		return baseName;
	}

	int duplicateIndex = 1;
	std::string uniqueName;
	do
	{
		uniqueName = baseName + " (" + std::to_string(duplicateIndex) + ")";
		++duplicateIndex;
	} while (nameExists(uniqueName));

	return uniqueName;
}
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
	m_selectedObjectIndex = -1;
	m_pScenePanel = std::make_unique<SceneHierarchyPanel>(
		m_objects,
		m_objectNames,
		m_selectedObjectIndex,
		[this](const SceneObjectCreateDesc& desc)
		{
			CreateObjectFromDesc(desc);
		});

	m_pInspectorPanel = std::make_unique<ObjectInspectorPanel>(m_objects, m_selectedObjectIndex);
	m_pPickingPanel = std::make_unique<PickingPanel>(m_pickingManager);
	m_pGraphicsEngine->AddEditorPanel(m_pScenePanel.get());
	m_pGraphicsEngine->AddEditorPanel(m_pPickingPanel.get());
	m_pGraphicsEngine->AddEditorPanel(m_pInspectorPanel.get());

	const int editorPanelWidth = GetEditorPanelWidth(static_cast<float>(screenWidth));
	m_renderViewportWidth = screenWidth - editorPanelWidth;
	m_pGraphicsEngine->SetRenderViewportWidth(m_renderViewportWidth);
}

void DemoScene::Update(float deltaTime)
{
	// 예전 DemoScene은 "0번은 캐릭터, 1번은 무기, 2번은 빌보드..."처럼
	// 고정된 인덱스에 특정 오브젝트가 반드시 있다고 가정했다.
	//
	// 지금은 시작 씬을 비워두고 ImGui에서 오브젝트를 하나씩 추가하는 방향으로 바꾸는 중이므로,
	// Update도 더 이상 GetCharacter()/GetWeapon() 같은 데모 전용 helper에 의존하면 안 된다.
	// 현재 씬에 들어있는 RenderObject 목록을 순회하면서 존재하는 오브젝트만 갱신한다.
	for (const std::unique_ptr<RenderObject>& object : m_objects)
	{
		if (!object)
		{
			continue;
		}

		object->UpdateAnimationTime(deltaTime);
		object->Update();
	}
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

void DemoScene::HandlePickingInput(InputManager& inputManager, Camera* camera)
{
	// 피킹은 "마우스로 씬 오브젝트를 선택하는 입력"이다.
	// 선택된 인덱스는 SceneHierarchyPanel/ObjectInspectorPanel이 이미 공유하고 있으므로,
	// 여기서는 ray 검사 결과를 m_selectedObjectIndex에 반영하기만 하면 된다.
	//
	// ImGui가 마우스를 사용 중일 때는 피킹하지 않는다.
	// 예를 들어 Inspector의 DragFloat를 누른 상태에서 씬 선택이 같이 바뀌면
	// 에디터 조작이 굉장히 불편해지기 때문이다.
	if (ImGui::GetIO().WantCaptureMouse)
	{
		return;
	}

	if (inputManager.GetKeyState(KEY::LBUTTON) != KEY_STATE::TAP)
	{
		return;
	}

	const PickingResult result = m_pickingManager.Pick(
		m_objects,
		inputManager.GetMousePos(),
		m_renderViewportWidth,
		m_pGraphicsEngine->GetScreenHeight(),
		camera);

	if (result.hit)
	{
		m_selectedObjectIndex = result.objectIndex;
		return;
	}

	// 빈 공간을 클릭했을 때는 선택을 해제한다.
	// 선택이 남아 있으면 피킹이 실패했는데도 마지막 선택 오브젝트의 외곽선이 계속 그려져서,
	// 사용자는 "아무것도 안 맞았는데 무언가 선택된 상태"처럼 느끼게 된다.
	m_selectedObjectIndex = -1;
}

void DemoScene::HandlePresentationInput(InputManager& inputManager, EasingFunc& easingFunc, float deltaTime)
{
	if (m_objects.empty())
	{
		return;
	}

	// 아래 easing UI 테스트는 특정 오브젝트가 없어도 동작하는 순수 프레젠테이션 값이다.
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
}

void DemoScene::HandleRenderInput(InputManager& inputManager)
{
	RenderObject* selectedObject = GetSelectedObject();
	if (!selectedObject)
	{
		return;
	}

	// 이제 렌더 테스트 입력은 고정된 PBR 구가 아니라 현재 선택된 오브젝트에 적용한다.
	// Add Object 기반 편집 흐름에서는 "특정 인덱스의 특정 데모 오브젝트"가 항상 있다는 보장이 없기 때문이다.
	if (inputManager.GetKeyState(KEY::U) == KEY_STATE::HOLD)
	{
		m_pbrSphereRotationY += 0.01f;
		selectedObject->SetObjectRot(Matrix::CreateRotationY(m_pbrSphereRotationY));
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

RenderObject* DemoScene::GetSelectedObject() const
{
	if (m_selectedObjectIndex < 0)
	{
		return nullptr;
	}

	return GetObject(static_cast<size_t>(m_selectedObjectIndex));
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

	m_pGraphicsEngine->AddParticle(1, particle);
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
	// 지금은 화면에 기본으로 생성되는 데모 오브젝트를 모두 비활성화한다.
	//
	// 다음 목표:
	// - 시작 시에는 빈 씬으로 둔다.
	// - ImGui의 Add Object에서 모델/렌더 타입을 고른 뒤에만 m_objects에 추가한다.
	// - 오브젝트가 추가될 때 main/shadow render item도 함께 등록한다.
	m_objects.clear();
	m_objectNames.clear();

#if 0
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
#endif
}

void DemoScene::CreateObjectFromDesc(const SceneObjectCreateDesc& desc)
{
	// Add Object에서 입력한 설정을 실제 RenderObject 초기화로 바꾸는 지점이다.
	//
	// 예전 DemoScene은 C++ 코드에 "캐릭터는 0번, 무기는 1번"처럼 고정되어 있었다.
	// 이제는 UI가 넘긴 desc를 기준으로 필요한 constant buffer와 렌더 목록을 구성한다.
	// 즉, 화면에 보이는 오브젝트는 반드시 이 생성 경로를 지나가게 된다.
	auto object = std::make_unique<RenderObject>(m_pGraphicsEngine);
	object->Initialize();

	const bool hasVertexBuffer = !desc.vertexBufferName.empty();
	const std::string modelInfoName = desc.modelName.empty()
		? desc.vertexBufferName
		: desc.modelName;
	const bool hasModel = !modelInfoName.empty();
	const bool hasTexture = !desc.textureName.empty();
	const bool hasAnimation = !desc.animationName.empty();

	if (hasVertexBuffer)
	{
		object->SetVIBuffer(desc.vertexBufferName);
	}

	if (hasModel)
	{
		// 피킹은 GPU vertex buffer가 아니라 CPU에 남아있는 ModelInfo의 AABB/triangle을 사용한다.
		// FBX를 선택한 경우에는 파일명으로 ModelInfo를 찾고,
		// 기본 도형처럼 Model FBX가 None인 경우에는 Vertex Buffer 이름으로 만들어 둔 ModelInfo를 사용한다.
		object->SetModelInfo(modelInfoName);
	}

	if (hasTexture)
	{
		object->SetDiffuseTexture(desc.textureName);
	}

	switch (desc.renderType)
	{
	case SceneRenderType::SkinnedMesh:
		object->CreateVSConstantBuffer();
		object->CreateVSBoneConstantBuffer();
		object->CreatePSConstantBuffer();
		if (hasAnimation)
		{
			object->SetAnimation(desc.animationName);
		}
		break;
	case SceneRenderType::PbrMesh:
		object->CreateVSConstantBuffer();
		object->CreateVSPBRConstantBuffer();
		object->CreatePSPBRConstantBuffer();
		object->SetPBRTextures(
			"albedo.png",
			"normal.png",
			"ao.png",
			"metallic.png",
			"roughness.png",
			"height.png");
		break;
	case SceneRenderType::EquipmentMesh:
		object->CreateVSConstantBuffer();
		object->CreateVSTargetBoneConstantBuffer();
		object->CreatePSConstantBuffer();
		break;
	case SceneRenderType::CubeMap:
	case SceneRenderType::Billboard:
	case SceneRenderType::StaticMesh:
	default:
		object->CreateVSConstantBuffer();
		object->CreatePSConstantBuffer();
		break;
	}

	object->SetObjectScl(Matrix::CreateScale(1.0f));
	object->SetObjectPos(Matrix::CreateTranslation({ 0.0f, 0.0f, 0.0f }));

	RenderObject* rawObject = object.get();
	m_objects.push_back(std::move(object));
	m_objectNames.push_back(MakeUniqueObjectName(m_objectNames, desc.name));

	m_mainRenderItems.push_back({ rawObject, desc.renderType });

	if (desc.castShadow &&
		desc.renderType != SceneRenderType::CubeMap &&
		desc.renderType != SceneRenderType::Billboard)
	{
		m_shadowRenderItems.push_back({ rawObject, desc.renderType });
	}
}

void DemoScene::CreateRenderItems()
{
	// 기본 데모 오브젝트 생성을 꺼둔 동안에는 렌더 아이템도 자동 등록하지 않는다.
	// 앞으로는 Add Object UI가 RenderObject를 만들 때 선택한 SceneRenderType에 맞춰
	// m_mainRenderItems / m_shadowRenderItems에 직접 등록하게 만들 예정이다.
	m_shadowRenderItems.clear();
	m_mainRenderItems.clear();

#if 0
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
#endif
}
