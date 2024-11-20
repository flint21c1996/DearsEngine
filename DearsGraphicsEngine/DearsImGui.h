#pragma once
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h> //사용자 지정 위젯을 만들거나 ImGui의 동작을 수정하는 경우에 사용
#include "UIEnum.h"
#include "GraphicsResourceManager.h"

/// <summary>
/// 추가해야 할 것
/// 최적화
/// </summary>
class InputManager;

/// <summary>
/// ImGui를 사용하기 위한 여러 기능 클래스
/// </summary>
class DearsImGui
{
public:
	DearsImGui(HWND _hWnd, ComPtr<ID3D11Device>& _pDevice, ComPtr<ID3D11DeviceContext>& m_pDeviceContext, int _endScreenWidth, int _endScreenHeight, GraphicsResourceManager* _pResourceManager);
	~DearsImGui();

	InputManager* m_pInputManager;

private:
	ComPtr<ID3D11Device> m_pDevice;					   // 그래픽스 엔진에서 디바이스를 받아온다.
	GraphicsResourceManager* m_pResourceManager;	   // 리소스 매니저의 포인터를 디바이스에서 받아온다 -> 컨테이너에 접근하기 위해 호출
	int m_endScreenWidth;							   // 스크린 가로 길이
	int m_endScreenHeight;							   // 스크린 세로 길이
	ImGuiWindow* window;							   // ImGui의 화면을 가리키는 변수
	ImGuiIO* io;									   // 창을 초기화하거나 설정하는데 사용

	Vector2 t_imagePos = Vector2(150, 600);			   // 지울 예정

public:
	/// 초기화에 셋팅하는 부분 : 생성 후 세팅 (순서대로 세팅)
	void UILoadFonts(std::string _basePath, std::string _fileName, float _size, bool _isKorean);	// 폰트를 로드한다. (폰트 파일 경로, 폰트 사이즈, 한글 여부)
	void UIBuildFonts();			// 폰트를 로드한 후, 폰트 텍스처를 다시 빌드

	/// D3D 렌더 전에 넣기 (순서대로 세팅)
	void UIBeginRender(); // BeginRender 전에 호출
	void UISetting();		 // 그릴 UI를 세팅
	void UIRender();		 // ImGui 렌더링

	///  D3D EndRender 전에 넣기
	void UIEndRender();	 // EndRender 전에 호출

	/// 캔버스 설정
	void UICanvasSet(Vector2 _posXY, Vector2 _sizeWH);	// 캔버스 설정 (시작 위치xy, 가로세로)
	void UICanvasSetFin();										// 캔버스 설정이 끝났음을 선언
	void NewUISetWindow(Vector2 _posXY, Vector2 _sizeWH, const std::string _windowName); // 새로운 창을 만들 때 사용 (시작 위치xy, 가로세로, 설정창의 이름(중복x))
	void SetUICurrentWindow();							// 현재 윈도우 가져오기, 기본 그리기 함수의 바로 위에 호출

