#pragma once

#include <functional>
#include <imgui.h>
#include <string>
#include <utility>
#include <vector>

#include "IDearsRenderer.h"
#include "IEditorPanel.h"
#include "RenderPipeline.h"

// 한 프레임을 구성하는 렌더 패스 목록을 실행 중에 보고 바꾸는 에디터 패널이다.
//
// - 체크박스: 패스를 켜고 끈다. (예: Particle을 꺼서 비용 확인, Shadow를 꺼서 그림자 없는 화면 확인)
// - 위/아래 버튼: 패스의 실행 순서를 바꾼다.
// - Reset: 게임 엔진이 등록한 기본 구성으로 되돌린다.
//
// 패널은 프레임 시작(BeginRender)에서 그려지고 패스는 그 뒤에 실행되므로,
// 여기서 순서를 바꿔도 "실행 중에는 목록 구조를 바꿀 수 없다"는 RenderPipeline 규칙에 걸리지 않는다.
class RenderPipelinePanel : public IEditorPanel
{
public:
	RenderPipelinePanel(IDearsRenderer* renderer, std::function<void()> resetCallback)
		: m_pRenderer(renderer)
		, m_resetCallback(std::move(resetCallback))
	{
	}

	const char* GetName() const override { return "Render Pipeline"; }

	void Draw() override
	{
		if (!m_pRenderer)
		{
			return;
		}

		// 기본 배치에서는 아래쪽 도킹 영역의 탭으로 들어간다. (GameEngine::InitializeEditorPanels)
		// 아래 값은 도킹에서 떼어 냈을 때의 처음 위치/크기이다.
		ImGui::SetNextWindowPos(ImVec2(220.0f, 60.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(360.0f, 320.0f), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Render Pipeline"))
		{
			ImGui::End();
			return;
		}
		// Use the DPI-scaled editor font.

		RenderPipeline& pipeline = m_pRenderer->GetRenderPipeline();
		const std::vector<std::string> names = pipeline.GetPassNames();

		// 순서 변경은 목록을 다 그린 뒤에 한 번만 적용한다.
		// 그리는 도중에 옮기면 같은 프레임에 한 패스가 두 번 보이거나 빠질 수 있다.
		int moveFrom = -1;
		int moveTo = -1;

		for (int index = 0; index < static_cast<int>(names.size()); ++index)
		{
			const std::string& name = names[index];
			IRenderPass* pass = pipeline.FindPass(name);
			if (!pass)
			{
				continue;
			}

			ImGui::PushID(index);

			bool enabled = pass->IsEnabled();
			if (ImGui::Checkbox("##enabled", &enabled))
			{
				pass->SetEnabled(enabled);
			}

			ImGui::SameLine();
			ImGui::BeginDisabled(index == 0);
			if (ImGui::ArrowButton("##up", ImGuiDir_Up))
			{
				moveFrom = index;
				moveTo = index - 1;
			}
			ImGui::EndDisabled();

			ImGui::SameLine();
			ImGui::BeginDisabled(index + 1 >= static_cast<int>(names.size()));
			if (ImGui::ArrowButton("##down", ImGuiDir_Down))
			{
				moveFrom = index;
				moveTo = index + 1;
			}
			ImGui::EndDisabled();

			ImGui::SameLine();
			ImGui::Text("%d. %s", index + 1, name.c_str());
			ImGui::SameLine();
			ImGui::TextDisabled("(%s)", ToRenderPassName(pass->GetType()));

			ImGui::PopID();
		}

		if (moveFrom >= 0 && moveTo >= 0)
		{
			if (moveTo < moveFrom)
			{
				pipeline.MovePassBefore(names[moveFrom], names[moveTo]);
			}
			else
			{
				pipeline.MovePassAfter(names[moveFrom], names[moveTo]);
			}
		}

		ImGui::Separator();
		if (m_resetCallback && ImGui::Button("Reset"))
		{
			m_resetCallback();
		}
		ImGui::SameLine();
		ImGui::TextDisabled("%d passes", static_cast<int>(names.size()));

		// 패스끼리는 순서 의존성이 있다. 막지는 않고 이유를 알려 준다.
		ImGui::TextWrapped(
			"Order matters: Geometry must run before Lighting, "
			"and Lighting before Forward (it shares the geometry depth).");

		ImGui::End();
	}

private:
	IDearsRenderer* m_pRenderer = nullptr;
	std::function<void()> m_resetCallback;
};
