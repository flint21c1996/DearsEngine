#pragma once
#include <algorithm>
#include <imgui.h>
#include <functional>
#include <cstring>
#include <vector>
#include <memory>
#include <string>
#include "DearsGraphicsEngine.h"
#include "EditorResourceCatalog.h"
#include "IEditorPanel.h"
#include "RenderObject.h"

static constexpr float EDITOR_PANEL_WIDTH_RATIO = 1.0f / 6.0f;

inline int GetEditorPanelWidth(float screenWidth)
{
	// The editor lane should take one sixth of the window width so the
	// render area keeps roughly a 5:1 proportion against the tool area.
	return static_cast<int>(screenWidth * EDITOR_PANEL_WIDTH_RATIO);
}

struct SceneObjectCreateDesc
{
	std::string name = "New Object";
	std::string vertexBufferName = "MyBox";
	std::string modelName;
	std::string textureName = "White.png";
	std::string animationName;
	std::string pbrAlbedoTextureName = "albedo.png";
	std::string pbrNormalTextureName = "normal.png";
	std::string pbrAOTextureName = "ao.png";
	std::string pbrMetallicTextureName = "metallic.png";
	std::string pbrRoughnessTextureName = "roughness.png";
	std::string pbrHeightTextureName = "height.png";
	// 텍스처 선택과 별개로 사용자가 Inspector에서 조절하는 PBR 숫자 설정이다.
	// Scene을 다시 열었을 때도 같은 재질 상태를 복원하기 위해 생성 정보에 함께 보관한다.
	float pbrMetallic = 0.0f;
	float pbrRoughness = 0.0f;
	float pbrHeightScale = 0.0f;
	// Thin Film Pixel Shader의 박막 간섭 계산에 사용하는 재질 전용 값이다.
	// n1/n2는 바깥 매질과 박막의 굴절률이고, thickness는 광학 경로 차이를 결정한다.
	float thinFilmOutsideIor = 1.0f;
	float thinFilmIor = 1.33f;
	float thinFilmThicknessModulation = 0.0f;
	int thinFilmThickness = 1;
	Vector3 rotationDegrees = Vector3::Zero;
	float lightStrength = 1.0f;
	Vector3 lightColor = Vector3::One;
	float lightRange = 20.0f;
	float spotPower = 32.0f;
	float shadowNear = 0.1f;
	float shadowFar = 100.0f;
	float shadowFovY = 70.0f;
	float shadowWidth = 30.0f;
	SceneRenderType renderType = SceneRenderType::StaticMesh;
	SceneRenderPath renderPath = SceneRenderPath::Forward;
	MaterialShadingModel shadingModel = MaterialShadingModel::DefaultLit;
	bool castShadow = true;
};

class SceneHierarchyPanel : public IEditorPanel
{
public:
	SceneHierarchyPanel(
		DearsGraphicsEngine* graphicsEngine,
		std::vector<std::unique_ptr<RenderObject>>& objects,
		std::vector<std::string>& objectNames,
		int& selectedIndex,
		std::function<void(const SceneObjectCreateDesc&)> createObjectCallback)
		: m_pGraphicsEngine(graphicsEngine)
		, m_objects(objects)
		, m_objectNames(objectNames)
		, m_selectedIndex(selectedIndex)
		, m_createObjectCallback(createObjectCallback)
	{}

	const char* GetName() const override { return "Scene Hierarchy"; }

