#include "UiRenderer.h"

// DockBuilder(기본 배치를 코드로 만드는 API)는 ImGui의 internal 헤더에 있다.
#include <imgui_internal.h>

UiRenderer::UiRenderer(
	HWND hWnd,
	ComPtr<ID3D11Device>& device,
	ComPtr<ID3D11DeviceContext>& deviceContext,
	int screenWidth,
	int screenHeight,
	GraphicsResourceManager* resourceManager)
	: m_pResourceManager(resourceManager)
{
	m_pDearsImGui = std::make_unique<DearsImGui>(
		hWnd,
		device,
		deviceContext,
		screenWidth,
		screenHeight,
		resourceManager);
}

void UiRenderer::AddEditorPanel(IEditorPanel* panel)
{
	if (!panel)
	{
		return;
	}

	m_editorPanels.push_back(panel);
	m_editorPanelVisibility.push_back(true);
}

void UiRenderer::SetDockSlot(const std::string& windowName, EditorDockSlot slot)
{
	for (std::pair<std::string, EditorDockSlot>& entry : m_dockSlots)
	{
		if (entry.first == windowName)
		{
			entry.second = slot;
			return;
		}
	}
	m_dockSlots.emplace_back(windowName, slot);
}

void UiRenderer::BeginDockSpace()
{
#ifdef IMGUI_HAS_DOCK
	// 메인 창 전체(메뉴 바 아래)를 덮는 보이지 않는 창을 하나 만들고 그 안을 도킹 영역으로 쓴다.
	// 이 창 자체는 움직이거나 다른 곳에 붙을 수 없고, 다른 에디터 창들이 이 영역에 붙는다.
	const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(mainViewport->WorkPos);
	ImGui::SetNextWindowSize(mainViewport->WorkSize);
	ImGui::SetNextWindowViewport(mainViewport->ID);

	const ImGuiWindowFlags hostFlags =
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("EditorDockSpaceHost", nullptr, hostFlags);
	ImGui::PopStyleVar(3);

	const ImGuiID dockSpaceId = ImGui::GetID("EditorDockSpace");

	// imgui.ini에 저장된 배치가 있으면 ImGui가 그대로 복원하므로 건드리지 않는다.
	// 저장된 배치가 없는 첫 실행과, 사용자가 Reset Layout을 고른 경우에만 기본 배치를 만든다.
	if (m_defaultLayoutRequested || ImGui::DockBuilderGetNode(dockSpaceId) == nullptr)
	{
		BuildDefaultDockLayout(dockSpaceId, mainViewport->WorkSize.x, mainViewport->WorkSize.y);
		m_defaultLayoutRequested = false;
	}

	ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
	ImGui::End();
#endif
}

void UiRenderer::BuildDefaultDockLayout(unsigned int dockSpaceId, float width, float height)
{
#ifdef IMGUI_HAS_DOCK
	ImGui::DockBuilderRemoveNode(dockSpaceId);
	ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(dockSpaceId, ImVec2(width, height));

	// 전체를 먼저 좌/우로 나누고, 왼쪽을 위(월드)/아래(에셋)로, 오른쪽을 위(목록)/아래(속성)로 나눈다.
	// SplitNode는 잘라 낸 쪽의 ID를 돌려주고 남은 쪽의 ID를 마지막 인자로 채워 준다.
	ImGuiID center = dockSpaceId;
	ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.22f, nullptr, &center);
	ImGuiID bottom = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.28f, nullptr, &center);
	ImGuiID rightBottom = ImGui::DockBuilderSplitNode(right, ImGuiDir_Down, 0.58f, nullptr, &right);

	ImGui::DockBuilderDockWindow("Viewport", center);
	for (const std::pair<std::string, EditorDockSlot>& entry : m_dockSlots)
	{
		ImGuiID target = 0;
		switch (entry.second)
		{
		case EditorDockSlot::Center:
			target = center;
			break;
		case EditorDockSlot::Right:
			target = right;
			break;
		case EditorDockSlot::RightBottom:
			target = rightBottom;
			break;
		case EditorDockSlot::Bottom:
			target = bottom;
			break;
		case EditorDockSlot::Floating:
		default:
			break;
		}

		if (target != 0)
		{
			ImGui::DockBuilderDockWindow(entry.first.c_str(), target);
		}
	}

	ImGui::DockBuilderFinish(dockSpaceId);
#else
	(void)dockSpaceId;
	(void)width;
	(void)height;
#endif
}

void UiRenderer::BeginFrame()
{
	m_pDearsImGui->UIBeginRender();
}

void UiRenderer::BeginViewport(const std::function<void*(int, int)>& ensureSurface)
{
	m_viewport.Begin(ensureSurface);
	m_viewportOpen = true;
	m_pDearsImGui->SetCanvasOrigin(m_viewport.canvasOrigin);
}

