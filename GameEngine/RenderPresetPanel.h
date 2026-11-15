#pragma once

#include <cstring>
#include <functional>
#include <imgui.h>
#include <string>
#include <utility>

#include "IEditorPanel.h"
#include "RenderPreset.h"
#include "ShaderCatalog.h"

// 렌더 프리셋을 만들고 고치고, 오브젝트로 끌어다 놓는 출발점이 되는 패널이다.
// 언리얼의 콘텐츠 브라우저에서 머티리얼을 끌어 액터에 놓는 흐름과 같다.
//
// 사용법:
// 1. "+ New Preset"으로 프리셋을 만들고, 타일을 눌러 아래 편집 영역에서
//    Vertex Shader / Pixel Shader / Render Path를 정한다.
// 2. 타일을 끌어서 Scene Hierarchy의 오브젝트나 Inspector의 Render Preset 칸에 놓는다.
// 3. 프리셋을 고치면 그 프리셋을 쓰는 모든 오브젝트에 바로 반영된다.
//
// 이 패널은 라이브러리(데이터)만 고치고, 오브젝트에 다시 적용하거나 파일로 저장하는 일은
// 콜백으로 Scene에 맡긴다. 패널이 RenderObject나 씬 파일 형식을 알 필요가 없게 하기 위해서다.
class RenderPresetPanel : public IEditorPanel
{
public:
	RenderPresetPanel(
		RenderPresetLibrary& library,
		std::function<void()> libraryEditedCallback,
		std::function<void(const std::string&, const std::string&)> presetRenamedCallback)
		: m_library(library)
		, m_libraryEditedCallback(std::move(libraryEditedCallback))
		, m_presetRenamedCallback(std::move(presetRenamedCallback))
	{
	}

	const char* GetName() const override { return "Render Presets"; }

	void Draw() override
	{
		// 콘텐츠 브라우저처럼 3D 뷰 아래쪽에 깔아 둔다. 처음 한 번만 위치를 정하고 이후에는 옮긴 대로 둔다.
		const ImVec2 screenSize = ImGui::GetIO().DisplaySize;
		const float panelHeight = 230.0f;
		ImGui::SetNextWindowPos(ImVec2(0.0f, screenSize.y - panelHeight), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(screenSize.x * 0.5f, panelHeight), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Render Presets"))
		{
			ImGui::End();
			return;
		}
		// Use the DPI-scaled editor font.

		if (ImGui::Button("+ New Preset"))
		{
			SelectPreset(m_library.Add(RenderPreset{ "New Preset" }));
			NotifyLibraryEdited();
		}
		ImGui::SameLine();
		ImGui::TextDisabled("Drag a preset onto an object in Scene Hierarchy or onto the Inspector slot.");

		DrawPresetTiles();

		ImGui::Separator();
		DrawSelectedPresetEditor();

		ImGui::End();
	}

private:
	static std::string DescribeShader(const std::string& shaderName)
	{
		return shaderName.empty() ? "Auto" : shaderName;
	}

	static std::string DescribePreset(const RenderPreset& preset)
	{
		return DescribeShader(preset.vertexShader) + " + " + DescribeShader(preset.pixelShader) +
			(preset.renderPath == SceneRenderPath::Deferred ? "  (Deferred)" : "  (Forward)");
	}

	void DrawPresetTiles()
	{
		const ImVec2 tileSize(130.0f, 44.0f);
		const float spacing = ImGui::GetStyle().ItemSpacing.x;
		const float rightEdge = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

		// 타일을 그리는 동안에는 목록을 바꾸지 않으므로 인덱스로 순회해도 안전하다.
		const std::vector<RenderPreset>& presets = m_library.GetPresets();
		for (int index = 0; index < static_cast<int>(presets.size()); ++index)
		{
			const RenderPreset& preset = presets[index];
			ImGui::PushID(index);

			if (ImGui::Selectable(
				preset.name.c_str(),
				m_selectedName == preset.name,
				ImGuiSelectableFlags_None,
				tileSize))
			{
				SelectPreset(preset.name);
			}

			// 드래그 페이로드에는 프리셋 이름만 싣는다.
			// 내용을 복사해서 실으면 드롭한 뒤에 프리셋을 고쳐도 오브젝트가 따라오지 않는다.
			if (ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload(
					RenderPresetDragPayloadType,
					preset.name.c_str(),
					preset.name.size() + 1);
				ImGui::Text("%s", preset.name.c_str());
				ImGui::TextDisabled("%s", DescribePreset(preset).c_str());
				ImGui::EndDragDropSource();
			}
			else if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s", DescribePreset(preset).c_str());
			}

			// 창 폭에 맞춰 타일을 줄바꿈한다.
			const float nextTileRight = ImGui::GetItemRectMax().x + spacing + tileSize.x;
			if (index + 1 < static_cast<int>(presets.size()) && nextTileRight < rightEdge)
			{
				ImGui::SameLine();
			}

			ImGui::PopID();
		}

		if (presets.empty())
		{
			ImGui::TextDisabled("No presets yet.");
		}
	}

