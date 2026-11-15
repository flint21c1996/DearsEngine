#include "DemoScene.h"

#include "DearsGraphicsEngine.h"
#include "EasingFunc.h"
#include "InputManager.h"

#include <imgui.h>
#include <ImGuizmo.h>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <exception>
#include <utility>
#include <filesystem>

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
// 렌더 프리셋은 특정 씬이 아니라 프로젝트 전체에서 쓰는 에셋이므로 씬 파일과 따로 저장한다.
constexpr const char* RenderPresetsPath = "../RenderPresets.txt";

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

std::string ToString(MaterialShadingModel shadingModel)
{
	return shadingModel == MaterialShadingModel::ThinFilm ? "ThinFilm" : "DefaultLit";
}

MaterialShadingModel ParseMaterialShadingModel(const std::string& value)
{
	return value == "ThinFilm"
		? MaterialShadingModel::ThinFilm
		: MaterialShadingModel::DefaultLit;
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
		if (!(stream >> values[index]) || !std::isfinite(values[index]))
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

	// 씬의 오브젝트가 프리셋을 이름으로 참조하므로 씬보다 먼저 읽는다.
	// 파일이 아직 없으면 바로 끌어다 쓸 수 있는 기본 프리셋을 만들어 저장해 둔다.
	if (!m_renderPresets.Load(RenderPresetsPath))
	{
		m_renderPresets.Clear();
		m_renderPresets.AddDefaults();
		m_renderPresets.Save(RenderPresetsPath);
	}

	if (!LoadSceneFromFile(SavedScenePath))
	{
		// Only an absent directory entry is a new scene. Invalid/unreadable files
		// (including dangling links or status errors) must not become save targets.
		std::error_code error;
		const auto status = std::filesystem::symlink_status(SavedScenePath, error);
		if (status.type() == std::filesystem::file_type::not_found &&
			(!error || error == std::errc::no_such_file_or_directory))
		{
			m_scenePath = SavedScenePath;
			m_sceneFileError.clear();
		}
	}

	// 불러온 직후의 상태가 실행 취소의 바닥이 된다. 여기까지만 되돌릴 수 있다.
	m_history.Reset(SerializeScene());
	m_historyDirty = false;
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
		},
		[this](int objectIndex, const std::string& presetName)
		{
			AssignRenderPreset(objectIndex, presetName);
		},
		[this](int objectIndex, const std::string& textureName)
		{
			AssignDiffuseTexture(objectIndex, textureName);
		});

	m_pInspectorPanel = std::make_unique<ObjectInspectorPanel>(
		m_pGraphicsEngine,
		m_objects,
		m_selectedObjectIndex,
		[this]()
		{
			HandleSceneObjectEdited();
		},
		&m_renderPresets,
		[this](int objectIndex, const std::string& presetName)
		{
			AssignRenderPreset(objectIndex, presetName);
		},
		[this](int objectIndex, const std::string& textureName)
		{
			AssignDiffuseTexture(objectIndex, textureName);
		});
	// 메시와 텍스처를 타일로 보여 주고 월드/Hierarchy/Inspector로 끌어다 놓는 출발점이다.
	m_pContentBrowserPanel = std::make_unique<ContentBrowserPanel>(m_pGraphicsEngine);
	// 월드 이미지가 받을 수 있는 드래그 종류: 메시(오브젝트 생성), 텍스처/프리셋(놓인 오브젝트에 적용).
	m_pGraphicsEngine->SetViewportDropTypes({
		MeshAssetDragPayloadType,
		TextureAssetDragPayloadType,
		RenderPresetDragPayloadType });
	// 프리셋 전용 브라우저: 프리셋을 만들고 고치며, 여기서 끌어다 Hierarchy/Inspector에 놓는다.
	m_pRenderPresetPanel = std::make_unique<RenderPresetPanel>(
		m_renderPresets,
		[this]()
		{
			HandleRenderPresetLibraryEdited();
		},
		[this](const std::string& oldName, const std::string& newName)
		{
			HandleRenderPresetRenamed(oldName, newName);
		});
	m_pPickingPanel = std::make_unique<PickingPanel>(m_pickingManager);
	m_pLightShadowMapPanel = std::make_unique<LightShadowMapPanel>(
		m_pGraphicsEngine,
		m_objects,
		m_selectedObjectIndex);
	m_pAStarDemoPanel = std::make_unique<AStarDemoPanel>();
	m_pGraphicsEngine->AddEditorPanel(m_pScenePanel.get());
	m_pGraphicsEngine->AddEditorPanel(m_pPickingPanel.get());
	m_pGraphicsEngine->AddEditorPanel(m_pInspectorPanel.get());
	m_pGraphicsEngine->AddEditorPanel(m_pLightShadowMapPanel.get());
	m_pGraphicsEngine->AddEditorPanel(m_pAStarDemoPanel.get());
	m_pGraphicsEngine->AddEditorPanel(m_pContentBrowserPanel.get());
	m_pGraphicsEngine->AddEditorPanel(m_pRenderPresetPanel.get());

