// DemoScene의 에디터 기능 구현부이다.
//
// 씬의 내용(오브젝트 생성, 저장 형식, 렌더 목록)은 DemoScene.cpp에 두고,
// "편집기를 어떻게 조작하는가"에 해당하는 것들을 이 파일에 모았다.
// - 이동/회전/크기 기즈모와 W/E/R 전환
// - 실행 취소/다시 실행 (Ctrl+Z / Ctrl+Y)
// - File/Edit 메뉴, 씬 새로 만들기/열기/다른 이름으로 저장
// - Play/Stop
// - 콘텐츠 브라우저에서 뷰포트로 끌어다 놓기
#include "DemoScene.h"

#include "Camera.h"
#include "DearsGraphicsEngine.h"
#include "InputManager.h"

#include <imgui.h>
#include <ImGuizmo.h>

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>

namespace
{
constexpr const char* DefaultScenePath = "../SavedScene.txt";
constexpr const char* ScenesDirectory = "../Scenes";

// 파일 이름으로 쓸 수 없는 문자를 걸러 낸다. 사용자가 입력한 이름이 경로를 벗어나지 못하게도 한다.
std::string SanitizeSceneName(const std::string& name)
{
	std::string result;
	for (const char character : name)
	{
		const bool isAllowed =
			(character >= 'a' && character <= 'z') ||
			(character >= 'A' && character <= 'Z') ||
			(character >= '0' && character <= '9') ||
			character == '_' || character == '-' || character == ' ';
		if (isAllowed)
		{
			result.push_back(character);
		}
	}

	// 앞뒤 공백만 있는 이름은 빈 이름으로 취급한다.
	const size_t first = result.find_first_not_of(' ');
	if (first == std::string::npos)
	{
		return std::string();
	}
	const size_t last = result.find_last_not_of(' ');
	return result.substr(first, last - first + 1);
}

std::string MakeScenePath(const std::string& sceneName)
{
	return std::string(ScenesDirectory) + "/" + sceneName + ".txt";
}
}

void DemoScene::UpdateEditor(InputManager& inputManager, Camera* camera)
{
	(void)inputManager;

	const ImGuiIO& io = ImGui::GetIO();
	const auto& viewport = m_pGraphicsEngine->GetEditorViewport();

	// 글자를 입력하는 중에는 단축키를 처리하지 않는다. 이름 칸에서 Ctrl+Z를 누르면
	// 씬이 아니라 그 입력 칸의 실행 취소가 동작해야 한다.
	if (!io.WantTextInput)
	{
		if (io.KeyCtrl)
		{
			if (ImGui::IsKeyPressed(ImGuiKey_Z, false))
			{
				// Ctrl+Shift+Z도 다시 실행으로 받는다.
				if (io.KeyShift)
				{
					Redo();
				}
				else
				{
					Undo();
				}
			}
			if (ImGui::IsKeyPressed(ImGuiKey_Y, false))
			{
				Redo();
			}
			if (ImGui::IsKeyPressed(ImGuiKey_S, false))
			{
				SaveSceneToFile();
			}
			if (ImGui::IsKeyPressed(ImGuiKey_D, false))
			{
				DuplicateSelectedObject();
			}
		}
		else if (viewport.hovered && !ImGui::IsMouseDown(ImGuiMouseButton_Right))
		{
			// 언리얼과 같은 배치: W 이동, E 회전, R 크기.
			// 마우스 오른쪽 버튼을 누르고 있는 동안에는 W/E가 카메라 이동이므로 전환하지 않는다.
			if (ImGui::IsKeyPressed(ImGuiKey_W, false))
			{
				m_gizmoOperation = 0;
			}
			if (ImGui::IsKeyPressed(ImGuiKey_E, false))
			{
				m_gizmoOperation = 1;
			}
			if (ImGui::IsKeyPressed(ImGuiKey_R, false))
			{
				m_gizmoOperation = 2;
			}
		}
	}

	HandleViewportDrop(camera);
	DrawTransformGizmo(camera);
	DrawScenePopups();
	CommitHistoryIfIdle();
}

