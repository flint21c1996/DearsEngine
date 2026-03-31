#pragma once
#include <imgui.h>
#include <vector>
#include <memory>
#include <string>
#include "IEditorPanel.h"
#include "RenderObject.h"
#include "SceneHierarchyPanel.h"

using DirectX::SimpleMath::Matrix;

class ObjectInspectorPanel : public IEditorPanel
{
public:
	ObjectInspectorPanel(
		std::vector<std::unique_ptr<RenderObject>>& objects,
		int& selectedIndex)
		: m_objects(objects)
		, m_selectedIndex(selectedIndex)
	{}

	const char* GetName() const override { return "Inspector"; }

	void Draw() override
	{
		ImVec2 screenSize = ImGui::GetIO().DisplaySize;
		float panelW = static_cast<float>(GetEditorPanelWidth(screenSize.x));
		float topH    = screenSize.y * 0.45f;

		ImGui::SetNextWindowPos(ImVec2(screenSize.x - panelW, topH), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(panelW, screenSize.y - topH), ImGuiCond_Always);

		ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse;

		ImGui::Begin("Inspector", nullptr, flags);
		ImGui::SetWindowFontScale(0.8f);

		if (m_selectedIndex < 0 || m_selectedIndex >= (int)m_objects.size())
		{
			ImGui::TextDisabled("No object selected.");
			ImGui::End();
			return;
		}

		RenderObject* obj = m_objects[m_selectedIndex].get();

		ImGui::Text("Object [%d]", m_selectedIndex);
		ImGui::Separator();
		ImGui::Spacing();

		// --- Transform ---
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			float pos[3] = { obj->ObjectPos._41, obj->ObjectPos._42, obj->ObjectPos._43 };
			if (ImGui::DragFloat3("Position", pos, 0.1f))
				obj->ObjectPos = Matrix::CreateTranslation(pos[0], pos[1], pos[2]);

			float scl[3] = { obj->ObjectScl._11, obj->ObjectScl._22, obj->ObjectScl._33 };
			if (ImGui::DragFloat3("Scale", scl, 0.01f, 0.001f, 1000.0f))
				obj->ObjectScl = Matrix::CreateScale(scl[0], scl[1], scl[2]);
		}

		ImGui::Spacing();

		// --- PBR Material ---
		if (obj->mIs_VSPBRConstant || obj->mIs_PSPBRConstant)
		{
			if (ImGui::CollapsingHeader("PBR Material"))
			{
				ImGui::SliderFloat("Metallic",    &obj->mPSPBRConstantBufferData.material.metallic,  0.0f, 1.0f);
				ImGui::SliderFloat("Roughness",   &obj->mPSPBRConstantBufferData.material.roughness, 0.0f, 1.0f);
				ImGui::SliderFloat("Height Scale",&obj->mVSPBRConstantBufferData.heightScale, 0.0f, 0.1f);
			}
		}

		// --- Render Flags ---
		if (ImGui::CollapsingHeader("Render Flags"))
		{
			ImGui::Checkbox("VS Constant",   &obj->mIs_VSconstant);
			ImGui::Checkbox("VS Bone",       &obj->mIs_VSBoneConstant);
			ImGui::Checkbox("PS Constant",   &obj->mIs_PSconstant);
			ImGui::Checkbox("Shadow",        &obj->mIs_VSShadowConstant);
			ImGui::Checkbox("Edge",          &obj->mIs_VSEdgeConstant);
			ImGui::Checkbox("Water",         &obj->mIs_VSWaterConstant);
			ImGui::Checkbox("PBR VS",        &obj->mIs_VSPBRConstant);
			ImGui::Checkbox("PBR PS",        &obj->mIs_PSPBRConstant);
			ImGui::Checkbox("Thin Film",     &obj->mIs_PSThinFilmConstant);
		}

		ImGui::End();
	}

private:
	std::vector<std::unique_ptr<RenderObject>>& m_objects;
	int& m_selectedIndex;
};
