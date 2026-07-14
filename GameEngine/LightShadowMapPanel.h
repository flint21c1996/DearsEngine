#pragma once

#include <imgui.h>
#include <memory>
#include <vector>

#include "DearsGraphicsEngine.h"
#include "IEditorPanel.h"
#include "LightEnum.h"
#include "RenderObject.h"

// Hierarchy에서 선택한 라이트가 바라보는 Shadow Map을 왼쪽 아래에 보여준다.
// 선택 상태는 Scene이 소유하고, 패널은 그 인덱스를 참조만 하므로 에디터와 렌더링 데이터가 중복되지 않는다.
class LightShadowMapPanel final : public IEditorPanel
{
public:
	LightShadowMapPanel(
		DearsGraphicsEngine* graphicsEngine,
		const std::vector<std::unique_ptr<RenderObject>>& objects,
		const int& selectedObjectIndex)
		: m_graphicsEngine(graphicsEngine)
		, m_objects(objects)
		, m_selectedObjectIndex(selectedObjectIndex)
	{
	}

	const char* GetName() const override { return "Light Shadow Map"; }

	void Draw() override
	{
		const RenderObject* lightObject = GetSelectedLight();
		if (!lightObject)
		{
			return;
		}

		const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
		constexpr float panelWidth = 420.0f;
		constexpr float previewWidth = 392.0f;
		constexpr float previewHeight = previewWidth * (9.0f / 16.0f);
		constexpr float panelHeight = previewHeight + 70.0f;
		ImGui::SetNextWindowPos(
			ImVec2(12.0f, displaySize.y - panelHeight - 12.0f),
			ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

		const ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse;
		ImGui::Begin(GetName(), nullptr, flags);

		if (lightObject->mSceneLight.lightType == static_cast<UINT>(LightEnum::POINT_LIGHT))
		{
			// Point Light 그림자는 여섯 방향의 큐브 Shadow Map이 필요하다.
			// 현재 엔진은 2D Shadow Map 한 장만 지원하므로 잘못된 미리보기를 대신 보여주지 않는다.
			ImGui::TextDisabled("Point Light shadow cubemap is not implemented.");
			ImGui::End();
			return;
		}

		if (m_graphicsEngine->UIDrawShadowMapDebugPreview(
			Vector2(previewWidth, previewHeight)))
		{
			if (lightObject->mSceneLight.lightType == static_cast<UINT>(LightEnum::DIRECTIONAL_LIGHT))
			{
				ImGui::Text(
					"Near %.2f   Far %.2f   Width %.1f",
					lightObject->mSceneLight.shadowNear,
					lightObject->mSceneLight.shadowFar,
					lightObject->mSceneLight.shadowWidth);
			}
			else
			{
				ImGui::Text(
					"Near %.2f   Far %.2f   FOV %.1f",
					lightObject->mSceneLight.shadowNear,
					lightObject->mSceneLight.shadowFar,
					lightObject->mSceneLight.shadowFovY);
			}
		}
		else
		{
			ImGui::TextDisabled("Shadow preview is unavailable.");
		}

		ImGui::End();
	}

private:
	const RenderObject* GetSelectedLight() const
	{
		if (m_selectedObjectIndex < 0 ||
			static_cast<size_t>(m_selectedObjectIndex) >= m_objects.size())
		{
			return nullptr;
		}

		const RenderObject* object = m_objects[m_selectedObjectIndex].get();
		return object && object->mIsLight ? object : nullptr;
	}

	DearsGraphicsEngine* m_graphicsEngine = nullptr;
	const std::vector<std::unique_ptr<RenderObject>>& m_objects;
	const int& m_selectedObjectIndex;
};
