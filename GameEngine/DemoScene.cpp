#include "DemoScene.h"

#include "DearsGraphicsEngine.h"
#include "EasingFunc.h"
#include "InputManager.h"

#include <imgui.h>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>

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
constexpr const char* SavedScenePath = "../SavedScene.txt";

std::string ToString(SceneRenderType renderType)
{
	switch (renderType)
	{
	case SceneRenderType::SkinnedMesh:
		return "SkinnedMesh";
	case SceneRenderType::EquipmentMesh:
		return "EquipmentMesh";
	case SceneRenderType::CubeMap:
		return "CubeMap";
	case SceneRenderType::Billboard:
		return "Billboard";
	case SceneRenderType::PbrMesh:
		return "PbrMesh";
	case SceneRenderType::DirectionalLight: return "DirectionalLight";
	case SceneRenderType::PointLight: return "PointLight";
	case SceneRenderType::SpotLight: return "SpotLight";
	case SceneRenderType::StaticMesh:
	default:
		return "StaticMesh";
	}
}

SceneRenderType ParseSceneRenderType(const std::string& value)
{
	if (value == "SkinnedMesh") return SceneRenderType::SkinnedMesh;
	if (value == "EquipmentMesh") return SceneRenderType::EquipmentMesh;
	if (value == "CubeMap") return SceneRenderType::CubeMap;
	if (value == "Billboard") return SceneRenderType::Billboard;
	if (value == "PbrMesh") return SceneRenderType::PbrMesh;
	if (value == "DirectionalLight") return SceneRenderType::DirectionalLight;
	if (value == "PointLight") return SceneRenderType::PointLight;
	if (value == "SpotLight") return SceneRenderType::SpotLight;
	return SceneRenderType::StaticMesh;
}

std::string ToString(SceneRenderPath renderPath)
{
	return renderPath == SceneRenderPath::Deferred ? "Deferred" : "Forward";
}

SceneRenderPath ParseSceneRenderPath(const std::string& value)
{
	return value == "Deferred" ? SceneRenderPath::Deferred : SceneRenderPath::Forward;
}

void WriteMatrix(std::ostream& stream, const Matrix& matrix)
{
	const float* values = &matrix._11;
	for (int index = 0; index < 16; ++index)
	{
		stream << values[index] << ' ';
	}
}

bool ReadMatrix(std::istream& stream, Matrix& matrix)
{
	float* values = &matrix._11;
	for (int index = 0; index < 16; ++index)
	{
		if (!(stream >> values[index]))
		{
			return false;
		}
	}

	return true;
}

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
	LoadSceneFromFile();
}