void DemoScene::DrawTransformGizmo(Camera* camera)
{
	const auto& viewport = m_pGraphicsEngine->GetEditorViewport();
	RenderObject* object = GetSelectedObject();

	// Play 중에는 편집 도구를 숨긴다. 실행 화면을 보는 동안 실수로 물체를 옮기지 않게 한다.
	if (!object || !camera || !viewport.Valid() || m_isPlaying)
	{
		m_gizmoWasUsing = false;
		return;
	}

	// 기즈모는 Viewport 창의 그리기 목록에 그린다. 이 함수는 Viewport 창이 아직 열려 있는 동안 호출되므로
	// 현재 창이 곧 Viewport이고, 그 창이 걸어 둔 클립 영역 덕분에 월드 이미지 밖으로 삐져나오지 않는다.
	ImGuizmo::SetOrthographic(!camera->IsPerspective());
	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(viewport.x, viewport.y, viewport.width, viewport.height);

	ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
	if (m_gizmoOperation == 1)
	{
		operation = ImGuizmo::ROTATE;
	}
	else if (m_gizmoOperation == 2)
	{
		operation = ImGuizmo::SCALE;
	}
	// 크기 조절은 항상 물체 자신의 축 기준이다. 월드 축 기준으로 늘리면 회전된 물체가 찌그러진다.
	const ImGuizmo::MODE mode =
		(m_gizmoLocalSpace || operation == ImGuizmo::SCALE) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;

	// SimpleMath의 Matrix는 행 벡터 기준 row-major이고, 메모리 배치가 ImGuizmo가 기대하는
	// float[16](OpenGL식 column-major)과 같다. 그래서 변환 없이 첫 원소의 주소를 그대로 넘긴다.
	Matrix view = camera->GetViewRow();
	Matrix projection = camera->GetProjRow();
	Matrix world = object->ObjectScl * object->ObjectRot * object->ObjectPos;

	const bool changed = ImGuizmo::Manipulate(&view._11, &projection._11, operation, mode, &world._11);
	if (changed)
	{
		Vector3 scale;
		Quaternion rotation;
		Vector3 translation;
		if (world.Decompose(scale, rotation, translation))
		{
			// 조작한 성분만 반영한다. 행렬을 분해했다 다시 합치는 과정의 오차가
			// 건드리지 않은 성분(예: 이동만 했는데 크기 값)에 쌓이지 않게 하기 위해서다.
			if (operation == ImGuizmo::TRANSLATE)
			{
				object->ObjectPos = Matrix::CreateTranslation(translation);
			}
			else if (operation == ImGuizmo::ROTATE)
			{
				object->ObjectRot = Matrix::CreateFromQuaternion(rotation);
				// Inspector는 사람이 읽는 degree 값을 따로 보관하므로 함께 갱신한다.
				// ToEuler()는 (pitch, yaw, roll) 순서의 radian을 돌려주며,
				// Inspector가 CreateFromYawPitchRoll(y, x, z)로 행렬을 만드는 순서와 맞는다.
				const Vector3 euler = rotation.ToEuler();
				object->mEditorRotationDegrees = Vector3(
					DirectX::XMConvertToDegrees(euler.x),
					DirectX::XMConvertToDegrees(euler.y),
					DirectX::XMConvertToDegrees(euler.z));
			}
			else
			{
				// 크기가 0이 되면 월드 행렬의 역행렬을 구할 수 없어 조명과 피킹이 깨진다.
				scale.x = (std::max)(scale.x, 0.001f);
				scale.y = (std::max)(scale.y, 0.001f);
				scale.z = (std::max)(scale.z, 0.001f);
				object->ObjectScl = Matrix::CreateScale(scale);
			}
		}
	}

	// 드래그하는 동안에는 저장하지 않고, 손을 뗀 순간에 한 번만 저장한다.
	// 그래야 기즈모 드래그 한 번이 실행 취소 한 단계가 된다.
	const bool isUsing = ImGuizmo::IsUsing();
	if (m_gizmoWasUsing && !isUsing)
	{
		HandleSceneObjectEdited();
	}
	m_gizmoWasUsing = isUsing;
}

