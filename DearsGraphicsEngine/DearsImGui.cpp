#pragma once
#include "DearsImGui.h"
#include <windows.h>
#include <iostream>
#include <wrl.h>
#include <sstream>
using Microsoft::WRL::ComPtr;

DearsImGui::DearsImGui(HWND _hWnd, ComPtr<ID3D11Device>& _pDevice, ComPtr<ID3D11DeviceContext>& m_pDeviceContext, int _endScreenWidth, int _endScreenHeight, GraphicsResourceManager* _pResourceManager)
{
	m_pInputManager = nullptr;

	m_pDevice = _pDevice;
	m_endScreenWidth = _endScreenWidth;
	m_endScreenHeight = _endScreenHeight;
	window = nullptr;
	m_pResourceManager = _pResourceManager;

	ImGui::CreateContext();
	io = &ImGui::GetIO(); // 창占쏙옙 占십깍옙화占싹거놂옙 占쏙옙占쏙옙占싹는듸옙 占쏙옙占?
	// Keep a small default editor font for panels. Large localized fonts are
	// loaded separately and used only where explicit UI text needs them.
	io->Fonts->AddFontDefault();
/*	io->FontGlobalScale = 0.5f;*/
	//io->Fonts->TexDesiredWidth = 2048; // 占쏙옙트 占쏙옙틀占쏙옙占쏙옙 占쌔쏙옙처 占십븝옙 占쏙옙占쏙옙, 占쏙옙占쏙옙 크占썩가 占쏙옙 占싱삼옙占쏙옙占쏙옙 占싼어가占쏙옙 占쏙옙占싸뤄옙 占쏙옙載? 占십뱄옙 크占쏙옙 GPU占쏙옙占쏙옙 占쏙옙 처占쏙옙占쌔쇽옙 占싹댐옙 占썅동.

	ImGui_ImplWin32_Init(_hWnd);
	ImGui_ImplDX11_Init(m_pDevice.Get(), m_pDeviceContext.Get());

	//占썩본 占쏙옙타占쏙옙 占쏙옙占쏙옙
	ImGui::StyleColorsDark();
}

DearsImGui::~DearsImGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void DearsImGui::UILoadFonts(std::string _basePath, std::string _fileName, float _size, bool _isKorean)
{
	const std::string fontPath = _basePath + _fileName;
	ImFont* tempFont;
	// 占썩본 占쏙옙트 크占쏙옙 占쏙옙占쏙옙
	if (_isKorean)
	{
		tempFont = io->Fonts->AddFontFromFileTTF(fontPath.c_str(), _size, nullptr, io->Fonts->GetGlyphRangesKorean());
	}
	else
	{
		tempFont = io->Fonts->AddFontFromFileTTF(fontPath.c_str(), _size);
	}
	m_pResourceManager->Add_Font(_fileName, tempFont); // 占쏙옙占쌀쏙옙 占쏙옙占쏙옙占싱너울옙 占쏙옙트 占쌩곤옙
}

void DearsImGui::UIBuildFonts()
{
	ImGui_ImplDX11_InvalidateDeviceObjects();
	ImGui_ImplDX11_CreateDeviceObjects();
}

void DearsImGui::UIBeginRender()
{
	// ImGui 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

}

///UI 占쏙옙占쏙옙 占쌜억옙
void DearsImGui::UISetting()
{
	// ?깅줉???먮뵒???⑤꼸 紐⑤몢 洹몃━湲?(?먯쑀 ?뚮줈??李?
	for (IEditorPanel* panel : m_panels)
		panel->Draw();
}

void DearsImGui::AddPanel(IEditorPanel* panel)
{
	m_panels.push_back(panel);
}

void DearsImGui::UIRender()
{
	ImGui::Render();
}