	void DrawSelectedPresetEditor()
	{
		const RenderPreset* selected = m_library.Find(m_selectedName);
		if (!selected)
		{
			ImGui::TextDisabled("Select a preset to edit it.");
			return;
		}

		// 편집은 복사본에 하고, 바뀐 경우에만 라이브러리에 반영한다.
		RenderPreset edited = *selected;
		bool contentEdited = false;

		ImGui::SetNextItemWidth(220.0f);
		ImGui::InputText("Name", m_nameBuffer, sizeof(m_nameBuffer));
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			const std::string newName = m_nameBuffer;
			const std::string oldName = m_selectedName;
			if (m_library.Rename(oldName, newName))
			{
				// 오브젝트는 이름으로 프리셋을 참조하므로, 참조하던 이름도 함께 바꿔 달라고 알린다.
				if (m_presetRenamedCallback)
				{
					m_presetRenamedCallback(oldName, newName);
				}
				SelectPreset(newName);
				NotifyLibraryEdited();
				return;
			}
			// 빈 이름이나 중복 이름이면 원래 이름으로 되돌린다.
			SelectPreset(oldName);
		}

		ImGui::SetNextItemWidth(220.0f);
		if (ImGui::BeginCombo("Vertex Shader", DescribeShader(edited.vertexShader).c_str()))
		{
			if (ImGui::Selectable("Auto", edited.vertexShader.empty()))
			{
				edited.vertexShader.clear();
				contentEdited = true;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Use the shader that matches each object's mesh type.");
			}
			for (const ShaderInfo& shader : ShaderCatalog::VertexShaders())
			{
				if (ImGui::Selectable(shader.name, edited.vertexShader == shader.name))
				{
					edited.vertexShader = shader.name;
					contentEdited = true;
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("%s", shader.description);
				}
			}
			ImGui::EndCombo();
		}

		// Vertex Shader를 직접 정했으면 같은 계열의 Pixel Shader만 고를 수 있다.
		// Auto면 오브젝트마다 계열이 달라질 수 있으므로 전부 보여 주고, 맞지 않는 오브젝트에서는
		// 그 오브젝트 계열의 기본 Pixel Shader로 대신 그려진다.
		const ShaderInfo* vertexShader = ShaderCatalog::FindVertexShader(edited.vertexShader);
		ImGui::SetNextItemWidth(220.0f);
		if (ImGui::BeginCombo("Pixel Shader", DescribeShader(edited.pixelShader).c_str()))
		{
			if (ImGui::Selectable("Auto", edited.pixelShader.empty()))
			{
				edited.pixelShader.clear();
				contentEdited = true;
			}
			for (const ShaderInfo& shader : ShaderCatalog::PixelShaders())
			{
				if (shader.writesGBuffer || (vertexShader && shader.linkage != vertexShader->linkage))
				{
					continue;
				}
				if (ImGui::Selectable(shader.name, edited.pixelShader == shader.name))
				{
					edited.pixelShader = shader.name;
					contentEdited = true;
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("%s", shader.description);
				}
			}
			ImGui::EndCombo();
		}

		const char* renderPathNames[] = { "Forward", "Deferred" };
		int renderPathIndex = edited.renderPath == SceneRenderPath::Deferred ? 1 : 0;
		ImGui::SetNextItemWidth(220.0f);
		if (ImGui::Combo("Render Path", &renderPathIndex, renderPathNames, 2))
		{
			edited.renderPath = renderPathIndex == 1 ? SceneRenderPath::Deferred : SceneRenderPath::Forward;
			contentEdited = true;
		}
		if (edited.renderPath == SceneRenderPath::Deferred)
		{
			ImGui::SameLine();
			ImGui::TextDisabled("PBR shaders only; others fall back to Forward.");
		}

		if (contentEdited)
		{
			m_library.Update(m_selectedName, edited);
			NotifyLibraryEdited();
		}

		if (ImGui::Button("Delete Preset"))
		{
			// 이 프리셋을 쓰던 오브젝트는 Scene이 다시 적용할 때 기본 렌더 설정으로 돌아간다.
			m_library.Remove(m_selectedName);
			m_selectedName.clear();
			NotifyLibraryEdited();
		}
	}

	void SelectPreset(const std::string& name)
	{
		m_selectedName = name;
		strncpy_s(m_nameBuffer, sizeof(m_nameBuffer), name.c_str(), _TRUNCATE);
	}

	void NotifyLibraryEdited()
	{
		if (m_libraryEditedCallback)
		{
			m_libraryEditedCallback();
		}
	}

	RenderPresetLibrary& m_library;
	std::function<void()> m_libraryEditedCallback;
	std::function<void(const std::string&, const std::string&)> m_presetRenamedCallback;
	std::string m_selectedName;
	char m_nameBuffer[64] = {};
};
