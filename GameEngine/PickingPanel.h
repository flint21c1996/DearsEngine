#pragma once

#include <imgui.h>

#include "IEditorPanel.h"
#include "PickingManager.h"
#include "SceneHierarchyPanel.h"

class PickingPanel : public IEditorPanel
{
public:
	explicit PickingPanel(PickingManager& pickingManager)
		: m_pickingManager(pickingManager)
	{
	}

	const char* GetName() const override { return "Picking"; }

	void Draw() override
	{
		ImVec2 screenSize = ImGui::GetIO().DisplaySize;
		float panelW = static_cast<float>(GetEditorPanelWidth(screenSize.x));
		float y = screenSize.y * 0.45f;
		float h = 110.0f;

		ImGui::SetNextWindowPos(ImVec2(screenSize.x - panelW, y), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(panelW, h), ImGuiCond_FirstUseEver);

		ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoCollapse;

		ImGui::Begin("Picking", nullptr, flags);
		// Use the DPI-scaled editor font.

		int mode = static_cast<int>(m_pickingManager.GetMode());
		if (ImGui::RadioButton("AABB", mode == static_cast<int>(PickingMode::AABB)))
		{
			m_pickingManager.SetMode(PickingMode::AABB);
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("OBB", mode == static_cast<int>(PickingMode::OBB)))
		{
			m_pickingManager.SetMode(PickingMode::OBB);
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Triangle", mode == static_cast<int>(PickingMode::Triangle)))
		{
			m_pickingManager.SetMode(PickingMode::Triangle);
		}

		const PickingResult& result = m_pickingManager.GetLastResult();
		if (result.hit)
		{
			ImGui::Text("Hit Object: %d", result.objectIndex);
			ImGui::Text("Distance: %.2f", result.distance);
		}
		else
		{
			ImGui::TextDisabled("No picking hit.");
		}

		ImGui::End();
	}

private:
	PickingManager& m_pickingManager;
};
