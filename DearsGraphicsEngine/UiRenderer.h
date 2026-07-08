#pragma once

#include <d3d11.h>
#include <directxtk/SimpleMath.h>
#include <memory>
#include <string>
#include <wrl.h>

#include "DearsImGui.h"
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

	void BeginFrame();
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