	/// 기본 그리기
	// 텍스트
	void UIDrawText(Vector2 _posXY, const std::string _text, Vector4 _rgba = Vector4(0.0f, 0.0f, 0.0f, 1.0f));				// 텍스트를 그리는 함수 (시작 위치xy, 쓰고 싶은 문구) 
	// 숫자가 있는 텍스트
	template<typename ...Args>
	void UIDrawTextWithNum(Vector2 _posXY, const std::string _formatText, Vector4 _rgba, Args&&... args);
	// 사각형
	void UIDrawRect(Vector2 _posXY, Vector2 _sizeWH, Vector4 _rgba, float _rounding = 0.0f, float _thickness = 1.0F);				// 사각형의 테두리만 그리는 함수 (시작 위치xy, 가로세로, RGBA, 모서리 둥근 정도, 두께)
	void UIDrawRectFilled(Vector2 _posXY, Vector2 _sizeWH, Vector4 _rgba, float _rounding = 0.0f);							// 채워진 사각형을 그리는 함수 (시작 위치xy, 가로세로, RGBA, 모서리 둥근 정도)
	void UIDrawRectwithBorder(Vector2 _posXY, Vector2 _sizeWH, Vector4 _rgba, float _rounding = 0.0f, float _thickness = 1.0F);	// 사각형과 테두리를 그리는 함수(시작 위치xy, 가로세로, RGBA, 모서리 둥근 정도, 두께)
	void UIFreeRect(Vector2 _posXY1, Vector2 _posXY2, Vector2 _posXY3, Vector2 _posXY4, Vector4 _rgba, float _thickness);	// 자유형 사각형의 테두리만 그리는 함수 (point 시계방향으로 4개, RGBA, 두께)
	void UIFreeRectFilled(Vector2 _posXY1, Vector2 _posXY2, Vector2 _posXY3, Vector2 _posXY4, Vector4 _rgba);				// 채워진 자유형 사각형을 그리는 함수 (point 시계방향으로 4개, RGBA)
	void UIFreeRectwithBorder(Vector2 _posXY1, Vector2 _posXY2, Vector2 _posXY3, Vector2 _posXY4, Vector4 _rgba, float _thickness, Vector4 _borderRgba); // 자유형 사각형과 테두리를 그리는 함수 (point 시계방향으로 4개, RGBA, 두께, 두께 RGBA)
	// 선
	void UIDrawLine(Vector2 _sPosXY, Vector2 _ePosXY, Vector4 _rgba);		// 선을 그리는 함수 (시작점 위치xy, 끝점 위치xy, RGBA)
	// 원
	void UIDrawCircle(Vector2 _posXY, float _radius, Vector4 _rgba);		// 원을 그리는 함수 (시작 위치xy, 반지름, RGBA)
	
	/// 이미지 그리기
	void UIDrawImage(Vector2 _posXY, Vector2 _sizeWH, ComPtr<ID3D11ShaderResourceView> _srv, Vector4 _rgba = Vector4(1.0f, 1.0f, 1.0f, 1.0f)); // 이미지를 그리는 가장 기본적인 함수 (시작 위치xy, 가로세로, 텍스처, 텍스처 추가 색상)

	/// 기능
	bool Hovering();			// 마우스가 현재 아이디 오브젝트 위에 존재하는지 확인
	bool Dragging();			// 마우스가 현재 드래그 중인지 확인
	void ResetDragDelta();		// 드래그 중인 위치값을 리셋

	/// 고유 아이디 추가와 삭제
	// ImGui에서 폰트를 사용하는 방식은 일반적인 메모리 할당과 다르다.
	// ImFont 객체는 ImGuiIO의 폰트 스택에 저장되며, 별도로 할당이나 해제할 필요가 없다.
	void UIDrawImageStart();	// 오브젝트 고유 아이디 추가
	void UIDrawImageFin();			// 오브젝트 고유 아이디 삭제
	void UIStartFontID(ImFont* _imFont);	// 폰트 아이디 추가
	void UIEndFontID();			// 폰트 아이디 삭제

	/// C++20에서 강제로 UTF-8 문자열을 char8_t 도입에 따라 자료형으로 바뀌기 때문에 이를 다시 문자열로 전환하는 함수
	std::string ConvertUTF8String(const std::u8string& _text); // u8 텍스트를 String 값으로 변환

private:
	void Drag(Vector2& _posXY); 
};

template<typename ...Args>
void DearsImGui::UIDrawTextWithNum(Vector2 _posXY, const std::string _formatText, Vector4 _rgba, Args&&... args)
{
	char buffer[256]; // 충분히 큰 버퍼를 확보 -> 추후 기획에서 부족하다 하면 512로 바꿀 것
	sprintf_s(buffer, sizeof(buffer), _formatText.c_str(), std::forward<Args>(args)...);

	UIDrawText(_posXY, std::string(buffer)/*fullText*/, _rgba);
}