void DemoScene::HandleViewportDrop(Camera* camera)
{
	std::string type;
	std::string data;
	float localX = 0.0f;
	float localY = 0.0f;
	if (!m_pGraphicsEngine->ConsumeViewportDrop(type, data, localX, localY) || !camera)
	{
		return;
	}

	const auto& viewport = m_pGraphicsEngine->GetEditorViewport();
	if (!viewport.Valid())
	{
		return;
	}

	// 놓은 위치 아래에 어떤 오브젝트가 있는지 클릭 선택과 같은 방식으로 찾는다.
	const Vector2 dropPosition(localX, localY);
	const PickingResult picked = m_pickingManager.Pick(
		m_objects,
		dropPosition,
		viewport.PixelWidth(),
		viewport.PixelHeight(),
		camera);

	if (type == MeshAssetDragPayloadType)
	{
		// 다른 물체 위에 놓았으면 그 표면에, 빈 곳에 놓았으면 카메라 앞 일정 거리에 만든다.
		const PickingRay ray = m_pickingManager.BuildRayFromScreen(
			dropPosition,
			viewport.PixelWidth(),
			viewport.PixelHeight(),
			camera);
		const float distance = picked.hit ? picked.distance : 20.0f;
		SpawnMeshAsset(data, ray.origin + ray.direction * distance);
		return;
	}

	if (!picked.hit)
	{
		return;
	}

	if (type == RenderPresetDragPayloadType)
	{
		m_selectedObjectIndex = picked.objectIndex;
		AssignRenderPreset(picked.objectIndex, data);
	}
	else if (type == TextureAssetDragPayloadType)
	{
		m_selectedObjectIndex = picked.objectIndex;
		AssignDiffuseTexture(picked.objectIndex, data);
	}
}

void DemoScene::SpawnMeshAsset(const std::string& vertexBufferName, const Vector3& position)
{
	SceneObjectCreateDesc desc;
	desc.vertexBufferName = vertexBufferName;
	desc.textureName = "White.png";
	desc.renderType = SceneRenderType::StaticMesh;
	desc.name = vertexBufferName;

	// 콘텐츠 브라우저의 표시 이름을 오브젝트 이름으로 쓰고,
	// 같은 표시 이름의 FBX가 있으면 피킹용 모델 정보로 연결한다. (기본 도형은 Vertex Buffer 이름으로 찾는다.)
	for (const EditorResourceOption& mesh : EditorResourceCatalog::VertexBuffers)
	{
		if (vertexBufferName != mesh.key)
		{
			continue;
		}

		desc.name = mesh.label;
		for (const EditorResourceOption& model : EditorResourceCatalog::Models)
		{
			if (std::strcmp(model.label, mesh.label) == 0)
			{
				desc.modelName = model.key;
			}
		}
	}

	// 전용 메시는 그에 맞는 종류로 만들어야 제대로 그려진다.
	if (vertexBufferName == "CubeMap")
	{
		desc.renderType = SceneRenderType::CubeMap;
		desc.castShadow = false;
	}
	else if (vertexBufferName == "BillBoardSquare")
	{
		desc.renderType = SceneRenderType::Billboard;
		desc.castShadow = false;
	}

	CreateObjectFromDesc(
		desc,
		Matrix::CreateTranslation(position),
		Matrix::Identity,
		Matrix::CreateScale(1.0f),
		true);
	m_selectedObjectIndex = static_cast<int>(m_objects.size()) - 1;
}

void DemoScene::AssignDiffuseTexture(int objectIndex, const std::string& textureName)
{
	if (objectIndex < 0 ||
		objectIndex >= static_cast<int>(m_objects.size()) ||
		objectIndex >= static_cast<int>(m_objectCreateDescs.size()) ||
		textureName.empty())
	{
		return;
	}

	RenderObject* object = m_objects[objectIndex].get();
	// Diffuse 텍스처는 일반 메시의 t0 슬롯이다. PBR 메시는 Albedo/Normal 등 별도 슬롯을 쓰므로 받지 않는다.
	if (!object || object->mIsLight || object->mIs_PSPBRConstant)
	{
		return;
	}

	object->SetDiffuseTexture(textureName);
	object->mEditorDiffuseTextureName = textureName;
	// 씬 파일은 생성 정보의 텍스처 이름을 저장하므로 함께 바꿔야 다시 열었을 때 유지된다.
	m_objectCreateDescs[objectIndex].textureName = textureName;
	SaveSceneToFile();
}

