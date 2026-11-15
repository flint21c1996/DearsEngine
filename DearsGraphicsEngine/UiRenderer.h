#pragma once

#include <d3d11.h>
#include <directxtk/SimpleMath.h>
#include <functional>
#include <memory>
#include <string>
#include <wrl.h>
#include <utility>
#include <vector>

#include "DearsImGui.h"
#include "EditorDockLayout.h"
#include "EditorViewportWindow.h"
#include "GraphicsResourceManager.h"

class IEditorPanel;

using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector4;
using Microsoft::WRL::ComPtr;

// ImGui 기반 UI 렌더링을 담당하는 클래스이다.
//
// DearsGraphicsEngine은 렌더 루프의 큰 흐름만 관리하고,
// UiRenderer는 폰트 로딩, 에디터 패널 등록, UI 캔버스, 텍스트/이미지 그리기 같은
// ImGui 세부 호출을 한곳에 모은다.
//
// 지금은 DX11 ImGui 구현체를 그대로 사용하지만, 게임 엔진 코드가 DearsImGui를 직접 알지 않게
// 한 번 감싸두면 나중에 에디터 UI와 런타임 UI를 나누기도 쉬워진다.
class UiRenderer
{
public:
	UiRenderer(
		HWND hWnd,
		ComPtr<ID3D11Device>& device,
		ComPtr<ID3D11DeviceContext>& deviceContext,
		int screenWidth,
		int screenHeight,
		GraphicsResourceManager* resourceManager);

	void AddEditorPanel(IEditorPanel* panel);

	// 창 이름(ImGui::Begin에 넘기는 이름)별 기본 도킹 자리를 등록한다.
	// 같은 자리에 여러 창을 등록하면 등록한 순서대로 탭으로 묶인다.
	void SetDockSlot(const std::string& windowName, EditorDockSlot slot);
	// 화면 전체를 덮는 도킹 영역을 만든다. 모든 에디터 창보다 먼저, 매 프레임 호출한다.
	// 저장된 배치가 없으면(첫 실행) 등록된 기본 자리대로 배치한다.
	void BeginDockSpace();
	// 다음 프레임에 기본 배치로 되돌린다.
	void RequestDefaultLayout() { m_defaultLayoutRequested = true; }

	void BeginFrame();
	void BeginViewport(const std::function<void*(int, int)>& ensureSurface);
	const EditorViewportState& GetViewportState() const { return m_viewport.state; }
	bool ShowViewportOverlay() const { return m_viewport.showOverlay && m_viewport.state.Valid(); }

	// 월드 이미지 위에 끌어다 놓을 수 있는 드래그 페이로드 종류를 등록한다.
	void SetViewportDropTypes(std::vector<std::string> types) { m_viewport.dropTypes = std::move(types); }
	// 이번 프레임에 월드 이미지에 놓인 것이 있으면 꺼내 주고 true를 돌려준다. 한 번 꺼내면 비워진다.
	bool ConsumeViewportDrop(std::string& type, std::string& data, float& x, float& y)
	{
		if (!m_viewport.drop.pending)
		{
			return false;
		}
		type = m_viewport.drop.type;
		data = m_viewport.drop.data;
		x = m_viewport.drop.x;
		y = m_viewport.drop.y;
		m_viewport.drop = {};
		return true;
	}

	// 메인 메뉴 바에서 Window 메뉴 앞에 그릴 내용(File, Edit, Play 버튼 등)을 게임 계층이 등록한다.
	void SetMenuBarCallback(std::function<void()> callback) { m_menuBarCallback = std::move(callback); }
	void DrawRegisteredPanels();
	void BeginCanvas(Vector2 position, Vector2 size = Vector2());
	void CacheCurrentWindow();
	void Render();
	void EndFrame();

	void AddFont(std::string basePath, std::string fileName, float size, bool isKorean);
	void BuildFonts();

	void DrawImageStart();
	void DrawImage(Vector2 position, Vector2 size, std::string textureName, Vector4 rgba = Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	void DrawImageEnd();

	void StartFont(std::string fontName);
	void DrawText(Vector2 position, std::u8string text, Vector4 rgba = Vector4(0.0f, 0.0f, 0.0f, 1.0f));

	template<typename ...Args>
	void DrawTextWithNum(Vector2 position, const std::u8string formatText, Vector4 rgba, Args&& ...args);

	void EndFont();

	void DrawRect(Vector2 position, Vector2 size, Vector4 rgba, float rounding = 0.0f, float thickness = 1.0f);
	void DrawRectFilled(Vector2 position, Vector2 size, Vector4 rgba, float rounding = 0.0f);
	void DrawRectWithBorder(Vector2 position, Vector2 size, Vector4 rgba, float rounding = 0.0f, float thickness = 1.0f);
	void DrawFreeRect(Vector2 position1, Vector2 position2, Vector2 position3, Vector2 position4, Vector4 rgba, float thickness);
	void DrawFreeRectFilled(Vector2 position1, Vector2 position2, Vector2 position3, Vector2 position4, Vector4 rgba);
	void DrawFreeRectWithBorder(Vector2 position1, Vector2 position2, Vector2 position3, Vector2 position4, Vector4 rgba, float thickness, Vector4 borderRgba);
	void DrawLine(Vector2 startPosition, Vector2 endPosition, Vector4 rgba);
	void DrawCircle(Vector2 position, float radius, Vector4 rgba);

private:
	std::unique_ptr<DearsImGui> m_pDearsImGui;
	GraphicsResourceManager* m_pResourceManager = nullptr;
	EditorViewportWindow m_viewport;
	bool m_viewportOpen = false;
	bool m_canvasOpen = false;

	void BuildDefaultDockLayout(unsigned int dockSpaceId, float width, float height);
	std::vector<std::pair<std::string, EditorDockSlot>> m_dockSlots;
	bool m_defaultLayoutRequested = false;
	std::function<void()> m_menuBarCallback;

	// UiRenderer가 등록된 모든 에디터 창과 표시 여부를 함께 관리한다.
	// 패널 클래스마다 별도의 단축키나 토글 코드를 만들지 않아도 Window 메뉴에서 제어할 수 있다.
	std::vector<IEditorPanel*> m_editorPanels;
	std::vector<bool> m_editorPanelVisibility;
};

template<typename ...Args>
void UiRenderer::DrawTextWithNum(Vector2 position, const std::u8string formatText, Vector4 rgba, Args&& ...args)
{
	m_pDearsImGui->UIDrawTextWithNum(
		position,
		m_pDearsImGui->ConvertUTF8String(formatText),
		rgba,
		std::forward<Args>(args)...);
}
