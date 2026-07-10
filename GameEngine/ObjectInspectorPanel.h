#pragma once
#include <imgui.h>
#include <functional>
#include <vector>
#include <memory>
#include <string>
#include "DearsGraphicsEngine.h"
#include "EditorResourceCatalog.h"
#include "IEditorPanel.h"
#include "RenderObject.h"
#include "SceneHierarchyPanel.h"

using DirectX::SimpleMath::Matrix;

class ObjectInspectorPanel : public IEditorPanel
{
public:
	ObjectInspectorPanel(
		DearsGraphicsEngine* graphicsEngine,
		std::vector<std::unique_ptr<RenderObject>>& objects,
		int& selectedIndex,
		std::function<void()> objectEditedCallback = nullptr)
		: m_pGraphicsEngine(graphicsEngine)
		, m_objects(objects)
		, m_selectedIndex(selectedIndex)
		, m_objectEditedCallback(objectEditedCallback)
	{}

	const char* GetName() const override { return "Inspector"; }

	void Draw() override
	{
		ImVec2 screenSize = ImGui::GetIO().DisplaySize;
		float panelW = static_cast<float>(GetEditorPanelWidth(screenSize.x));
		float topH    = screenSize.y * 0.45f + 110.0f;

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
			{
				obj->ObjectPos = Matrix::CreateTranslation(pos[0], pos[1], pos[2]);
				NotifyObjectEdited();
			}

			float scl[3] = { obj->ObjectScl._11, obj->ObjectScl._22, obj->ObjectScl._33 };
			if (ImGui::DragFloat3("Scale", scl, 0.01f, 0.001f, 1000.0f))
			{
				obj->ObjectScl = Matrix::CreateScale(scl[0], scl[1], scl[2]);
				NotifyObjectEdited();
			}
		}

		ImGui::Spacing();

		// --- Resources ---
		if (ImGui::CollapsingHeader("Resources", ImGuiTreeNodeFlags_DefaultOpen))
		{
			DrawTextRow("Vertex Buffer", obj->mEditorVertexBufferName);
			DrawTextRow("Picking FBX", obj->mEditorModelName.empty() ? obj->mEditorVertexBufferName : obj->mEditorModelName);

			if (!obj->mEditorDiffuseTextureName.empty())
			{
				DrawTextureRow("Diffuse", obj->mEditorDiffuseTextureName);
			}

			if (!obj->mEditorAnimationName.empty())
			{
				DrawTextRow("Animation", obj->mEditorAnimationName);
			}
		}

		ImGui::Spacing();

		// --- PBR Material ---
		if (obj->mIs_VSPBRConstant || obj->mIs_PSPBRConstant)
		{
			if (ImGui::CollapsingHeader("PBR Material", ImGuiTreeNodeFlags_DefaultOpen))
			{
				DrawTextureRow("Albedo", obj->mEditorPbrAlbedoTextureName);
				DrawTextureRow("Normal", obj->mEditorPbrNormalTextureName);
				DrawTextureRow("AO", obj->mEditorPbrAOTextureName);
				DrawTextureRow("Metallic", obj->mEditorPbrMetallicTextureName);
				DrawTextureRow("Roughness", obj->mEditorPbrRoughnessTextureName);
				DrawTextureRow("Height", obj->mEditorPbrHeightTextureName);
				ImGui::Separator();
				ImGui::SliderFloat("Metallic",    &obj->mPSPBRConstantBufferData.material.metallic,  0.0f, 1.0f);
				ImGui::SliderFloat("Roughness",   &obj->mPSPBRConstantBufferData.material.roughness, 0.0f, 1.0f);
				ImGui::SliderFloat("Height Scale",&obj->mVSPBRConstantBufferData.heightScale, 0.0f, 0.1f);
			}
		}

		// --- Render Flags ---
		if (ImGui::CollapsingHeader("Render Flags"))
		{
			// 이 값들은 단순한 UI 옵션이 아니라 실제 GPU constant buffer가 생성되었는지와 연결된다.
			// 체크박스로 bool만 바꾸면 버퍼가 없는 상태에서 UpdateBuffer()가 호출되어 프로그램이 터진다.
			// 그래서 지금은 읽기 전용 상태 표시로 두고, 렌더 경로 변경은 Add Object/전용 설정 UI에서 처리한다.
			ImGui::BeginDisabled();
			ImGui::Checkbox("VS Constant",   &obj->mIs_VSconstant);
			ImGui::Checkbox("VS Bone",       &obj->mIs_VSBoneConstant);
			ImGui::Checkbox("PS Constant",   &obj->mIs_PSconstant);
			ImGui::Checkbox("Shadow",        &obj->mIs_VSShadowConstant);
			ImGui::Checkbox("Edge",          &obj->mIs_VSEdgeConstant);
			ImGui::Checkbox("Water",         &obj->mIs_VSWaterConstant);
			ImGui::Checkbox("PBR VS",        &obj->mIs_VSPBRConstant);
			ImGui::Checkbox("PBR PS",        &obj->mIs_PSPBRConstant);
			ImGui::Checkbox("Thin Film",     &obj->mIs_PSThinFilmConstant);
			ImGui::EndDisabled();
		}

		ImGui::End();
	}

private:
	void DrawTextRow(const char* label, const std::string& value)
	{
		ImGui::TextDisabled("%s", label);
		ImGui::SameLine(90.0f);
		ImGui::TextUnformatted(value.empty() ? "None" : value.c_str());
	}

	void DrawTextureRow(const char* label, const std::string& textureName)
	{
		ImGui::TextDisabled("%s", label);
		ImGui::SameLine(90.0f);

		if (!m_pGraphicsEngine || textureName.empty())
		{
			ImGui::TextDisabled("None");
			return;
		}

		auto srv = m_pGraphicsEngine->Get_Textures(textureName);
		if (!srv)
		{
			ImGui::TextDisabled("%s (missing)", textureName.c_str());
			return;
		}

		ImGui::Image(static_cast<ImTextureID>(srv.Get()), ImVec2(28.0f, 28.0f));
		ImGui::SameLine();
		ImGui::TextUnformatted(textureName.c_str());
	}

	void NotifyObjectEdited()
	{
		if (m_objectEditedCallback)
		{
			m_objectEditedCallback();
		}
	}

	DearsGraphicsEngine* m_pGraphicsEngine = nullptr;
	std::vector<std::unique_ptr<RenderObject>>& m_objects;
	int& m_selectedIndex;
	std::function<void()> m_objectEditedCallback;
};