// The world image establishes its own extent before scene input is processed.
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

		// 애니메이션 시간은 Play 중에만 흐른다. 편집 중에는 멈춰 있어야 물체를 배치하기 쉽고,
		// "실행해 봐야 움직인다"는 점이 편집 상태와 실행 상태를 구분해 준다.
		// 상수 버퍼 갱신(Update)은 편집 중에도 해야 옮긴 위치가 화면에 반영된다.
		if (m_isPlaying)
		{
			object->UpdateAnimationTime(deltaTime);
		}
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
	const auto& viewport = m_pGraphicsEngine->GetEditorViewport();
	const auto mouse = inputManager.GetMousePos();
	const auto local = viewport.ToLocal(mouse.x, mouse.y);
	if (!viewport.AllowsSceneInput(mouse.x, mouse.y) || !local) return;
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

	if (ImGui::GetIO().WantTextInput)
	{
		return;
	}

	if (inputManager.GetKeyState(KEY::LBUTTON) != KEY_STATE::TAP)
	{
		return;
	}

	// 기즈모 핸들을 누른 클릭은 "선택 바꾸기"가 아니라 "선택한 물체 옮기기"이다.
	// 여기서 피킹까지 하면 핸들 뒤에 있는 다른 물체가 선택되어 버린다.
	if (GetSelectedObject() && (ImGuizmo::IsOver() || ImGuizmo::IsUsing()))
	{
		return;
	}

	const PickingResult result = m_pickingManager.Pick(
		m_objects,
		Vector2(local->x, local->y),
		viewport.PixelWidth(),
		viewport.PixelHeight(),
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

int DemoScene::CollectLights(CommonConstantBufferData& buffer) const
{
	// GPU 상수 버퍼는 고정 크기이므로 매 프레임 먼저 비우고,
	// Hierarchy 순서대로 활성 라이트를 MAX_LIGHTS개까지만 복사한다.
	buffer.lightNum = 0;
	for (UINT index = 0; index < MAX_LIGHTS; ++index)
	{
		buffer.light[index] = Light();
	}

	// RenderObject의 Transform을 GPU Light 구조체로 옮기는 작업을 한곳에 모은다.
	// 그림자 카메라와 디버그 절두체가 같은 방향/Up 계산을 공유해야
	// 두 기능이 서로 다른 축을 사용하는 실수를 막을 수 있다.
	auto appendLight = [&buffer](const RenderObject* object)
	{
		if (!object || !object->mIsLight || buffer.lightNum >= MAX_LIGHTS)
		{
			return;
		}

		Light light = object->mSceneLight;
		light.position = Vector3(object->ObjectPos._41, object->ObjectPos._42, object->ObjectPos._43);
		// 로컬 +Z를 라이트가 빛을 내보내는 방향으로 정의한다.
		light.direction = Vector3::TransformNormal(Vector3::UnitZ, object->ObjectRot);
		light.direction.Normalize();
		// 로컬 +Y도 함께 전달해야 그림자 카메라의 Roll이 오브젝트 Rotation을 그대로 따른다.
		// direction만 보고 임의의 Up을 고르면 수직 방향 근처에서 절두체가 갑자기 회전한다.
		light.shadowUp = Vector3::TransformNormal(Vector3::UnitY, object->ObjectRot);
		light.shadowUp.Normalize();
		buffer.light[buffer.lightNum] = light;
		++buffer.lightNum;
	};

	const RenderObject* selectedObject = GetSelectedObject();
	int selectedLightIndex = -1;

	// GPU Light와 Shadow Map 배열은 같은 인덱스를 사용하므로 라이트 순서를 매 프레임
	// Hierarchy 순서로 고정한다. 선택된 라이트를 0번으로 옮기면 클릭할 때마다 모든
	// Shadow Map의 의미가 바뀌므로, 선택 상태는 인덱스만 따로 기억한다.
	for (const std::unique_ptr<RenderObject>& object : m_objects)
	{
		if (!object || !object->mIsLight || buffer.lightNum >= MAX_LIGHTS)
		{
			continue;
		}

		if (object.get() == selectedObject)
		{
			selectedLightIndex = static_cast<int>(buffer.lightNum);
		}
		appendLight(object.get());
	}

	return selectedLightIndex;
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
	const MaterialShadingModel effectiveShadingModel =
		desc.renderType == SceneRenderType::PbrMesh
		? desc.shadingModel
		: MaterialShadingModel::DefaultLit;
	const SceneRenderPath effectiveRenderPath =
		effectiveShadingModel == MaterialShadingModel::ThinFilm
		? SceneRenderPath::Forward
		: desc.renderPath;

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
		object->mSceneLight.shadowNear = desc.shadowNear;
		object->mSceneLight.shadowFar = desc.shadowFar;
		object->mSceneLight.shadowFovY = desc.shadowFovY;
		object->mSceneLight.shadowWidth = desc.shadowWidth;
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
		object->mPSThinFilmConstantBufferData.n1 = desc.thinFilmOutsideIor;
		object->mPSThinFilmConstantBufferData.n2 = desc.thinFilmIor;
		object->mPSThinFilmConstantBufferData.time = desc.thinFilmThicknessModulation;
		object->mPSThinFilmConstantBufferData.d = desc.thinFilmThickness;
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
		// 텍스처 사용 플래그까지 채운 뒤 Shading Model을 설정해야
		// Thin Film b3 버퍼가 PBR 재질 상태를 온전히 복사해서 생성된다.
		object->SetShadingModel(effectiveShadingModel);
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

	// Inspector의 Rendering 항목과 RenderDispatcher가 읽는 렌더 설정이다.
	object->mRenderType = desc.renderType;
	object->mRenderPath = effectiveRenderPath;
	object->mRenderPresetName = desc.renderPresetName;
	object->mVertexShaderOverride = desc.vertexShaderOverride;
	object->mPixelShaderOverride = desc.pixelShaderOverride;

	RenderObject* rawObject = object.get();
	m_objects.push_back(std::move(object));

	SceneObjectCreateDesc storedDesc = desc;
	storedDesc.name = MakeUniqueObjectName(m_objectNames, desc.name);
	storedDesc.shadingModel = effectiveShadingModel;
	storedDesc.renderPath = effectiveRenderPath;
	m_objectNames.push_back(storedDesc.name);
	m_objectCreateDescs.push_back(storedDesc);

	if (!rawObject->mIsLight)
	{
		m_mainRenderItems.push_back({ rawObject, desc.renderType, effectiveRenderPath });
	}

	if (!rawObject->mIsLight && desc.castShadow &&
		desc.renderType != SceneRenderType::CubeMap &&
		desc.renderType != SceneRenderType::Billboard)
	{
		m_shadowRenderItems.push_back({ rawObject, desc.renderType, effectiveRenderPath });
	}

	// 저장된 프리셋 참조가 있으면 지금의 프리셋 내용으로 다시 풀어 낸다.
	// 씬을 저장한 뒤에 프리셋을 고쳤더라도 다시 열었을 때 최신 내용이 적용된다.
	if (!rawObject->mRenderPresetName.empty())
	{
		ApplyRenderPresetToObject(m_objects.size() - 1);
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
	// 편집이 한 번 끝날 때마다 호출된다. 두 가지 일을 한다.
	// 1. 실행 취소 기록에 남길 변경이 생겼다고 표시한다. (실제 기록은 UpdateEditor가 조작이 끝난 뒤에 남긴다.)
	// 2. 현재 씬 파일에 저장한다.
	//
	// Play 중에는 아무것도 하지 않는다. 실행 중에 바뀐 상태가 편집 원본 파일이나
	// 실행 취소 기록에 섞이면 Stop을 눌러도 원래 씬으로 돌아갈 수 없기 때문이다.
	if (m_isPlaying)
	{
		return;
	}
	m_historyDirty = true;

	std::ofstream file(m_scenePath, std::ios::trunc);
	if (!file.is_open())
	{
		return;
	}
	WriteScene(file);
}

std::string DemoScene::SerializeScene() const
{
	std::ostringstream stream;
	WriteScene(stream);
	return stream.str();
}

void DemoScene::WriteScene(std::ostream& file) const
{
	// 파일 저장, 실행 취소 스냅샷, Play 시작 시점의 백업이 모두 이 형식을 쓴다.
	// 한 가지 형식만 있으면 "저장했다 다시 열기"가 되는 값은 실행 취소와 Stop 복원도 자동으로 된다.

	// Version 6부터 PBR Shading Model과 Thin Film 전용 굴절률/두께 값도 저장한다.
	// Version 7부터 Vertex/Pixel Shader 재정의도 저장한다.
	// Version 8부터 오브젝트에 지정한 렌더 프리셋의 이름도 저장한다.
	file << "DearsScene 8\n";
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
			<< ToString(object->GetShadingModel()) << ' '
			<< std::quoted(object->mVertexShaderOverride) << ' '
			<< std::quoted(object->mPixelShaderOverride) << ' '
			<< std::quoted(object->mRenderPresetName) << ' '
			<< desc.castShadow << ' '
			<< object->mPSPBRConstantBufferData.material.metallic << ' '
			<< object->mPSPBRConstantBufferData.material.roughness << ' '
			<< object->mVSPBRConstantBufferData.heightScale << ' '
			<< object->mPSThinFilmConstantBufferData.n1 << ' '
			<< object->mPSThinFilmConstantBufferData.n2 << ' '
			<< object->mPSThinFilmConstantBufferData.time << ' '
			<< object->mPSThinFilmConstantBufferData.d << ' '
			<< object->mEditorRotationDegrees.x << ' '
			<< object->mEditorRotationDegrees.y << ' '
			<< object->mEditorRotationDegrees.z << ' '
			<< object->mSceneLight.strength << ' '
			<< object->mSceneLight.lightColor.x << ' '
			<< object->mSceneLight.lightColor.y << ' '
			<< object->mSceneLight.lightColor.z << ' '
			<< object->mSceneLight.fallOffEnd << ' '
			<< object->mSceneLight.spotPower << ' '
			<< object->mSceneLight.shadowNear << ' '
			<< object->mSceneLight.shadowFar << ' '
			<< object->mSceneLight.shadowFovY << ' '
			<< object->mSceneLight.shadowWidth << ' ';

		WriteMatrix(file, object->ObjectPos);
		WriteMatrix(file, object->ObjectRot);
		WriteMatrix(file, object->ObjectScl);
		file << '\n';
	}
}

bool DemoScene::LoadSceneFromFile(const std::string& scenePath)
{
	try
	{
		std::string loadedPath = scenePath;
		std::ifstream file(loadedPath);
		if (file.is_open() && ReadScene(file))
		{
			m_scenePath.swap(loadedPath);
			m_sceneFileError.clear();
			return true;
		}
	}
	catch (const std::exception&)
	{
		// Construction happened in a temporary scene; the live scene is untouched.
	}
	m_sceneFileError = "Cannot open scene (unreadable, invalid or unsupported): " + scenePath;
	return false;
}

void DemoScene::ClearSceneObjects()
{
	// 렌더 목록은 RenderObject 포인터를 들고 있으므로 오브젝트보다 먼저 비운다.
	m_mainRenderItems.clear();
	m_shadowRenderItems.clear();
	m_objects.clear();
	m_objectNames.clear();
	m_objectCreateDescs.clear();
	m_selectedObjectIndex = -1;
}

bool DemoScene::RestoreScene(const std::string& serializedScene)
{
	const int previousSelection = m_selectedObjectIndex;
	try
	{
		std::istringstream stream(serializedScene);
		if (ReadScene(stream))
		{
			m_selectedObjectIndex =
				previousSelection >= 0 && previousSelection < static_cast<int>(m_objects.size())
				? previousSelection : -1;
			return true;
		}
	}
	catch (const std::exception&)
	{
	}
	m_sceneFileError = "Cannot restore scene: invalid snapshot or object creation failed.";
	return false;
}

bool DemoScene::ReadScene(std::istream& file)
{
	struct ParsedObject
	{
		SceneObjectCreateDesc desc;
		Matrix position, rotation, scale;
	};
	std::vector<ParsedObject> parsed;
	std::string magic;
	int version = 0;
	if (!(file >> magic >> version) || magic != "DearsScene" || version < 1 || version > 8)
	{
		return false;
	}

	long long objectCount = 0;
	if (!(file >> objectCount) || objectCount < 0)
	{
		return false;
	}

	for (long long index = 0; index < objectCount; ++index)
	{
		SceneObjectCreateDesc desc;
		std::string renderTypeName;
		std::string renderPathName = "Forward";
		std::string shadingModelName = "DefaultLit";
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
			return false;
		}

		if (version >= 2 && !(file >> renderPathName))
		{
			return false;
		}
		if (version >= 6 && !(file >> shadingModelName))
		{
			return false;
		}
		// Version 6 이하에는 셰이더 재정의가 없으므로 빈 문자열(= 종류별 기본 셰이더)을 그대로 쓴다.
		if (version >= 7 && !(file
			>> std::quoted(desc.vertexShaderOverride)
			>> std::quoted(desc.pixelShaderOverride)))
		{
			return false;
		}
		if (version >= 8 && !(file >> std::quoted(desc.renderPresetName)))
		{
			return false;
		}
		if (!(file >> desc.castShadow))
		{
			return false;
		}

		// 구버전 Scene에는 PBR 숫자 설정이 없으므로 desc의 기본값을 그대로 사용한다.
		// Version 3부터는 Inspector에서 마지막으로 조절한 값을 읽어 복원한다.
		if (version >= 3 && !(file
			>> desc.pbrMetallic
			>> desc.pbrRoughness
			>> desc.pbrHeightScale))
		{
			return false;
		}

		// Version 5 이하에는 Thin Film이 렌더 목록에 연결되지 않았으므로
		// 구버전 파일은 Default Lit과 구조체 기본값을 그대로 사용한다.
		if (version >= 6 && !(file
			>> desc.thinFilmOutsideIor
			>> desc.thinFilmIor
			>> desc.thinFilmThicknessModulation
			>> desc.thinFilmThickness))
		{
			return false;
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
			return false;
		}

		// Version 4까지는 그림자 카메라 설정이 하드코딩되어 있었다.
		// 구버전 파일은 SceneObjectCreateDesc의 안전한 기본값을 사용한다.
		if (version >= 5 && !(file
			>> desc.shadowNear
			>> desc.shadowFar
			>> desc.shadowFovY
			>> desc.shadowWidth))
		{
			return false;
		}

		if (!ReadMatrix(file, position) || !ReadMatrix(file, rotation) || !ReadMatrix(file, scale))
		{
			return false;
		}

		desc.renderType = ParseSceneRenderType(renderTypeName);
		desc.renderPath = ParseSceneRenderPath(renderPathName);
		desc.shadingModel = ParseMaterialShadingModel(shadingModelName);
		if (ToString(desc.renderType) != renderTypeName ||
			ToString(desc.renderPath) != renderPathName ||
			ToString(desc.shadingModel) != shadingModelName)
		{
			return false;
		}
		for (float value : {
			desc.pbrMetallic, desc.pbrRoughness, desc.pbrHeightScale,
			desc.thinFilmOutsideIor, desc.thinFilmIor, desc.thinFilmThicknessModulation,
			desc.rotationDegrees.x, desc.rotationDegrees.y, desc.rotationDegrees.z,
			desc.lightStrength, desc.lightColor.x, desc.lightColor.y, desc.lightColor.z,
			desc.lightRange, desc.spotPower, desc.shadowNear, desc.shadowFar,
			desc.shadowFovY, desc.shadowWidth })
		{
			if (!std::isfinite(value)) return false;
		}
		parsed.push_back({ std::move(desc), position, rotation, scale });
	}
	// Validate the complete stream before constructing or replacing any live state.
	file >> std::ws;
	if (file.bad() || !file.eof()) return false;

	DemoScene loaded(m_pGraphicsEngine);
	loaded.m_renderPresets = m_renderPresets;
	for (const ParsedObject& object : parsed)
	{
		loaded.CreateObjectFromDesc(
			object.desc, object.position, object.rotation, object.scale, false);
	}

	// All parsing and object construction succeeded. Panel references to these
	// vector objects remain valid; render-item pointers move with their owners.
	m_mainRenderItems.swap(loaded.m_mainRenderItems);
	m_shadowRenderItems.swap(loaded.m_shadowRenderItems);
	m_objects.swap(loaded.m_objects);
	m_objectNames.swap(loaded.m_objectNames);
	m_objectCreateDescs.swap(loaded.m_objectCreateDescs);
	m_selectedObjectIndex = -1;
	return true;
}