void DemoScene::RegisterEditorPanels(int screenWidth)
{
	// The hierarchy and inspector panels belong to the demo world layout,
	// so they are registered from the scene instead of the engine core.
	// Panel width is derived from the window width so the render area keeps
	// a stable 5:1 proportion against the editor lane.
	m_selectedObjectIndex = -1;
	m_pScenePanel = std::make_unique<SceneHierarchyPanel>(
		m_pGraphicsEngine,
		m_objects,
		m_objectNames,
		m_selectedObjectIndex,
		[this](const SceneObjectCreateDesc& desc)
		{
			CreateObjectFromDesc(desc);
		});

	m_pInspectorPanel = std::make_unique<ObjectInspectorPanel>(
		m_pGraphicsEngine,
		m_objects,
		m_selectedObjectIndex,
		[this]()
		{
			HandleSceneObjectEdited();
		});
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
	// Delete는 현재 선택된 씬 오브젝트를 제거하는 에디터 명령이다.
	// ImGui 텍스트 입력/콤보 조작 중에는 키보드를 UI가 사용하므로 삭제하지 않는다.
	if (!ImGui::GetIO().WantCaptureKeyboard &&
		inputManager.GetKeyState(KEY::DELETE_KEY) == KEY_STATE::TAP)
	{
		DeleteSelectedObject();
		return;
	}

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

void DemoScene::CollectLights(CommonConstantBufferData& buffer) const
{
	// GPU 상수 버퍼는 고정 크기이므로 매 프레임 먼저 비우고,
	// Hierarchy 순서대로 활성 라이트를 MAX_LIGHTS개까지만 복사한다.
	buffer.lightNum = 0;
	for (UINT index = 0; index < MAX_LIGHTS; ++index)
	{
		buffer.light[index] = Light();
	}

	for (const std::unique_ptr<RenderObject>& object : m_objects)
	{
		if (!object || !object->mIsLight || buffer.lightNum >= MAX_LIGHTS)
		{
			continue;
		}

		Light light = object->mSceneLight;
		light.position = Vector3(object->ObjectPos._41, object->ObjectPos._42, object->ObjectPos._43);
		// 로컬 +Z를 라이트가 빛을 내보내는 방향으로 정의한다.
		light.direction = Vector3::TransformNormal(Vector3::UnitZ, object->ObjectRot);
		light.direction.Normalize();
		buffer.light[buffer.lightNum] = light;
		++buffer.lightNum;
	}
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
	m_objectCreateDescs.clear();
	m_shadowRenderItems.clear();
	m_mainRenderItems.clear();

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
	// CubeMap은 장면 전체를 감싸는 배경이므로 마우스로 선택하지 않는다.
	GetCubeMap()->SetPickable(false);
	GetCubeMap()->CreateVSConstantBuffer();
	GetCubeMap()->CreatePSConstantBuffer();
	GetCubeMap()->SetVIBuffer("CubeMap");
	GetCubeMap()->SetObjectScl(Matrix::CreateScale(1));
	GetCubeMap()->SetObjectPos(Matrix::CreateTranslation({ 0,0,0 }));
#endif
}

void DemoScene::CreateObjectFromDesc(const SceneObjectCreateDesc& desc)
{
	CreateObjectFromDesc(
		desc,
		Matrix::CreateTranslation({ 0.0f, 0.0f, 0.0f }),
		Matrix::CreateFromYawPitchRoll(
			DirectX::XMConvertToRadians(desc.rotationDegrees.y),
			DirectX::XMConvertToRadians(desc.rotationDegrees.x),
			DirectX::XMConvertToRadians(desc.rotationDegrees.z)),
		Matrix::CreateScale(1.0f),
		true);
}

void DemoScene::CreateObjectFromDesc(const SceneObjectCreateDesc& desc, Matrix position, Matrix rotation, Matrix scale, bool saveScene)
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
		// 기본 도형처럼 Picking FBX가 None인 경우에는 Vertex Buffer 이름으로 만들어 둔 ModelInfo를 사용한다.
		object->SetModelInfo(modelInfoName);
	}

	if (hasTexture)
	{
		object->SetDiffuseTexture(desc.textureName);
	}

	switch (desc.renderType)
	{
	case SceneRenderType::DirectionalLight:
	case SceneRenderType::PointLight:
	case SceneRenderType::SpotLight:
		// 라이트는 월드 Transform과 조명 데이터만 가지며 GPU 메시 버퍼는 만들지 않는다.
		object->mIsLight = true;
		object->SetPickable(false);
		object->mSceneLight.strength = desc.lightStrength;
		object->mSceneLight.lightColor = desc.lightColor;
		object->mSceneLight.fallOffStart = 0.0f;
		object->mSceneLight.fallOffEnd = desc.lightRange;
		object->mSceneLight.spotPower = desc.spotPower;
		object->mSceneLight.lightType = static_cast<UINT>(
			desc.renderType == SceneRenderType::DirectionalLight ? LightEnum::DIRECTIONAL_LIGHT :
			desc.renderType == SceneRenderType::PointLight ? LightEnum::POINT_LIGHT : LightEnum::SPOT_LIGHT);
		break;
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
		// Constant Buffer를 생성하기 전에 저장된 재질 값을 넣어야
		// 로드 직후 첫 렌더링부터 저장 당시와 동일한 값이 GPU에 전달된다.
		object->mPSPBRConstantBufferData.material.metallic = desc.pbrMetallic;
		object->mPSPBRConstantBufferData.material.roughness = desc.pbrRoughness;
		object->mVSPBRConstantBufferData.heightScale = desc.pbrHeightScale;
		object->CreateVSConstantBuffer();
		object->CreateVSPBRConstantBuffer();
		object->CreatePSPBRConstantBuffer();
		object->SetPBRTextures(
			desc.pbrAlbedoTextureName,
			desc.pbrNormalTextureName,
			desc.pbrAOTextureName,
			desc.pbrMetallicTextureName,
			desc.pbrRoughnessTextureName,
			desc.pbrHeightTextureName);
		break;
	case SceneRenderType::EquipmentMesh:
		object->CreateVSConstantBuffer();
		object->CreateVSTargetBoneConstantBuffer();
		object->CreatePSConstantBuffer();
		break;
	case SceneRenderType::CubeMap:
		// CubeMap의 메시가 ray와 먼저 충돌해 실제 장면 오브젝트의 선택을 막지 않도록 한다.
		object->SetPickable(false);
		object->CreateVSConstantBuffer();
		object->CreatePSConstantBuffer();
		break;
	case SceneRenderType::Billboard:
	case SceneRenderType::StaticMesh:
	default:
		object->CreateVSConstantBuffer();
		object->CreatePSConstantBuffer();
		break;
	}

	object->SetObjectScl(scale);
	object->SetObjectRot(rotation);
	object->SetObjectPos(position);
	object->mEditorRotationDegrees = desc.rotationDegrees;

	RenderObject* rawObject = object.get();
	m_objects.push_back(std::move(object));

	SceneObjectCreateDesc storedDesc = desc;
	storedDesc.name = MakeUniqueObjectName(m_objectNames, desc.name);
	m_objectNames.push_back(storedDesc.name);
	m_objectCreateDescs.push_back(storedDesc);

	if (!rawObject->mIsLight)
	{
		m_mainRenderItems.push_back({ rawObject, desc.renderType, desc.renderPath });
	}

	if (!rawObject->mIsLight && desc.castShadow &&
		desc.renderType != SceneRenderType::CubeMap &&
		desc.renderType != SceneRenderType::Billboard)
	{
		m_shadowRenderItems.push_back({ rawObject, desc.renderType, desc.renderPath });
	}

	if (saveScene)
	{
		SaveSceneToFile();
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

void DemoScene::SaveSceneToFile() const
{
	std::ofstream file(SavedScenePath, std::ios::trunc);
	if (!file.is_open())
	{
		return;
	}

	// Version 4부터 Inspector Rotation과 월드 라이트 설정을 저장한다.
	file << "DearsScene 4\n";
	file << m_objects.size() << '\n';

	for (size_t index = 0; index < m_objects.size(); ++index)
	{
		const RenderObject* object = m_objects[index].get();
		if (!object || index >= m_objectCreateDescs.size())
		{
			continue;
		}

		const SceneObjectCreateDesc& desc = m_objectCreateDescs[index];
		file << std::quoted(desc.name) << ' '
			<< std::quoted(desc.vertexBufferName) << ' '
			<< std::quoted(desc.modelName) << ' '
			<< std::quoted(desc.textureName) << ' '
			<< std::quoted(desc.animationName) << ' '
			<< std::quoted(desc.pbrAlbedoTextureName) << ' '
			<< std::quoted(desc.pbrNormalTextureName) << ' '
			<< std::quoted(desc.pbrAOTextureName) << ' '
			<< std::quoted(desc.pbrMetallicTextureName) << ' '
			<< std::quoted(desc.pbrRoughnessTextureName) << ' '
			<< std::quoted(desc.pbrHeightTextureName) << ' '
			<< ToString(desc.renderType) << ' '
			<< ToString(desc.renderPath) << ' '
			<< desc.castShadow << ' '
			<< object->mPSPBRConstantBufferData.material.metallic << ' '
			<< object->mPSPBRConstantBufferData.material.roughness << ' '
			<< object->mVSPBRConstantBufferData.heightScale << ' '
			<< object->mEditorRotationDegrees.x << ' '
			<< object->mEditorRotationDegrees.y << ' '
			<< object->mEditorRotationDegrees.z << ' '
			<< object->mSceneLight.strength << ' '
			<< object->mSceneLight.lightColor.x << ' '
			<< object->mSceneLight.lightColor.y << ' '
			<< object->mSceneLight.lightColor.z << ' '
			<< object->mSceneLight.fallOffEnd << ' '
			<< object->mSceneLight.spotPower << ' ';

		WriteMatrix(file, object->ObjectPos);
		WriteMatrix(file, object->ObjectRot);
		WriteMatrix(file, object->ObjectScl);
		file << '\n';
	}
}

void DemoScene::LoadSceneFromFile()
{
	std::ifstream file(SavedScenePath);
	if (!file.is_open())
	{
		return;
	}

	std::string magic;
	int version = 0;
	file >> magic >> version;
	if (magic != "DearsScene" || version < 1 || version > 4)
	{
		return;
	}

	size_t objectCount = 0;
	file >> objectCount;

	for (size_t index = 0; index < objectCount; ++index)
	{
		SceneObjectCreateDesc desc;
		std::string renderTypeName;
		std::string renderPathName = "Forward";
		Matrix position;
		Matrix rotation;
		Matrix scale;

		if (!(file >> std::quoted(desc.name)
			>> std::quoted(desc.vertexBufferName)
			>> std::quoted(desc.modelName)
			>> std::quoted(desc.textureName)
			>> std::quoted(desc.animationName)
			>> std::quoted(desc.pbrAlbedoTextureName)
			>> std::quoted(desc.pbrNormalTextureName)
			>> std::quoted(desc.pbrAOTextureName)
			>> std::quoted(desc.pbrMetallicTextureName)
			>> std::quoted(desc.pbrRoughnessTextureName)
			>> std::quoted(desc.pbrHeightTextureName)
			>> renderTypeName))
		{
			return;
		}

		if (version >= 2 && !(file >> renderPathName))
		{
			return;
		}
		if (!(file >> desc.castShadow))
		{
			return;
		}

		// 구버전 Scene에는 PBR 숫자 설정이 없으므로 desc의 기본값을 그대로 사용한다.
		// Version 3부터는 Inspector에서 마지막으로 조절한 값을 읽어 복원한다.
		if (version >= 3 && !(file
			>> desc.pbrMetallic
			>> desc.pbrRoughness
			>> desc.pbrHeightScale))
		{
			return;
		}

		if (version >= 4 && !(file
			>> desc.rotationDegrees.x
			>> desc.rotationDegrees.y
			>> desc.rotationDegrees.z
			>> desc.lightStrength
			>> desc.lightColor.x
			>> desc.lightColor.y
			>> desc.lightColor.z
			>> desc.lightRange
			>> desc.spotPower))
		{
			return;
		}

		if (!ReadMatrix(file, position) || !ReadMatrix(file, rotation) || !ReadMatrix(file, scale))
		{
			return;
		}

		desc.renderType = ParseSceneRenderType(renderTypeName);
		desc.renderPath = ParseSceneRenderPath(renderPathName);
		CreateObjectFromDesc(desc, position, rotation, scale, false);
	}
}

void DemoScene::HandleSceneObjectEdited()
{
	SaveSceneToFile();
}

void DemoScene::DeleteSelectedObject()
{
	if (m_selectedObjectIndex < 0 ||
		m_selectedObjectIndex >= static_cast<int>(m_objects.size()))
	{
		return;
	}

	const size_t eraseIndex = static_cast<size_t>(m_selectedObjectIndex);
	RenderObject* eraseObject = m_objects[eraseIndex].get();

	// 렌더 아이템은 RenderObject 포인터를 들고 있으므로,
	// 실제 unique_ptr을 지우기 전에 같은 포인터를 가리키는 항목을 먼저 제거한다.
	const auto removeRenderItem = [eraseObject](std::vector<SceneRenderItem>& items)
		{
			items.erase(
				std::remove_if(
					items.begin(),
					items.end(),
					[eraseObject](const SceneRenderItem& item)
					{
						return item.object == eraseObject;
					}),
				items.end());
		};

	removeRenderItem(m_mainRenderItems);
	removeRenderItem(m_shadowRenderItems);

	m_objects.erase(m_objects.begin() + eraseIndex);

	if (eraseIndex < m_objectNames.size())
	{
		m_objectNames.erase(m_objectNames.begin() + eraseIndex);
	}

	if (eraseIndex < m_objectCreateDescs.size())
	{
		m_objectCreateDescs.erase(m_objectCreateDescs.begin() + eraseIndex);
	}

	// 삭제 뒤에는 같은 위치에 밀려온 다음 오브젝트를 선택한다.
	// 마지막 오브젝트를 지운 경우에는 새 마지막 오브젝트를 선택하고,
	// 씬이 비었다면 선택을 해제한다.
	if (m_objects.empty())
	{
		m_selectedObjectIndex = -1;
	}
	else if (eraseIndex >= m_objects.size())
	{
		m_selectedObjectIndex = static_cast<int>(m_objects.size()) - 1;
	}
	else
	{
		m_selectedObjectIndex = static_cast<int>(eraseIndex);
	}

	SaveSceneToFile();
}
