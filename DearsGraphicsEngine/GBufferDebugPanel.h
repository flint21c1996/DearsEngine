#pragma once

#include <algorithm>
#include <array>
#include <imgui.h>

#include "GBuffer.h"
#include "GBufferDebugRenderer.h"
#include "IEditorPanel.h"

// Geometry Pass가 기록한 각 G-Buffer를 최종 화면 위에 나란히 보여주는 디버그 패널이다.
//
// 이 패널은 리소스를 생성하거나 수정하지 않고 GBuffer가 소유한 SRV를 읽기만 한다.
// 따라서 디퍼드 렌더링 결과가 이상할 때 문제가 Geometry 기록인지 Lighting 계산인지
// 눈으로 빠르게 분리해서 확인할 수 있다.
class GBufferDebugPanel final : public IEditorPanel
{
public:
	GBufferDebugPanel(GBuffer& gBuffer, GBufferDebugRenderer& debugRenderer)
		: m_gBuffer(gBuffer)
		, m_debugRenderer(debugRenderer)
	{
	}

	const char* GetName() const override { return "Deferred G-Buffer"; }

	void Draw() override
	{
		const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
		const float renderWidth = displaySize.x * (5.0f / 6.0f);
		const float panelMargin = 12.0f;
		const float panelHeight = 330.0f;

		// 최종 화면을 가리지 않도록 3D 뷰포트 하단에 얕은 가로 패널로 배치한다.
		ImGui::SetNextWindowPos(
			ImVec2(panelMargin, displaySize.y - panelHeight - panelMargin),
			ImGuiCond_Always);
		ImGui::SetNextWindowSize(
			ImVec2(renderWidth - panelMargin * 2.0f, panelHeight),
			ImGuiCond_Always);

		const ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse;

		ImGui::Begin(GetName(), nullptr, flags);
		ImGui::SetWindowFontScale(0.8f);

		const std::array<const char*, 7> labels = {
			"Albedo", "World Normal", "Metallic", "Roughness", "AO", "Depth", "World Position"
		};
		const std::array<ID3D11ShaderResourceView*, 7> views = {
			m_gBuffer.GetShaderResourceView(GBuffer::Target::Albedo),
			m_gBuffer.GetShaderResourceView(GBuffer::Target::Normal),
			m_debugRenderer.GetView(GBufferDebugRenderer::View::Metallic),
			m_debugRenderer.GetView(GBufferDebugRenderer::View::Roughness),
			m_debugRenderer.GetView(GBufferDebugRenderer::View::AO),
			m_debugRenderer.GetView(GBufferDebugRenderer::View::LinearDepth),
			m_debugRenderer.GetView(GBufferDebugRenderer::View::WorldPosition),
		};

		const float spacing = ImGui::GetStyle().ItemSpacing.x;
		const float previewWidth = (ImGui::GetContentRegionAvail().x - spacing * 3.0f) / 4.0f;
		// 디버그 텍스처의 실제 해상도와 관계없이 사람이 비교하기 익숙한 16:9로 표시한다.
		// 모든 항목이 같은 비율을 사용하므로 행 사이 높이와 정렬도 안정적으로 유지된다.
		const float previewHeight = previewWidth * (9.0f / 16.0f);

		for (size_t index = 0; index < views.size(); ++index)
		{
			if (index > 0 && index % 4 != 0)
			{
				ImGui::SameLine();
			}

			ImGui::BeginGroup();
			ImGui::TextUnformatted(labels[index]);
			if (views[index])
			{
				ImGui::Image(
					reinterpret_cast<ImTextureID>(views[index]),
					ImVec2(previewWidth, previewHeight));
			}
			else
			{
				ImGui::Dummy(ImVec2(previewWidth, previewHeight));
				ImGui::TextDisabled("Unavailable");
			}
			ImGui::EndGroup();
		}

		ImGui::End();
	}

private:
	GBuffer& m_gBuffer;
	GBufferDebugRenderer& m_debugRenderer;
};