void DemoScene::HandleSceneObjectEdited()
{
	if (m_selectedObjectIndex >= 0 &&
		m_selectedObjectIndex < static_cast<int>(m_objects.size()) &&
		m_selectedObjectIndex < static_cast<int>(m_objectCreateDescs.size()))
	{
		RenderObject* object = m_objects[m_selectedObjectIndex].get();
		SceneObjectCreateDesc& desc = m_objectCreateDescs[m_selectedObjectIndex];
		if (object)
		{
			desc.shadingModel = object->GetShadingModel();

			// Shading Model처럼 프리셋 결과에 영향을 주는 값이 바뀌었을 수 있으므로
			// 프리셋을 다시 풀어서 오브젝트 설정과 렌더 목록을 맞춘다.
			ApplyRenderPresetToObject(static_cast<size_t>(m_selectedObjectIndex));

			desc.thinFilmOutsideIor = object->mPSThinFilmConstantBufferData.n1;
			desc.thinFilmIor = object->mPSThinFilmConstantBufferData.n2;
			desc.thinFilmThicknessModulation = object->mPSThinFilmConstantBufferData.time;
			desc.thinFilmThickness = object->mPSThinFilmConstantBufferData.d;

			// 현재 Thin Film 셰이더는 G-Buffer를 출력하지 않고 최종 색상을 계산한다.
			// Inspector에서 런타임에 Thin Film으로 바꾼 경우에도 Scene Item을 Forward로 맞춘다.
			if (desc.shadingModel == MaterialShadingModel::ThinFilm)
			{
				desc.renderPath = SceneRenderPath::Forward;
				object->mRenderPath = SceneRenderPath::Forward;
				for (SceneRenderItem& item : m_mainRenderItems)
				{
					if (item.object == object)
					{
						item.renderPath = SceneRenderPath::Forward;
					}
				}
			}
		}
	}

	SaveSceneToFile();
}

