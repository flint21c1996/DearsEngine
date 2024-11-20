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
	io = &ImGui::GetIO(); // 창을 초기화하거나 설정하는데 사용
/*	io->FontGlobalScale = 0.5f;*/
	//io->Fonts->TexDesiredWidth = 2048; // 폰트 아틀라스의 텍스처 너비를 제한, 가로 크기가 이 이상으로 넘어가면 세로로 들어감. 너무 크면 GPU에서 못 처리해서 하는 행동.

	ImGui_ImplWin32_Init(_hWnd);
	ImGui_ImplDX11_Init(m_pDevice.Get(), m_pDeviceContext.Get());

	//기본 스타일 설정
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
	// 기본 폰트 크기 변경
	if (_isKorean)
	{
		tempFont = io->Fonts->AddFontFromFileTTF(fontPath.c_str(), _size, nullptr, io->Fonts->GetGlyphRangesKorean());
	}
	else
	{
		tempFont = io->Fonts->AddFontFromFileTTF(fontPath.c_str(), _size);
	}
	m_pResourceManager->Add_Font(_fileName, tempFont); // 리소스 컨테이너에 폰트 추가
}

void DearsImGui::UIBuildFonts()
{
	ImGui_ImplDX11_InvalidateDeviceObjects();
	ImGui_ImplDX11_CreateDeviceObjects();
}

void DearsImGui::UIBeginRender()
{
	// ImGui 프레임 시작
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

}

///UI 관련 작업 -> 게임 엔진의 컨텐츠에서 할 부분임
void DearsImGui::UISetting()
{
	///// 0. 캔버스 설정
	//DrawStart(Vector2(0, 0), Vector2(static_cast<float>(m_endScreenWidth), static_cast<float>(m_endScreenHeight)));

	///// 1. 아무런 이벤트가 없는 이미지
	//DrawImageStart();
	////StartUI_TYPE(UI_TYPE::IMAGE);
	//DrawImage(Vector2(1720, 20), Vector2(200, 200), "coco.jpg");
	//DrawImage(Vector2(845, 800), Vector2(230, 75), "startButton.png");
	//if (Hovering() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	//{
	//	DrawText(Vector2(960, 750), "click!");
	//}
	//DrawImage(t_imagePos, Vector2(200, 200), "bird.png");
	//Drag(t_imagePos);
	//DrawImageEnd();

	///// 4. 기본 도형 그리기 전 윈도우 가져오기
	//GetCurrentWindow();

	///// 5. 도형 그리기 -> 혹시 아래에 그려야 하면 함수 분리를 해야 함. 필요에 따라 추후 제작(현재 예정 없음)
	//DrawRect(Vector2(100, 100), Vector2(200, 50), Vector4(1.0f, 1.0f, 0.0f, 1.0f));
	//DrawLine(Vector2(110, 110), Vector2(160, 160), Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	//DrawCircle(Vector2(150, 150), 20.0f, Vector4(0.0f, 0.0f, 1.0f, 1.0f));

	///// 7. 폰트를 출력하기 (함수화 해야 함)

	///// 8. 캔버스 설정 완료
	//DrawEnd();

	/// 새 창 만들기 예시
	// newSettingWindow(Vector2(100,100), Vector2(500,500), "Setting1");
	// ...(기능 추가)
	// DrawEnd();
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
	// 창의 위치 설정
	ImGui::SetNextWindowPos(ImVec2(_posXY.x, _posXY.y)); // 창의 위치 설정
	ImGui::SetNextWindowSize(ImVec2(_sizeWH.x, _sizeWH.y)); // 창의 크기 설정
	// ImGui 스타일에서 패딩을 제거
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

	ImGui::Begin("UI Canvas", NULL, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav); //여기서 Begin과 End는 설정창을 그리게 한다.
}

void DearsImGui::UICanvasSetFin()
{
	ImGui::End();
	// 스타일 설정 복원
	ImGui::PopStyleVar(2);
}

void DearsImGui::NewUISetWindow(Vector2 _posXY, Vector2 _sizeWH, const std::string _windowName)
{
	ImGui::SetNextWindowPos(ImVec2(_posXY.x, _posXY.y)); // 창의 위치 설정
	ImGui::SetNextWindowSize(ImVec2(_sizeWH.x, _sizeWH.y)); // 창의 크기 설정
	ImGui::Begin("Settings1");
}

void DearsImGui::SetUICurrentWindow()
{
	window = ImGui::GetCurrentWindow();
}

void DearsImGui::UIDrawText(Vector2 _posXY, const std::string _text, Vector4 _rgba)
{
	ImGui::SetCursorPos(ImVec2(_posXY.x, _posXY.y)); // 이미지를 표시할 위치 (x, y)

		// _text를 '\n'으로 나누어 각 줄을 처리
	std::stringstream ss(_text);
	std::string line;

	while (std::getline(ss, line, '\n'))
	{
		ImGui::TextColored(ImVec4(_rgba.x, _rgba.y, _rgba.z, _rgba.w), line.c_str());

		// 다음 줄로 넘어가기 위해 Y 좌표를 현재 줄 높이만큼 누적하여 조정
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
	ImGui::SetCursorPos(ImVec2(_posXY.x, _posXY.y)); // 이미지를 표시할 위치 (x, y)
	ImGui::Image((void*)_srv.Get(), ImVec2(_sizeWH.x, _sizeWH.y), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImVec4(_rgba.x, _rgba.y, _rgba.z, _rgba.w)); // 이미지를 화면에 표시
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
	// 드래그 처리
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
	// char8_t는 C++20부터 도입된 자료형이다. 더 이상 문자열로 취급 받지 않으므로 변환을 해줘야만 한다.
	return std::string(_text.begin(), _text.end());
}