void DemoScene::DuplicateSelectedObject()
{
	if (m_selectedObjectIndex < 0 ||
		m_selectedObjectIndex >= static_cast<int>(m_objects.size()) ||
		m_selectedObjectIndex >= static_cast<int>(m_objectCreateDescs.size()) ||
		m_isPlaying)
	{
		return;
	}

	const RenderObject* source = m_objects[m_selectedObjectIndex].get();
	if (!source)
	{
		return;
	}

	// 생성 정보에는 "만들 때의 값"이 들어 있으므로, Inspector에서 그 뒤에 바꾼 값을 먼저 옮겨 담는다.
	// (씬 파일을 저장할 때 오브젝트에서 직접 읽어 쓰는 값들과 같은 목록이다.)
	SceneObjectCreateDesc desc = m_objectCreateDescs[m_selectedObjectIndex];
	desc.shadingModel = source->GetShadingModel();
	desc.pbrMetallic = source->mPSPBRConstantBufferData.material.metallic;
	desc.pbrRoughness = source->mPSPBRConstantBufferData.material.roughness;
	desc.pbrHeightScale = source->mVSPBRConstantBufferData.heightScale;
	desc.thinFilmOutsideIor = source->mPSThinFilmConstantBufferData.n1;
	desc.thinFilmIor = source->mPSThinFilmConstantBufferData.n2;
	desc.thinFilmThicknessModulation = source->mPSThinFilmConstantBufferData.time;
	desc.thinFilmThickness = source->mPSThinFilmConstantBufferData.d;
	desc.rotationDegrees = source->mEditorRotationDegrees;
	desc.lightStrength = source->mSceneLight.strength;
	desc.lightColor = source->mSceneLight.lightColor;
	desc.lightRange = source->mSceneLight.fallOffEnd;
	desc.spotPower = source->mSceneLight.spotPower;
	desc.shadowNear = source->mSceneLight.shadowNear;
	desc.shadowFar = source->mSceneLight.shadowFar;
	desc.shadowFovY = source->mSceneLight.shadowFovY;
	desc.shadowWidth = source->mSceneLight.shadowWidth;

	// 원본과 완전히 겹치면 복제된 것을 알아보기 어려우므로 X축으로 조금 옮겨서 만든다.
	const Matrix position = source->ObjectPos * Matrix::CreateTranslation(1.0f, 0.0f, 0.0f);
	const Matrix rotation = source->ObjectRot;
	const Matrix scale = source->ObjectScl;

	CreateObjectFromDesc(desc, position, rotation, scale, true);
	m_selectedObjectIndex = static_cast<int>(m_objects.size()) - 1;
}

void DemoScene::CommitHistoryIfIdle()
{
	if (!m_historyDirty || m_isPlaying)
	{
		return;
	}

	// 슬라이더를 끌거나 기즈모를 잡고 있는 동안에는 매 프레임 값이 바뀐다.
	// 손을 뗄 때까지 기다렸다가 한 번만 기록해야 드래그 한 번이 실행 취소 한 단계가 된다.
	if (ImGui::IsAnyItemActive() || ImGuizmo::IsUsing())
	{
		return;
	}

	m_history.Commit(SerializeScene());
	m_historyDirty = false;
}

void DemoScene::Undo()
{
	if (m_isPlaying)
	{
		return;
	}

	// 아직 기록하지 않은 변경이 있으면 먼저 기록해야 "방금 한 것"부터 되돌릴 수 있다.
	const SceneHistory previousHistory = m_history;
	const bool previousDirty = m_historyDirty;
	if (m_historyDirty)
	{
		m_history.Commit(SerializeScene());
		m_historyDirty = false;
	}

	if (const std::string* state = m_history.Undo())
	{
		if (!RestoreScene(*state))
		{
			m_history = previousHistory;
			m_historyDirty = previousDirty;
			return;
		}
		// 되돌린 상태를 파일에도 반영한다. 복원 자체는 새 편집이 아니므로 기록에는 남기지 않는다.
		SaveSceneToFile();
		m_historyDirty = false;
	}
}

void DemoScene::Redo()
{
	if (m_isPlaying || m_historyDirty)
	{
		// 새로 편집한 뒤에는 다시 실행할 대상이 없다. (Commit이 뒤쪽 기록을 버린다.)
		return;
	}

	const SceneHistory previousHistory = m_history;
	if (const std::string* state = m_history.Redo())
	{
		if (!RestoreScene(*state))
		{
			m_history = previousHistory;
			return;
		}
		SaveSceneToFile();
		m_historyDirty = false;
	}
}