void DemoScene::ApplyRenderPresetToObject(size_t objectIndex)
{
	if (objectIndex >= m_objects.size() || objectIndex >= m_objectCreateDescs.size())
	{
		return;
	}

	RenderObject* object = m_objects[objectIndex].get();
	if (!object || object->mIsLight)
	{
		return;
	}
	SceneObjectCreateDesc& desc = m_objectCreateDescs[objectIndex];

	// 오브젝트는 프리셋을 이름으로만 기억한다. 지워진 프리셋을 가리키고 있으면 참조를 풀고
	// 오브젝트 고유의 기본 방식(종류별 기본 셰이더 + 생성할 때 고른 렌더 경로)으로 되돌린다.
	const RenderPreset* preset = m_renderPresets.Find(object->mRenderPresetName);
	if (!preset)
	{
		object->mRenderPresetName.clear();
	}

	// desc.renderPath는 프리셋이 없을 때 쓰는 오브젝트 고유의 경로라서 프리셋으로 덮어쓰지 않는다.
	// 그래야 프리셋을 해제했을 때 원래 경로로 돌아갈 수 있다.
	object->mVertexShaderOverride = preset ? preset->vertexShader : std::string();
	object->mPixelShaderOverride = preset ? preset->pixelShader : std::string();
	object->mRenderPath = preset ? preset->renderPath : desc.renderPath;

	// Thin Film Pixel Shader는 전용 b3 상수 버퍼를 읽는다. PBR 재질을 가진 오브젝트가 프리셋으로
	// 처음 Thin Film을 쓰게 되면 그 버퍼를 만들어 둔다. SetShadingModel(ThinFilm)이 PBR 설정을
	// 복사해 버퍼를 만들어 주므로 잠깐 거쳤다가, 오브젝트 고유의 Shading Model은 원래대로 돌려놓는다.
	const MeshRoute route = ResolveMeshRoute(
		object->mRenderType,
		object->mRenderPath,
		object->GetShadingModel(),
		object->mVertexShaderOverride,
		object->mPixelShaderOverride);
	if (route.pixelShader == ShaderCatalog::PixelShader::ThinFilm &&
		object->mIs_PSPBRConstant &&
		!object->mIs_PSThinFilmConstant)
	{
		const MaterialShadingModel ownShadingModel = object->GetShadingModel();
		object->SetShadingModel(MaterialShadingModel::ThinFilm);
		object->SetShadingModel(ownShadingModel);
	}

	desc.renderPresetName = object->mRenderPresetName;
	desc.vertexShaderOverride = object->mVertexShaderOverride;
	desc.pixelShaderOverride = object->mPixelShaderOverride;

	// 렌더 목록의 renderPath는 Geometry/Forward 패스가 오브젝트를 나누는 기준이므로
	// 오브젝트 쪽 값과 항상 같아야 한다.
	for (SceneRenderItem& item : m_mainRenderItems)
	{
		if (item.object == object)
		{
			item.renderPath = object->mRenderPath;
		}
	}
	for (SceneRenderItem& item : m_shadowRenderItems)
	{
		if (item.object == object)
		{
			item.renderPath = object->mRenderPath;
		}
	}
}

