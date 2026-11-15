#pragma once

#include <imgui.h>
#include <string>

#include "DearsGraphicsEngine.h"
#include "EditorResourceCatalog.h"
#include "IEditorPanel.h"

// 콘텐츠 브라우저에서 끌어 온 항목의 ImGui 드래그 페이로드 이름이다.
// 페이로드 내용은 널 문자로 끝나는 리소스 키이다. (메시는 Vertex Buffer 이름, 텍스처는 파일 이름)
constexpr const char* MeshAssetDragPayloadType = "DEARS_ASSET_MESH";
constexpr const char* TextureAssetDragPayloadType = "DEARS_ASSET_TEXTURE";

// 불러온 메시와 텍스처를 타일로 보여 주고, 월드나 Inspector로 끌어다 놓는 출발점이 되는 패널이다.
//
// - 메시를 Viewport에 놓으면 그 자리에 오브젝트가 생긴다.
// - 텍스처를 Viewport의 오브젝트, Scene Hierarchy의 오브젝트, Inspector의 Diffuse 칸에 놓으면 그 텍스처가 입혀진다.
// - 렌더 프리셋은 옆 탭의 Render Presets 패널에서 같은 방식으로 끌어다 쓴다.
//
// 목록은 아직 EditorResourceCatalog의 고정 목록을 읽는다. 리소스 매니저가 "불러온 리소스 이름 나열" API를
// 갖게 되면 이 패널은 그 API를 읽도록 바꾸면 되고, 드래그 앤 드롭 쪽은 그대로 쓸 수 있다.
class ContentBrowserPanel : public IEditorPanel
{
public:
	explicit ContentBrowserPanel(DearsGraphicsEngine* graphicsEngine)
		: m_pGraphicsEngine(graphicsEngine)
	{
	}

	const char* GetName() const override { return "Content Browser"; }

	void Draw() override
	{
		const ImVec2 screenSize = ImGui::GetIO().DisplaySize;
		ImGui::SetNextWindowPos(ImVec2(0.0f, screenSize.y - 260.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(screenSize.x * 0.6f, 260.0f), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Content Browser"))
		{
			ImGui::End();
			return;
		}

		if (ImGui::BeginTabBar("ContentBrowserTabs"))
		{
			if (ImGui::BeginTabItem("Meshes"))
			{
				ImGui::TextDisabled("Drag a mesh into the Viewport to place it.");
				DrawMeshTiles();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Textures"))
			{
				ImGui::TextDisabled("Drag a texture onto an object (Viewport / Hierarchy) or the Inspector's Diffuse slot.");
				DrawTextureTiles();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

		ImGui::End();
	}

private:
	// 창 폭에 맞춰 타일을 줄바꿈한다. 방금 그린 타일 다음에 하나 더 들어갈 자리가 있으면 같은 줄에 둔다.
	static void WrapTiles(float tileWidth, bool hasNext)
	{
		const float rightEdge = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
		const float nextRight = ImGui::GetItemRectMax().x + ImGui::GetStyle().ItemSpacing.x + tileWidth;
		if (hasNext && nextRight < rightEdge)
		{
			ImGui::SameLine();
		}
	}

	void DrawMeshTiles()
	{
		const ImVec2 tileSize(120.0f, 56.0f);
		const auto& meshes = EditorResourceCatalog::VertexBuffers;
		for (int index = 0; index < static_cast<int>(meshes.size()); ++index)
		{
			ImGui::PushID(index);
			ImGui::Button(meshes[index].label, tileSize);
			if (ImGui::BeginDragDropSource())
			{
				const std::string key = meshes[index].key;
				ImGui::SetDragDropPayload(MeshAssetDragPayloadType, key.c_str(), key.size() + 1);
				ImGui::Text("Mesh: %s", meshes[index].label);
				ImGui::EndDragDropSource();
			}
			WrapTiles(tileSize.x, index + 1 < static_cast<int>(meshes.size()));
			ImGui::PopID();
		}
	}

	void DrawTextureTiles()
	{
		const float thumbnail = 64.0f;
		const auto& textures = EditorResourceCatalog::Textures;
		// 0번은 "None"이라 타일로 보여 주지 않는다.
		for (int index = 1; index < static_cast<int>(textures.size()); ++index)
		{
			ImGui::PushID(index);
			ImGui::BeginGroup();

			const std::string key = textures[index].key;
			// ImTextureID 변환은 DX11 SRV를 아는 쪽에서만 할 수 있어서 기존 Inspector와 같은 방식으로 얻는다.
			auto srv = m_pGraphicsEngine ? m_pGraphicsEngine->Get_Textures(key) : nullptr;
			if (srv)
			{
				ImGui::ImageButton("##thumbnail", static_cast<ImTextureID>(srv.Get()), ImVec2(thumbnail, thumbnail));
			}
			else
			{
				ImGui::Button("missing", ImVec2(thumbnail + 8.0f, thumbnail + 8.0f));
			}

			if (ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload(TextureAssetDragPayloadType, key.c_str(), key.size() + 1);
				ImGui::Text("Texture: %s", textures[index].label);
				ImGui::EndDragDropSource();
			}

			ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + thumbnail + 8.0f);
			ImGui::TextUnformatted(textures[index].label);
			ImGui::PopTextWrapPos();

			ImGui::EndGroup();
			WrapTiles(thumbnail + 8.0f, index + 1 < static_cast<int>(textures.size()));
			ImGui::PopID();
		}
	}

	DearsGraphicsEngine* m_pGraphicsEngine = nullptr;
};