void DemoScene::SetPlaying(bool playing)
{
	if (playing == m_isPlaying)
	{
		return;
	}

	if (playing)
	{
		// 편집 중이던 변경을 기록에 남긴 뒤, 지금 상태를 "편집 원본"으로 보관한다.
		if (m_historyDirty)
		{
			m_history.Commit(SerializeScene());
			m_historyDirty = false;
		}
		m_playSnapshot = SerializeScene();
		m_isPlaying = true;
		return;
	}

	// Keep Play and its snapshot intact if restoration fails.
	if (!RestoreScene(m_playSnapshot)) return;
	m_isPlaying = false;
	m_playSnapshot.clear();
	m_historyDirty = false;
}

std::string DemoScene::GetSceneDisplayName() const
{
	return std::filesystem::path(m_scenePath).stem().string();
}

void DemoScene::NewScene(const std::string& sceneName)
{
	const std::string name = SanitizeSceneName(sceneName);
	if (name.empty() || m_isPlaying) return;

	std::error_code error;
	std::filesystem::create_directories(ScenesDirectory, error);
	if (error)
	{
		m_sceneFileError = "Cannot create the scene directory.";
		return;
	}

	const std::string path = MakeScenePath(name);
	DemoScene empty(m_pGraphicsEngine);
	const std::string content = empty.SerializeScene();
	// CREATE_NEW rejects collisions atomically, including a file created after
	// a directory scan. Never open an existing scene with truncation.
	HANDLE file = CreateFileW(std::filesystem::path(path).c_str(),
		GENERIC_WRITE | DELETE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE)
	{
		m_sceneFileError = "Scene already exists or cannot be created: " + path;
		return;
	}
	DWORD written = 0;
	const bool saved = WriteFile(file, content.data(), static_cast<DWORD>(content.size()), &written, nullptr)
		&& written == content.size() && FlushFileBuffers(file);
	if (!saved)
	{
		// Delete only this newly created file, through its still-exclusive handle.
		FILE_DISPOSITION_INFO remove{ TRUE };
		SetFileInformationByHandle(file, FileDispositionInfo, &remove, sizeof(remove));
	}
	const bool closed = CloseHandle(file) != FALSE;
	if (!saved || !closed)
	{
		m_sceneFileError = "Cannot write the new scene: " + path;
		return;
	}

	// Disk creation succeeded; now replace memory, path and history together.
	ClearSceneObjects();
	m_scenePath = path;
	m_history.Reset(content);
	m_historyDirty = false;
	m_sceneFileError.clear();
}

void DemoScene::OpenScene(const std::string& scenePath)
{
	if (m_isPlaying || scenePath.empty()) return;

	if (!LoadSceneFromFile(scenePath)) return;
	m_history.Reset(SerializeScene());
	m_historyDirty = false;
}

void DemoScene::SaveSceneAs(const std::string& sceneName)
{
	const std::string name = SanitizeSceneName(sceneName);
	if (name.empty() || m_isPlaying)
	{
		return;
	}

	std::error_code error;
	std::filesystem::create_directories(ScenesDirectory, error);

	// 이후의 편집은 새 파일에 저장된다. 원래 파일은 마지막으로 저장된 상태 그대로 남는다.
	m_scenePath = MakeScenePath(name);
	SaveSceneToFile();
}