void DemoScene::AssignRenderPreset(int objectIndex, const std::string& presetName)
{
	if (objectIndex < 0 || objectIndex >= static_cast<int>(m_objects.size()))
	{
		return;
	}

	RenderObject* object = m_objects[objectIndex].get();
	if (!object || object->mIsLight)
	{
		return;
	}

	// 빈 이름이나 없는 프리셋은 "프리셋 해제"로 처리된다. (ApplyRenderPresetToObject가 참조를 푼다.)
	object->mRenderPresetName = presetName;
	ApplyRenderPresetToObject(static_cast<size_t>(objectIndex));
	SaveSceneToFile();
}

void DemoScene::HandleRenderPresetLibraryEdited()
{
	// 프리셋은 여러 오브젝트가 함께 쓰는 에셋이다. 내용이 바뀌거나 지워졌으면
	// 그 프리셋을 쓰는 오브젝트를 찾아다니는 대신 전부 다시 풀어서 항상 일치하게 만든다.
	for (size_t index = 0; index < m_objects.size(); ++index)
	{
		if (m_objects[index] && !m_objects[index]->mRenderPresetName.empty())
		{
			ApplyRenderPresetToObject(index);
		}
	}

	m_renderPresets.Save(RenderPresetsPath);
	SaveSceneToFile();
}

void DemoScene::HandleRenderPresetRenamed(const std::string& oldName, const std::string& newName)
{
	// 오브젝트는 이름으로 프리셋을 참조하므로 이름이 바뀌면 참조도 따라가야 한다.
	// 저장은 뒤이어 호출되는 HandleRenderPresetLibraryEdited()가 한다.
	for (size_t index = 0; index < m_objects.size(); ++index)
	{
		RenderObject* object = m_objects[index].get();
		if (object && object->mRenderPresetName == oldName)
		{
			object->mRenderPresetName = newName;
			if (index < m_objectCreateDescs.size())
			{
				m_objectCreateDescs[index].renderPresetName = newName;
			}
		}
	}
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