	void Draw() override
	{
		ImVec2 screenSize = ImGui::GetIO().DisplaySize;
		float panelW = static_cast<float>(GetEditorPanelWidth(screenSize.x));
		float topH    = screenSize.y * 0.45f;

		ImGui::SetNextWindowPos(ImVec2(screenSize.x - panelW, 0.0f), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(panelW, topH), ImGuiCond_Always);

		ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse;

		ImGui::Begin("Scene Hierarchy", nullptr, flags);
		ImGui::SetWindowFontScale(0.8f);

		for (int i = 0; i < (int)m_objects.size(); i++)
		{
			const std::string& label = (i < (int)m_objectNames.size())
				? m_objectNames[i]
				: ("Object " + std::to_string(i));

			bool selected = (m_selectedIndex == i);
			if (ImGui::Selectable(label.c_str(), selected))
				m_selectedIndex = i;
		}

		ImGui::Separator();

		if (ImGui::Button("+ Add Object"))
			ImGui::OpenPopup("AddObjectPopup");

		if (ImGui::BeginPopup("AddObjectPopup"))
		{
			static SceneObjectCreateDesc desc;
			static char nameBuf[128] = "New Object";
			static int vertexBufferIndex = 0;
			static int modelIndex = 0;
			static int textureIndex = 0;
			static int animationIndex = 0;
			static int pbrMaterialIndex = 0;
			static int pbrAlbedoIndex = 1;
			static int pbrNormalIndex = 2;
			static int pbrAOIndex = 3;
			static int pbrMetallicIndex = 4;
			static int pbrRoughnessIndex = 5;
			static int pbrHeightIndex = 6;
			static int renderTypeIndex = static_cast<int>(SceneRenderType::StaticMesh);
			static int renderPathIndex = static_cast<int>(SceneRenderPath::Forward);
			static int shadingModelIndex = static_cast<int>(MaterialShadingModel::DefaultLit);

			if (ImGui::IsWindowAppearing())
			{
				// 팝업을 열자마자 이름 입력칸에 포커스를 준다.
				// 이렇게 해두면 Add Object를 누른 뒤 바로 타이핑해서 이름을 바꿀 수 있다.
				ImGui::SetKeyboardFocusHere();
			}

			ImGui::SetNextItemWidth(220.0f);
			ImGui::InputText("Name", nameBuf, sizeof(nameBuf), ImGuiInputTextFlags_AutoSelectAll);

			DrawResourceCombo("Render Type", renderTypeIndex, EditorResourceCatalog::RenderTypes);

			const SceneRenderType selectedRenderType = EditorResourceCatalog::RenderTypeValues[renderTypeIndex];
			const bool usesModel =
				selectedRenderType != SceneRenderType::CubeMap &&
				selectedRenderType != SceneRenderType::DirectionalLight &&
				selectedRenderType != SceneRenderType::PointLight &&
				selectedRenderType != SceneRenderType::SpotLight;
			const bool usesDiffuseTexture =
				selectedRenderType == SceneRenderType::StaticMesh ||
				selectedRenderType == SceneRenderType::SkinnedMesh ||
				selectedRenderType == SceneRenderType::EquipmentMesh ||
				selectedRenderType == SceneRenderType::Billboard;
			const bool usesAnimation = selectedRenderType == SceneRenderType::SkinnedMesh;
			const bool usesPbr = selectedRenderType == SceneRenderType::PbrMesh;
			const bool isLight =
				selectedRenderType == SceneRenderType::DirectionalLight ||
				selectedRenderType == SceneRenderType::PointLight ||
				selectedRenderType == SceneRenderType::SpotLight;

			if (!usesPbr)
			{
				shadingModelIndex = static_cast<int>(MaterialShadingModel::DefaultLit);
			}
			else
			{
				DrawResourceCombo("Shading Model", shadingModelIndex, EditorResourceCatalog::ShadingModels);
				DrawHelpTooltip(
					"Default Lit: 일반 PBR Pixel Shader를 사용한다.\n"
					"Thin Film: 박막 간섭 전용 Pixel Shader를 사용한다.");
			}

			const MaterialShadingModel selectedShadingModel =
				EditorResourceCatalog::ShadingModelValues[shadingModelIndex];
			const bool supportsDeferred =
				usesPbr && selectedShadingModel == MaterialShadingModel::DefaultLit;

			// 현재 Deferred Lighting은 Default Lit 재질만 해석할 수 있다.
			// Thin Film은 b3와 전용 PS에서 최종 색상을 만들기 때문에 Forward로 고정한다.
			if (!supportsDeferred)
			{
				renderPathIndex = static_cast<int>(SceneRenderPath::Forward);
				ImGui::BeginDisabled();
			}
			const char* renderPaths[] = { "Forward", "Deferred" };
			ImGui::SetNextItemWidth(220.0f);
			ImGui::Combo("Render Path", &renderPathIndex, renderPaths, 2);
			if (!supportsDeferred)
			{
				ImGui::EndDisabled();
			}
			DrawHelpTooltip(
				"Forward: Pixel Shader에서 즉시 조명을 계산한다.\n"
				"Deferred: Geometry Pass에서 G-Buffer에 기록한다.\n"
				"현재 Deferred Geometry는 Default Lit PBR Mesh만 지원한다.");

			if (!isLight)
			{
				DrawResourceCombo("Vertex Buffer", vertexBufferIndex, EditorResourceCatalog::VertexBuffers);
			}
			if (usesModel)
			{
				DrawResourceCombo("Picking FBX", modelIndex, EditorResourceCatalog::Models);
				DrawHelpTooltip(
					"CPU 피킹/AABB/Triangle 검사에 사용할 ModelInfo를 고른다.\n"
					"None이면 Vertex Buffer 이름과 같은 ModelInfo를 사용한다.\n"
					"예: Vertex Buffer가 Box면 MyBox ModelInfo로 피킹한다.");
			}

			if (usesDiffuseTexture)
			{
				DrawResourceCombo("Texture", textureIndex, EditorResourceCatalog::Textures);
				DrawTexturePreview(EditorResourceCatalog::Textures[textureIndex].key);
			}

			if (usesAnimation)
			{
				DrawResourceCombo("Animation", animationIndex, EditorResourceCatalog::Animations);
			}

			if (usesPbr)
			{
				ImGui::SeparatorText("PBR Texture Set");
				DrawResourceCombo("Preset", pbrMaterialIndex, EditorResourceCatalog::PbrMaterials);

				if (pbrMaterialIndex == 0)
				{
					// 현재 LoadDemoAssets()는 metalball2 폴더의 PBR 텍스처를
					// albedo.png/normal.png 같은 파일명 키로 등록한다.
					pbrAlbedoIndex = 1;
					pbrNormalIndex = 2;
					pbrAOIndex = 3;
					pbrMetallicIndex = 4;
					pbrRoughnessIndex = 5;
					pbrHeightIndex = 6;
				}
				else if (pbrMaterialIndex == 1)
				{
					pbrAlbedoIndex = 0;
					pbrNormalIndex = 0;
					pbrAOIndex = 0;
					pbrMetallicIndex = 0;
					pbrRoughnessIndex = 0;
					pbrHeightIndex = 0;
				}

				const bool canEditPbrSlots = pbrMaterialIndex == 2;
				if (!canEditPbrSlots)
				{
					ImGui::BeginDisabled();
				}

				DrawResourceCombo("Albedo", pbrAlbedoIndex, EditorResourceCatalog::PbrTextures);
				DrawResourceCombo("Normal", pbrNormalIndex, EditorResourceCatalog::PbrTextures);
				DrawResourceCombo("AO", pbrAOIndex, EditorResourceCatalog::PbrTextures);
				DrawResourceCombo("Metallic", pbrMetallicIndex, EditorResourceCatalog::PbrTextures);
				DrawResourceCombo("Roughness", pbrRoughnessIndex, EditorResourceCatalog::PbrTextures);
				DrawResourceCombo("Height", pbrHeightIndex, EditorResourceCatalog::PbrTextures);

				if (!canEditPbrSlots)
				{
					ImGui::EndDisabled();
				}

				DrawPbrPreviewStrip(
					EditorResourceCatalog::PbrTextures[pbrAlbedoIndex].key,
					EditorResourceCatalog::PbrTextures[pbrNormalIndex].key,
					EditorResourceCatalog::PbrTextures[pbrMetallicIndex].key,
					EditorResourceCatalog::PbrTextures[pbrRoughnessIndex].key);

				if (selectedShadingModel == MaterialShadingModel::ThinFilm)
				{
					ImGui::SeparatorText("Thin Film");
					ImGui::DragFloat("Outside IOR", &desc.thinFilmOutsideIor, 0.01f, 1.0f, 3.0f);
					ImGui::DragFloat("Film IOR", &desc.thinFilmIor, 0.01f, 1.0f, 3.0f);
					ImGui::DragInt("Film Thickness", &desc.thinFilmThickness, 1.0f, 1, 2000);
					ImGui::SliderFloat(
						"Thickness Modulation",
						&desc.thinFilmThicknessModulation,
						0.0f,
						1.0f);
				}
			}

			if (isLight)
			{
				ImGui::SeparatorText("Light");
				ImGui::DragFloat("Strength", &desc.lightStrength, 0.05f, 0.0f, 100.0f);
				ImGui::ColorEdit3("Color", &desc.lightColor.x);
				if (selectedRenderType != SceneRenderType::DirectionalLight)
				{
					ImGui::DragFloat("Range", &desc.lightRange, 0.1f, 0.1f, 10000.0f);
				}
				if (selectedRenderType == SceneRenderType::SpotLight)
				{
					ImGui::DragFloat("Spot Power", &desc.spotPower, 0.5f, 1.0f, 256.0f);
				}
				// Point도 Cube 여섯 면에서 같은 Near/Far를 사용한다.
				// 다만 각 면의 FOV는 정확히 90도여야 빈 방향이 생기지 않으므로 별도 입력을 받지 않는다.
				if (selectedRenderType == SceneRenderType::DirectionalLight ||
					selectedRenderType == SceneRenderType::PointLight ||
					selectedRenderType == SceneRenderType::SpotLight)
				{
					ImGui::DragFloat("Shadow Near", &desc.shadowNear, 0.05f, 0.01f, 9999.0f);
					ImGui::DragFloat("Shadow Far", &desc.shadowFar, 0.5f, 0.02f, 10000.0f);
					desc.shadowNear = (std::max)(0.01f, (std::min)(desc.shadowNear, desc.shadowFar - 0.01f));
					desc.shadowFar = (std::max)(desc.shadowFar, desc.shadowNear + 0.01f);
				}
				if (selectedRenderType == SceneRenderType::DirectionalLight)
				{
					// Directional Shadow의 직교 투영 가로 범위를 월드 단위로 지정한다.
					ImGui::DragFloat("Shadow Width", &desc.shadowWidth, 0.5f, 0.1f, 10000.0f);
				}
				else if (selectedRenderType == SceneRenderType::SpotLight)
				{
					ImGui::DragFloat("Shadow FOV", &desc.shadowFovY, 0.25f, 1.0f, 179.0f);
				}
			}

			if (!isLight)
			{
				ImGui::Checkbox("Cast Shadow", &desc.castShadow);
			}
			ImGui::Spacing();

			if (ImGui::Button("Create", ImVec2(100, 0)))
			{
				desc.name = nameBuf;
				desc.vertexBufferName = isLight ? "" : EditorResourceCatalog::VertexBuffers[vertexBufferIndex].key;
				desc.modelName = usesModel ? EditorResourceCatalog::Models[modelIndex].key : "";
				desc.textureName = usesDiffuseTexture ? EditorResourceCatalog::Textures[textureIndex].key : "";
				desc.animationName = usesAnimation ? EditorResourceCatalog::Animations[animationIndex].key : "";
				desc.pbrAlbedoTextureName = usesPbr ? EditorResourceCatalog::PbrTextures[pbrAlbedoIndex].key : "";
				desc.pbrNormalTextureName = usesPbr ? EditorResourceCatalog::PbrTextures[pbrNormalIndex].key : "";
				desc.pbrAOTextureName = usesPbr ? EditorResourceCatalog::PbrTextures[pbrAOIndex].key : "";
				desc.pbrMetallicTextureName = usesPbr ? EditorResourceCatalog::PbrTextures[pbrMetallicIndex].key : "";
				desc.pbrRoughnessTextureName = usesPbr ? EditorResourceCatalog::PbrTextures[pbrRoughnessIndex].key : "";
				desc.pbrHeightTextureName = usesPbr ? EditorResourceCatalog::PbrTextures[pbrHeightIndex].key : "";
				desc.renderType = selectedRenderType;
				desc.renderPath = static_cast<SceneRenderPath>(renderPathIndex);
				desc.shadingModel = selectedShadingModel;

				if (m_createObjectCallback)
				{
					m_createObjectCallback(desc);
				}

				m_selectedIndex = (int)m_objects.size() - 1;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(100, 0)))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}

