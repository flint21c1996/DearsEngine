#pragma once
#include <algorithm>
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

			float rotation[3] = {
				obj->mEditorRotationDegrees.x,
				obj->mEditorRotationDegrees.y,
				obj->mEditorRotationDegrees.z };
			if (ImGui::DragFloat3("Rotation", rotation, 0.25f))
			{
				obj->mEditorRotationDegrees = Vector3(rotation[0], rotation[1], rotation[2]);
				// UI는 degree를 사용하지만 SimpleMath 회전 함수는 radian을 받는다.
				obj->ObjectRot = Matrix::CreateFromYawPitchRoll(
					DirectX::XMConvertToRadians(rotation[1]),
					DirectX::XMConvertToRadians(rotation[0]),
					DirectX::XMConvertToRadians(rotation[2]));
				NotifyObjectEdited();
			}

			// Unreal의 Scale 자물쇠와 같은 역할이다.
			// 활성화하면 한 축의 변화 비율을 나머지 두 축에도 적용하므로,
			// 이미 비균등한 Scale인 오브젝트도 현재 비율을 유지하며 커지고 작아진다.
			ImGui::Checkbox("Uniform Scale", &m_uniformScale);
			const float oldScale[3] = { obj->ObjectScl._11, obj->ObjectScl._22, obj->ObjectScl._33 };
			float scl[3] = { oldScale[0], oldScale[1], oldScale[2] };
			if (ImGui::DragFloat3("Scale", scl, 0.01f, 0.001f, 1000.0f))
			{
				if (m_uniformScale)
				{
					for (int axis = 0; axis < 3; ++axis)
					{
						if (scl[axis] == oldScale[axis])
						{
							continue;
						}

						const float ratio = oldScale[axis] != 0.0f
							? scl[axis] / oldScale[axis]
							: 1.0f;
						scl[0] = oldScale[0] * ratio;
						scl[1] = oldScale[1] * ratio;
						scl[2] = oldScale[2] * ratio;
						break;
					}
				}
				obj->ObjectScl = Matrix::CreateScale(scl[0], scl[1], scl[2]);
				NotifyObjectEdited();
			}
		}

		ImGui::Spacing();

		if (obj->mIsLight && ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
		{
			bool lightEditFinished = false;
			ImGui::DragFloat("Strength", &obj->mSceneLight.strength, 0.05f, 0.0f, 100.0f);
			lightEditFinished |= ImGui::IsItemDeactivatedAfterEdit();
			lightEditFinished |= ImGui::ColorEdit3("Color", &obj->mSceneLight.lightColor.x);
			if (obj->mSceneLight.lightType != static_cast<UINT>(LightEnum::DIRECTIONAL_LIGHT))
			{
				ImGui::DragFloat("Range", &obj->mSceneLight.fallOffEnd, 0.1f, 0.1f, 10000.0f);
				lightEditFinished |= ImGui::IsItemDeactivatedAfterEdit();
			}
			if (obj->mSceneLight.lightType == static_cast<UINT>(LightEnum::SPOT_LIGHT))
			{
				ImGui::DragFloat("Spot Power", &obj->mSceneLight.spotPower, 0.5f, 1.0f, 256.0f);
				lightEditFinished |= ImGui::IsItemDeactivatedAfterEdit();
			}

			// 두 라이트 모두 Near/Far를 사용하지만, Directional은 Width, Spot은 FOV로 범위를 정한다.
			if (obj->mSceneLight.lightType == static_cast<UINT>(LightEnum::DIRECTIONAL_LIGHT) ||
				obj->mSceneLight.lightType == static_cast<UINT>(LightEnum::SPOT_LIGHT))
			{
				ImGui::DragFloat("Shadow Near", &obj->mSceneLight.shadowNear, 0.05f, 0.01f, 9999.0f);
				lightEditFinished |= ImGui::IsItemDeactivatedAfterEdit();
				ImGui::DragFloat("Shadow Far", &obj->mSceneLight.shadowFar, 0.5f, 0.02f, 10000.0f);
				lightEditFinished |= ImGui::IsItemDeactivatedAfterEdit();

				// Near가 Far를 넘어가면 투영행렬이 뒤집히므로 항상 작은 간격을 보장한다.
				obj->mSceneLight.shadowNear = (std::max)(0.01f,
					(std::min)(obj->mSceneLight.shadowNear, obj->mSceneLight.shadowFar - 0.01f));
				obj->mSceneLight.shadowFar = (std::max)(
					obj->mSceneLight.shadowFar, obj->mSceneLight.shadowNear + 0.01f);
			}
			if (obj->mSceneLight.lightType == static_cast<UINT>(LightEnum::DIRECTIONAL_LIGHT))
			{
				// Directional은 직교 투영이므로 FOV 대신 그림자를 담을 월드 가로 범위를 조절한다.
				ImGui::DragFloat("Shadow Width", &obj->mSceneLight.shadowWidth, 0.5f, 0.1f, 10000.0f);
				lightEditFinished |= ImGui::IsItemDeactivatedAfterEdit();
			}
			else if (obj->mSceneLight.lightType == static_cast<UINT>(LightEnum::SPOT_LIGHT))
			{
				ImGui::DragFloat("Shadow FOV", &obj->mSceneLight.shadowFovY, 0.25f, 1.0f, 179.0f);
				lightEditFinished |= ImGui::IsItemDeactivatedAfterEdit();
			}
			if (lightEditFinished)
			{
				NotifyObjectEdited();
			}
		}

		ImGui::Spacing();

		// --- Resources ---
		if (!obj->mIsLight && ImGui::CollapsingHeader("Resources", ImGuiTreeNodeFlags_DefaultOpen))
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
				// Shading Model은 Render Flag처럼 버퍼 존재 여부를 직접 뒤집는 옵션이 아니다.
				// SetShadingModel()을 통과시키면 Thin Film에 필요한 b3 버퍼가 없을 때 안전하게 생성된다.
				const char* shadingModelNames[] = { "Default Lit", "Thin Film" };
				int shadingModelIndex = static_cast<int>(obj->GetShadingModel());
				if (ImGui::Combo("Shading Model", &shadingModelIndex, shadingModelNames, 2))
				{
					obj->SetShadingModel(static_cast<MaterialShadingModel>(shadingModelIndex));
					NotifyObjectEdited();
				}

				DrawTextureRow("Albedo", obj->mEditorPbrAlbedoTextureName);
				DrawTextureRow("Normal", obj->mEditorPbrNormalTextureName);
				DrawTextureRow("AO", obj->mEditorPbrAOTextureName);
				DrawTextureRow("Metallic", obj->mEditorPbrMetallicTextureName);
				DrawTextureRow("Roughness", obj->mEditorPbrRoughnessTextureName);
				DrawTextureRow("Height", obj->mEditorPbrHeightTextureName);
				ImGui::Separator();
				// 슬라이더 값은 GPU에 전달되는 런타임 설정인 동시에 Scene에 보존해야 하는 에디터 데이터다.
				// 하나라도 변경되면 Scene 저장 콜백을 호출하여 다음 실행에서도 같은 값이 복원되게 한다.
				bool materialEditFinished = false;
				ImGui::SliderFloat("Metallic", &obj->mPSPBRConstantBufferData.material.metallic, 0.0f, 1.0f);
				materialEditFinished |= ImGui::IsItemDeactivatedAfterEdit();
				ImGui::SliderFloat("Roughness", &obj->mPSPBRConstantBufferData.material.roughness, 0.0f, 1.0f);
				materialEditFinished |= ImGui::IsItemDeactivatedAfterEdit();
				ImGui::SliderFloat("Height Scale", &obj->mVSPBRConstantBufferData.heightScale, 0.0f, 0.1f);
				materialEditFinished |= ImGui::IsItemDeactivatedAfterEdit();
				if (materialEditFinished)
				{
					NotifyObjectEdited();
				}

				if (obj->GetShadingModel() == MaterialShadingModel::ThinFilm)
				{
					ImGui::SeparatorText("Thin Film");
					bool thinFilmEditFinished = false;
					ImGui::DragFloat(
						"Outside IOR",
						&obj->mPSThinFilmConstantBufferData.n1,
						0.01f,
						1.0f,
						3.0f);
					thinFilmEditFinished |= ImGui::IsItemDeactivatedAfterEdit();
					ImGui::DragFloat(
						"Film IOR",
						&obj->mPSThinFilmConstantBufferData.n2,
						0.01f,
						1.0f,
						3.0f);
					thinFilmEditFinished |= ImGui::IsItemDeactivatedAfterEdit();
					ImGui::DragInt(
						"Film Thickness",
						&obj->mPSThinFilmConstantBufferData.d,
						1.0f,
						1,
						2000);
					thinFilmEditFinished |= ImGui::IsItemDeactivatedAfterEdit();
					ImGui::SliderFloat(
						"Thickness Modulation",
						&obj->mPSThinFilmConstantBufferData.time,
						0.0f,
						1.0f);
					thinFilmEditFinished |= ImGui::IsItemDeactivatedAfterEdit();
					if (thinFilmEditFinished)
					{
						NotifyObjectEdited();
					}
				}
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
	bool m_uniformScale = true;
};
