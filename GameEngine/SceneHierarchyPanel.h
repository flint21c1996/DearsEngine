#pragma once
#include <imgui.h>
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

class SceneHierarchyPanel : public IEditorPanel
{
public:
	SceneHierarchyPanel(
		std::vector<std::unique_ptr<RenderObject>>& objects,
		int& selectedIndex,
		DearsGraphicsEngine* pEngine)
		: m_objects(objects)
		, m_selectedIndex(selectedIndex)
		, m_pEngine(pEngine)
	{}

	const char* GetName() const override { return "Scene Hierarchy"; }

	void RegisterName(const std::string& name) { m_names.push_back(name); }

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
			const std::string& label = (i < (int)m_names.size())
				? m_names[i]
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
			static char nameBuf[128] = "New Object";
			ImGui::SetNextItemWidth(220.0f);
			ImGui::InputText("Name", nameBuf, sizeof(nameBuf));
			ImGui::Spacing();

			if (ImGui::Button("Create", ImVec2(100, 0)))
			{
				auto obj = std::make_unique<RenderObject>(m_pEngine);
				obj->Initialize();
				m_objects.push_back(std::move(obj));
				m_names.push_back(nameBuf);
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
	int& m_selectedIndex;
	DearsGraphicsEngine* m_pEngine;
	std::vector<std::string> m_names;
};
