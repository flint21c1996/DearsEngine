#pragma once
#include <imgui.h>
#include <functional>
#include <cstring>
#include <vector>
#include <memory>
#include <string>
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
	SceneRenderType renderType = SceneRenderType::StaticMesh;
	bool castShadow = true;
};

class SceneHierarchyPanel : public IEditorPanel
{
public:
	SceneHierarchyPanel(
		std::vector<std::unique_ptr<RenderObject>>& objects,
		std::vector<std::string>& objectNames,
		int& selectedIndex,
		std::function<void(const SceneObjectCreateDesc&)> createObjectCallback)
		: m_objects(objects)
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
			static int renderTypeIndex = static_cast<int>(SceneRenderType::StaticMesh);

			// 아래 이름들은 현재 LoadDemoAssets()에서 미리 로드한 리소스 키다.
			//
			// Vertex Buffer:
			// - FBX를 로드하면 mesh 이름으로 vertex/index buffer가 등록된다.
			// - 예: Rock_05.fbx 안의 mesh 이름이 Rock_05라면 Vertex Buffer 키는 "Rock_05"다.
			// - MyBox/MySphere/MySquare 같은 이름은 FBX가 아니라 GeometryGenerator가 만든 기본 도형이다.
			//
			// Model FBX / Texture / Animation:
			// - AddModel/AddTexture/AddAnimation에 넘긴 파일명이 그대로 리소스 키가 된다.
			// - 예: AddModel(..., "Rock_05.fbx") -> Model FBX 키 "Rock_05.fbx"
			// - 기본 도형은 Vertex Buffer 이름과 같은 ModelInfo를 내부에서 만들기 때문에 Model FBX를 None으로 둔다.
			const char* vertexBufferLabels[] =
			{
				"MyBox",
				"MySphere",
				"MySquare",
				"BillBoardSquare",
				"CubeMap",
				"Rock_05",
				"Character 01",
				"Axe 01"
			};

			const char* modelLabels[] =
			{
				"None",
				"Rock_05.fbx",
				"Character 01.FBX",
				"Axe 01.fbx",
				"Hat 04.FBX",
				"Michelle.fbx",
				"Sung.fbx"
			};

			const char* modelKeys[] =
			{
				"",
				"Rock_05.fbx",
				"Character 01.FBX",
				"Axe 01.fbx",
				"Hat 04.FBX",
				"Michelle.fbx",
				"Sung.fbx"
			};

			const char* textureLabels[] =
			{
				"White.png",
				"Red.png",
				"Yellow.png",
				"Blue.png",
				"ss.png",
				"Water.png",
				"albedo.png",
				"pngegg.png"
			};

			const char* animationLabels[] =
			{
				"",
				"Character@Slash Attack.FBX",
				"Character@Idle.FBX",
				"Character@Run Forward Bare Hands In Place.FBX",
				"CatwalkIdle.fbx",
				"CatwalkWalkForward.fbx"
			};

			const char* renderTypeLabels[] =
			{
				"StaticMesh",
				"SkinnedMesh",
				"EquipmentMesh",
				"CubeMap",
				"Billboard",
				"PbrMesh"
			};

			if (ImGui::IsWindowAppearing())
			{
				// 팝업을 열자마자 이름 입력칸에 포커스를 준다.
				// 이렇게 해두면 Add Object를 누른 뒤 바로 타이핑해서 이름을 바꿀 수 있다.
				ImGui::SetKeyboardFocusHere();
			}

			ImGui::SetNextItemWidth(220.0f);
			ImGui::InputText("Name", nameBuf, sizeof(nameBuf), ImGuiInputTextFlags_AutoSelectAll);
			ImGui::SetNextItemWidth(220.0f);
			ImGui::Combo("Render Type", &renderTypeIndex, renderTypeLabels, IM_ARRAYSIZE(renderTypeLabels));
			ImGui::SetNextItemWidth(220.0f);
			ImGui::Combo("Vertex Buffer", &vertexBufferIndex, vertexBufferLabels, IM_ARRAYSIZE(vertexBufferLabels));
			ImGui::SetNextItemWidth(220.0f);
			ImGui::Combo("Model FBX", &modelIndex, modelLabels, IM_ARRAYSIZE(modelLabels));
			ImGui::SetNextItemWidth(220.0f);
			ImGui::Combo("Texture", &textureIndex, textureLabels, IM_ARRAYSIZE(textureLabels));
			ImGui::SetNextItemWidth(220.0f);
			ImGui::Combo("Animation", &animationIndex, animationLabels, IM_ARRAYSIZE(animationLabels));
			ImGui::Checkbox("Cast Shadow", &desc.castShadow);
			ImGui::Spacing();

			if (ImGui::Button("Create", ImVec2(100, 0)))
			{
				desc.name = nameBuf;
				desc.vertexBufferName = vertexBufferLabels[vertexBufferIndex];
				desc.modelName = modelKeys[modelIndex];
				desc.textureName = textureLabels[textureIndex];
				desc.animationName = animationLabels[animationIndex];
				desc.renderType = static_cast<SceneRenderType>(renderTypeIndex);

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
	std::vector<std::unique_ptr<RenderObject>>& m_objects;
	std::vector<std::string>& m_objectNames;
	int& m_selectedIndex;
	std::function<void(const SceneObjectCreateDesc&)> m_createObjectCallback;
};