void DearsImGui::UIEndRender()
{
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void DearsImGui::UICanvasSet(Vector2 _posXY, Vector2 _sizeWH)
{
	// 창占쏙옙 占쏙옙치 占쏙옙占쏙옙
	ImGui::SetNextWindowPos(ImVec2(_posXY.x, _posXY.y)); // 창占쏙옙 占쏙옙치 占쏙옙占쏙옙
	ImGui::SetNextWindowSize(ImVec2(_sizeWH.x, _sizeWH.y)); // 창占쏙옙 크占쏙옙 占쏙옙占쏙옙
	// ImGui 占쏙옙타占싹울옙占쏙옙 占싻듸옙占쏙옙 占쏙옙占쏙옙
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

	// The canvas is only a drawing surface for overlay text and images.
	// It should never block mouse interaction with editor panels behind it.
	ImGui::Begin(
		"UI Canvas",
		NULL,
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoInputs
	);
}

void DearsImGui::UICanvasSetFin()
{
	ImGui::End();
	// 占쏙옙타占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙
	ImGui::PopStyleVar(2);
}

void DearsImGui::NewUISetWindow(Vector2 _posXY, Vector2 _sizeWH, const std::string _windowName)
{
	ImGui::SetNextWindowPos(ImVec2(_posXY.x, _posXY.y)); // 창占쏙옙 占쏙옙치 占쏙옙占쏙옙
	ImGui::SetNextWindowSize(ImVec2(_sizeWH.x, _sizeWH.y)); // 창占쏙옙 크占쏙옙 占쏙옙占쏙옙
	ImGui::Begin("Settings1");
}

void DearsImGui::SetUICurrentWindow()
{
	window = ImGui::GetCurrentWindow();
}

void DearsImGui::UIDrawText(Vector2 _posXY, const std::string _text, Vector4 _rgba)
{
	ImGui::SetCursorPos(ImVec2(_posXY.x, _posXY.y)); // 占싱뱄옙占쏙옙占쏙옙 표占쏙옙占쏙옙 占쏙옙치 (x, y)

		// _text占쏙옙 '\n'占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙 占쏙옙占쏙옙 처占쏙옙
	std::stringstream ss(_text);
	std::string line;

	while (std::getline(ss, line, '\n'))
	{
		ImGui::TextColored(ImVec4(_rgba.x, _rgba.y, _rgba.z, _rgba.w), line.c_str());

		// 占쏙옙占쏙옙 占쌕뤄옙 占싼어가占쏙옙 占쏙옙占쏙옙 Y 占쏙옙표占쏙옙 占쏙옙占쏙옙 占쏙옙 占쏙옙占싱몌옙큼 占쏙옙占쏙옙占싹울옙 占쏙옙占쏙옙
		_posXY.y += ImGui::GetTextLineHeightWithSpacing();
		ImGui::SetCursorPos(ImVec2(_posXY.x, _posXY.y));
	}
}

void DearsImGui::UIDrawRect(Vector2 _posXY, Vector2 _sizeWH, Vector4 _rgba, float _rounding, float _thickness)
{
	window->DrawList->AddRect(ImVec2(_posXY.x, _posXY.y), ImVec2(_posXY.x + _sizeWH.x, _posXY.y + _sizeWH.y), ImGui::ColorConvertFloat4ToU32(ImVec4(_rgba.x, _rgba.y, _rgba.z, _rgba.w)), _rounding, 0, _thickness);
}

void DearsImGui::UIDrawRectFilled(Vector2 _posXY, Vector2 _sizeWH, Vector4 _rgba, float _rounding/* = 0.0f*/)
{
	window->DrawList->AddRectFilled(ImVec2(_posXY.x, _posXY.y), ImVec2(_posXY.x + _sizeWH.x, _posXY.y + _sizeWH.y), ImGui::ColorConvertFloat4ToU32(ImVec4(_rgba.x, _rgba.y, _rgba.z, _rgba.w)), _rounding);
}


void DearsImGui::UIDrawRectwithBorder(Vector2 _posXY, Vector2 _sizeWH, Vector4 _rgba, float _rounding, float _thickness)
{
	window->DrawList->AddRectFilled(ImVec2(_posXY.x, _posXY.y), ImVec2(_posXY.x + _sizeWH.x, _posXY.y + _sizeWH.y), ImGui::ColorConvertFloat4ToU32(ImVec4(_rgba.x, _rgba.y, _rgba.z, _rgba.w)), _rounding);
	window->DrawList->AddRect(ImVec2(_posXY.x, _posXY.y), ImVec2(_posXY.x + _sizeWH.x, _posXY.y + _sizeWH.y), ImGui::ColorConvertFloat4ToU32(ImVec4(_rgba.x, _rgba.y, _rgba.z, _rgba.w)), _rounding, 0, _thickness);
}

void DearsImGui::UIFreeRect(Vector2 _posXY1, Vector2 _posXY2, Vector2 _posXY3, Vector2 _posXY4, Vector4 _rgba, float _thickness)
{
	ImVec2 p1(_posXY1.x, _posXY1.y);
	ImVec2 p2(_posXY2.x, _posXY2.y);
	ImVec2 p3(_posXY3.x, _posXY3.y);
	ImVec2 p4(_posXY4.x, _posXY4.y);

	ImVec2 points[6] = { p1, p2, p3, p4, p1, p2 };

	window->DrawList->AddPolyline(points, 6, ImGui::ColorConvertFloat4ToU32(ImVec4(_rgba.x, _rgba.y, _rgba.z, _rgba.w)), ImDrawFlags_RoundCornersAll, _thickness);
}

void DearsImGui::UIFreeRectFilled(Vector2 _posXY1, Vector2 _posXY2, Vector2 _posXY3, Vector2 _posXY4, Vector4 _rgba)
{
	ImVec2 p1(_posXY1.x, _posXY1.y);
	ImVec2 p2(_posXY2.x, _posXY2.y);
	ImVec2 p3(_posXY3.x, _posXY3.y);
	ImVec2 p4(_posXY4.x, _posXY4.y);

	ImVec2 points[4] = { p1, p2, p3, p4 };

	window->DrawList->AddConvexPolyFilled(points, 4, ImGui::ColorConvertFloat4ToU32(ImVec4(_rgba.x, _rgba.y, _rgba.z, _rgba.w)));
}

void DearsImGui::UIFreeRectwithBorder(Vector2 _posXY1, Vector2 _posXY2, Vector2 _posXY3, Vector2 _posXY4, Vector4 _rgba, float _thickness, Vector4 _borderRgba)
{
	ImVec2 p1(_posXY1.x, _posXY1.y);
	ImVec2 p2(_posXY2.x, _posXY2.y);
	ImVec2 p3(_posXY3.x, _posXY3.y);
	ImVec2 p4(_posXY4.x, _posXY4.y);

	ImVec2 points[4] = { p1, p2, p3, p4 };
	ImVec2 borderPoints[6] = { p1, p2, p3, p4, p1, p2 };

	window->DrawList->AddConvexPolyFilled(points, 4, ImGui::ColorConvertFloat4ToU32(ImVec4(_rgba.x, _rgba.y, _rgba.z, _rgba.w)));
	window->DrawList->AddPolyline(borderPoints, 6, ImGui::ColorConvertFloat4ToU32(ImVec4(_borderRgba.x, _borderRgba.y, _borderRgba.z, _borderRgba.w)), ImDrawFlags_RoundCornersAll, _thickness);
}

void DearsImGui::UIDrawLine(Vector2 _sPosXY, Vector2 _ePosXY, Vector4 _rgba)
{
	window->DrawList->AddLine(ImVec2(_sPosXY.x, _sPosXY.y), ImVec2(_ePosXY.x, _ePosXY.y), ImGui::ColorConvertFloat4ToU32(ImVec4(_rgba.x, _rgba.y, _rgba.z, _rgba.w)));
}

void DearsImGui::UIDrawCircle(Vector2 _posXY, float _radius, Vector4 _rgba)
{
	ImGui::GetWindowDrawList()->AddCircle(ImVec2(_posXY.x, _posXY.y), _radius, ImGui::ColorConvertFloat4ToU32(ImVec4(_rgba.x, _rgba.y, _rgba.z, _rgba.w)));
}

void DearsImGui::UIDrawImage(Vector2 _posXY, Vector2 _sizeWH, ComPtr<ID3D11ShaderResourceView> _srv, Vector4 _rgba)
{
	ImGui::SetCursorPos(ImVec2(_posXY.x, _posXY.y)); // 占싱뱄옙占쏙옙占쏙옙 표占쏙옙占쏙옙 占쏙옙치 (x, y)
	ImGui::Image((void*)_srv.Get(), ImVec2(_sizeWH.x, _sizeWH.y), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImVec4(_rgba.x, _rgba.y, _rgba.z, _rgba.w)); // 占싱뱄옙占쏙옙占쏙옙 화占썽에 표占쏙옙
}

bool DearsImGui::Hovering()
{
	return ImGui::IsItemHovered();
}

bool DearsImGui::Dragging()
{
	return ImGui::IsMouseDragging(ImGuiMouseButton_Left);
}

void DearsImGui::ResetDragDelta()
{
	ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
}

void DearsImGui::UIDrawImageStart()
{
	ImGui::PushID(0);
}

void DearsImGui::UIDrawImageFin()
{
	ImGui::PopID();
}

void DearsImGui::UIStartFontID(ImFont* _imFont)
{
	ImGui::PushFont(_imFont);
}

void DearsImGui::UIEndFontID()
{
	ImGui::PopFont();
}

void DearsImGui::Drag(Vector2& _posXY)
{
	// 占썲래占쏙옙 처占쏙옙
	if (ImGui::IsItemHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
	{
		ImVec2 dragDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
		_posXY.x += dragDelta.x;
		_posXY.y += dragDelta.y;
		std::cout << "ImGui Drag Position(x, y)" << dragDelta.x << ", " << dragDelta.y << ")" << std::endl;
		ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
	}
}

std::string DearsImGui::ConvertUTF8String(const std::u8string& _text)
{
	// char8_t占쏙옙 C++20占쏙옙占쏙옙 占쏙옙占쌉듸옙 占쌘뤄옙占쏙옙占싱댐옙. 占쏙옙 占싱삼옙 占쏙옙占쌘울옙占쏙옙 占쏙옙占?占쏙옙占쏙옙 占쏙옙占쏙옙占실뤄옙 占쏙옙환占쏙옙 占쏙옙占쏙옙蔘占?占싼댐옙.
	return std::string(_text.begin(), _text.end());
}