void DemoScene::DrawEditorMenuBar()
{
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("New Scene...", nullptr, false, !m_isPlaying))
		{
			m_pendingScenePopup = 1;
			strncpy_s(m_sceneNameBuffer, sizeof(m_sceneNameBuffer), "NewScene", _TRUNCATE);
		}

		if (ImGui::BeginMenu("Open Scene", !m_isPlaying))
		{
			// 기본 씬 파일과 Scenes 폴더 안의 씬들을 보여 준다.
			if (ImGui::MenuItem("SavedScene", nullptr, m_scenePath == DefaultScenePath))
			{
				OpenScene(DefaultScenePath);
			}

			std::error_code error;
			if (std::filesystem::is_directory(ScenesDirectory, error))
			{
				for (const std::filesystem::directory_entry& entry :
					std::filesystem::directory_iterator(ScenesDirectory, error))
				{
					if (!entry.is_regular_file() || entry.path().extension() != ".txt")
					{
						continue;
					}

					const std::string path = MakeScenePath(entry.path().stem().string());
					if (ImGui::MenuItem(entry.path().stem().string().c_str(), nullptr, m_scenePath == path))
					{
						OpenScene(path);
					}
				}
			}
			ImGui::EndMenu();
		}

		ImGui::Separator();
		// 편집할 때마다 자동으로 저장되므로 Save는 "지금 확실히 저장"하는 용도이다.
		if (ImGui::MenuItem("Save", "Ctrl+S", false, !m_isPlaying))
		{
			SaveSceneToFile();
		}
		if (ImGui::MenuItem("Save As...", nullptr, false, !m_isPlaying))
		{
			m_pendingScenePopup = 2;
			strncpy_s(m_sceneNameBuffer, sizeof(m_sceneNameBuffer), GetSceneDisplayName().c_str(), _TRUNCATE);
		}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Edit"))
	{
		if (ImGui::MenuItem("Undo", "Ctrl+Z", false, !m_isPlaying && (m_history.CanUndo() || m_historyDirty)))
		{
			Undo();
		}
		if (ImGui::MenuItem("Redo", "Ctrl+Y", false, !m_isPlaying && !m_historyDirty && m_history.CanRedo()))
		{
			Redo();
		}
		ImGui::Separator();
		const bool hasSelection = GetSelectedObject() != nullptr;
		if (ImGui::MenuItem("Duplicate", "Ctrl+D", false, hasSelection && !m_isPlaying))
		{
			DuplicateSelectedObject();
		}
		if (ImGui::MenuItem("Delete", "Del", false, hasSelection))
		{
			DeleteSelectedObject();
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Move", "W", m_gizmoOperation == 0))
		{
			m_gizmoOperation = 0;
		}
		if (ImGui::MenuItem("Rotate", "E", m_gizmoOperation == 1))
		{
			m_gizmoOperation = 1;
		}
		if (ImGui::MenuItem("Scale", "R", m_gizmoOperation == 2))
		{
			m_gizmoOperation = 2;
		}
		ImGui::MenuItem("Local Space", nullptr, &m_gizmoLocalSpace);
		ImGui::EndMenu();
	}

	// Play / Stop: 메뉴 바에 버튼으로 둔다.
	ImGui::Separator();
	if (m_isPlaying)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.70f, 0.20f, 0.20f, 1.0f));
		if (ImGui::SmallButton("Stop"))
		{
			SetPlaying(false);
		}
		ImGui::PopStyleColor();
		ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "Playing - changes are discarded on Stop");
	}
	else
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.55f, 0.25f, 1.0f));
		if (ImGui::SmallButton("Play"))
		{
			SetPlaying(true);
		}
		ImGui::PopStyleColor();
		ImGui::TextDisabled("%s", GetSceneDisplayName().c_str());
	}
	ImGui::Separator();
}

void DemoScene::DrawScenePopups()
{
	if (!m_sceneFileError.empty())
	{
		ImGui::OpenPopup("Scene File Error");
		if (ImGui::BeginPopupModal("Scene File Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::TextWrapped("%s", m_sceneFileError.c_str());
			if (ImGui::Button("OK"))
			{
				m_sceneFileError.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	// 메뉴 항목 안에서는 팝업을 바로 열 수 없어서(ID 범위가 다르다) 표시만 해 두고 여기서 연다.
	const char* popupTitle = "Scene Name";
	if (m_pendingScenePopup != 0 && !ImGui::IsPopupOpen(popupTitle))
	{
		ImGui::OpenPopup(popupTitle);
	}

	if (!ImGui::BeginPopupModal(popupTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		return;
	}

	ImGui::TextUnformatted(m_pendingScenePopup == 1
		? "Create an empty scene:"
		: "Save the current scene as:");
	ImGui::SetNextItemWidth(260.0f);
	const bool entered = ImGui::InputText(
		"##sceneName",
		m_sceneNameBuffer,
		sizeof(m_sceneNameBuffer),
		ImGuiInputTextFlags_EnterReturnsTrue);
	ImGui::TextDisabled("Saved to %s/<name>.txt", ScenesDirectory);

	const bool isValid = !SanitizeSceneName(m_sceneNameBuffer).empty();
	ImGui::BeginDisabled(!isValid);
	const bool confirmed = ImGui::Button("OK", ImVec2(120.0f, 0.0f)) || (entered && isValid);
	ImGui::EndDisabled();
	ImGui::SameLine();
	const bool cancelled = ImGui::Button("Cancel", ImVec2(120.0f, 0.0f));

	if (confirmed)
	{
		if (m_pendingScenePopup == 1)
		{
			NewScene(m_sceneNameBuffer);
		}
		else
		{
			SaveSceneAs(m_sceneNameBuffer);
		}
	}
	if (confirmed || cancelled)
	{
		m_pendingScenePopup = 0;
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();
}