		ImGui::End();
	}

private:
	template <size_t Count>
	void DrawResourceCombo(const char* label, int& selectedIndex, const std::array<EditorResourceOption, Count>& options)
	{
		if (selectedIndex < 0 || selectedIndex >= static_cast<int>(Count))
		{
			selectedIndex = 0;
		}

		ImGui::SetNextItemWidth(220.0f);
		if (ImGui::BeginCombo(label, options[selectedIndex].label))
		{
			for (int index = 0; index < static_cast<int>(Count); ++index)
			{
				const bool selected = selectedIndex == index;
				if (ImGui::Selectable(options[index].label, selected))
				{
					selectedIndex = index;
				}
				if (selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
	}

	void DrawTexturePreview(const char* textureName)
	{
		if (!m_pGraphicsEngine || !textureName || textureName[0] == '\0')
		{
			ImGui::Dummy(ImVec2(48.0f, 48.0f));
			ImGui::SameLine();
			ImGui::TextDisabled("No preview");
			return;
		}

		auto srv = m_pGraphicsEngine->Get_Textures(textureName);
		if (!srv)
		{
			ImGui::Dummy(ImVec2(48.0f, 48.0f));
			ImGui::SameLine();
			ImGui::TextDisabled("Missing texture");
			return;
		}

		ImGui::Image(static_cast<ImTextureID>(srv.Get()), ImVec2(48.0f, 48.0f));
		ImGui::SameLine();
		ImGui::TextUnformatted(textureName);
	}

	void DrawHelpTooltip(const char* message)
	{
		ImGui::SameLine();
		ImGui::TextDisabled("?");
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(320.0f);
			ImGui::TextUnformatted(message);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	void DrawPbrPreviewStrip(const char* albedo, const char* normal, const char* metallic, const char* roughness)
	{
		ImGui::TextDisabled("Preview");
		DrawSmallPreview(albedo, "A");
		ImGui::SameLine();
		DrawSmallPreview(normal, "N");
		ImGui::SameLine();
		DrawSmallPreview(metallic, "M");
		ImGui::SameLine();
		DrawSmallPreview(roughness, "R");
	}

	void DrawSmallPreview(const char* textureName, const char* fallbackLabel)
	{
		if (!m_pGraphicsEngine || !textureName || textureName[0] == '\0')
		{
			ImGui::Button(fallbackLabel, ImVec2(36.0f, 36.0f));
			return;
		}

		auto srv = m_pGraphicsEngine->Get_Textures(textureName);
		if (!srv)
		{
			ImGui::Button(fallbackLabel, ImVec2(36.0f, 36.0f));
			return;
		}

		ImGui::Image(static_cast<ImTextureID>(srv.Get()), ImVec2(36.0f, 36.0f));
	}

	DearsGraphicsEngine* m_pGraphicsEngine = nullptr;
	std::vector<std::unique_ptr<RenderObject>>& m_objects;
	std::vector<std::string>& m_objectNames;
	int& m_selectedIndex;
	std::function<void(const SceneObjectCreateDesc&)> m_createObjectCallback;
};