void UiRenderer::DrawRegisteredPanels()
{
	// 상단 Window 메뉴는 등록된 에디터 패널 목록으로 자동 생성한다.
	// 메뉴를 통해 숨긴 창도 다시 켤 수 있으므로 각 창 내부에 별도의 복구 버튼이 필요 없다.
	if (ImGui::BeginMainMenuBar())
	{
		// File/Edit 메뉴와 Play 버튼처럼 씬을 아는 쪽이 그려야 하는 항목이 먼저 온다.
		if (m_menuBarCallback)
		{
			m_menuBarCallback();
		}

		if (ImGui::BeginMenu("Window"))
		{
			for (size_t index = 0; index < m_editorPanels.size(); ++index)
			{
				bool visible = m_editorPanelVisibility[index];
				if (ImGui::MenuItem(m_editorPanels[index]->GetName(), nullptr, visible))
				{
					m_editorPanelVisibility[index] = !visible;
				}
			}
			ImGui::Separator();
			// 창을 옮기다 배치가 엉켰을 때 처음 상태(가운데 월드, 오른쪽 목록/속성, 아래 에셋)로 되돌린다.
			if (ImGui::MenuItem("Reset Layout"))
			{
				RequestDefaultLayout();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	for (size_t index = 0; index < m_editorPanels.size(); ++index)
	{
		if (m_editorPanelVisibility[index])
		{
			m_editorPanels[index]->Draw();
		}
	}
}

void UiRenderer::BeginCanvas(Vector2 position, Vector2 size)
{
	m_pDearsImGui->UICanvasSet(position, size);
	m_canvasOpen = true;
}

void UiRenderer::CacheCurrentWindow()
{
	m_pDearsImGui->SetUICurrentWindow();
}

void UiRenderer::Render()
{
	if (m_canvasOpen) { m_pDearsImGui->UICanvasSetFin(); m_canvasOpen = false; }
	if (m_viewportOpen) { m_viewport.End(); m_viewportOpen = false; }
	m_pDearsImGui->UIRender();
}

void UiRenderer::EndFrame()
{
	m_pDearsImGui->UIEndRender();
}

void UiRenderer::AddFont(std::string basePath, std::string fileName, float size, bool isKorean)
{
	m_pDearsImGui->UILoadFonts(basePath, fileName, size, isKorean);
}

void UiRenderer::BuildFonts()
{
	m_pDearsImGui->UIBuildFonts();
}

void UiRenderer::DrawImageStart()
{
	m_pDearsImGui->UIDrawImageStart();
}

void UiRenderer::DrawImage(Vector2 position, Vector2 size, std::string textureName, Vector4 rgba)
{
	ComPtr<ID3D11ShaderResourceView> srv = m_pResourceManager->Get_Textures(textureName);
	m_pDearsImGui->UIDrawImage(position, size, srv, rgba);
}

void UiRenderer::DrawImageEnd()
{
	m_pDearsImGui->UIDrawImageFin();
}

void UiRenderer::StartFont(std::string fontName)
{
	m_pDearsImGui->UIStartFontID(m_pResourceManager->Get_Font(fontName));
}

void UiRenderer::DrawText(Vector2 position, std::u8string text, Vector4 rgba)
{
	m_pDearsImGui->UIDrawText(position, m_pDearsImGui->ConvertUTF8String(text), rgba);
}

void UiRenderer::EndFont()
{
	m_pDearsImGui->UIEndFontID();
}

void UiRenderer::DrawRect(Vector2 position, Vector2 size, Vector4 rgba, float rounding, float thickness)
{
	m_pDearsImGui->UIDrawRect(position, size, rgba, rounding, thickness);
}

void UiRenderer::DrawRectFilled(Vector2 position, Vector2 size, Vector4 rgba, float rounding)
{
	m_pDearsImGui->UIDrawRectFilled(position, size, rgba, rounding);
}

void UiRenderer::DrawRectWithBorder(Vector2 position, Vector2 size, Vector4 rgba, float rounding, float thickness)
{
	m_pDearsImGui->UIDrawRectwithBorder(position, size, rgba, rounding, thickness);
}

void UiRenderer::DrawFreeRect(Vector2 position1, Vector2 position2, Vector2 position3, Vector2 position4, Vector4 rgba, float thickness)
{
	m_pDearsImGui->UIFreeRect(position1, position2, position3, position4, rgba, thickness);
}

void UiRenderer::DrawFreeRectFilled(Vector2 position1, Vector2 position2, Vector2 position3, Vector2 position4, Vector4 rgba)
{
	m_pDearsImGui->UIFreeRectFilled(position1, position2, position3, position4, rgba);
}

void UiRenderer::DrawFreeRectWithBorder(Vector2 position1, Vector2 position2, Vector2 position3, Vector2 position4, Vector4 rgba, float thickness, Vector4 borderRgba)
{
	m_pDearsImGui->UIFreeRectwithBorder(position1, position2, position3, position4, rgba, thickness, borderRgba);
}

void UiRenderer::DrawLine(Vector2 startPosition, Vector2 endPosition, Vector4 rgba)
{
	m_pDearsImGui->UIDrawLine(startPosition, endPosition, rgba);
}

void UiRenderer::DrawCircle(Vector2 position, float radius, Vector4 rgba)
{
	m_pDearsImGui->UIDrawCircle(position, radius, rgba);
}
