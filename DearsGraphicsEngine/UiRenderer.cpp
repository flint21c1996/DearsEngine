#include "UiRenderer.h"

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

void UiRenderer::BeginFrame()
{
	m_pDearsImGui->UIBeginRender();
}

void UiRenderer::DrawRegisteredPanels()
{
	// 상단 Window 메뉴는 등록된 에디터 패널 목록으로 자동 생성한다.
	// 메뉴를 통해 숨긴 창도 다시 켤 수 있으므로 각 창 내부에 별도의 복구 버튼이 필요 없다.
	if (ImGui::BeginMainMenuBar())
	{
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
}

void UiRenderer::CacheCurrentWindow()
{
	m_pDearsImGui->SetUICurrentWindow();
}

void UiRenderer::Render()
{
	m_pDearsImGui->UICanvasSetFin();
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
