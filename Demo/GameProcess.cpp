#include "GameProcess.h"
#include "GameEngine.h"
//#include "../GameEngine/GameEngine.h"
#include "imgui.h"

#ifdef _DEBUG
#include<iostream>
#endif

GameProcess::GameProcess()
{
	m_WindowPosX = 0;
	m_WindowPosY = 0;
	m_ScreenWidth = 1920;
	m_ScreenHeight = 1080;
	m_hWnd = {};
	m_Msg = {};

	m_pGameEngine = nullptr;
}

GameProcess::~GameProcess()
{

}

HRESULT GameProcess::Initialize(HINSTANCE hInstance)
{
	//운영체제에서 창 클래스를 정의하고 등록한다. WNDCLASSEX는 창 클래스의 속성을 정의하는 구조체이다. 
	WNDCLASSEX wc = {
		sizeof(WNDCLASSEX),												//구조체의 크기를 나타낸다. 
		CS_CLASSDC,														//창 클래스 스타일을 설정한다.
		WndProc,														//창 프로시저를 가리킨다.  창 프로시저는 창이 메시지를 어떻게 처리할지를 결정하는 함수이다. 
		0,
		0,
		GetModuleHandle(NULL),											//인스턴스 핸들을 가져온다. NULL일 시 현재 실행중인 인스턴스의 핸들을 가져온다.
		NULL,
		NULL,
		NULL,
		NULL,
		L"MangoWaffle",													// lpszClassName, L-string, 창 클래스의 이름을 설정한다. 
		NULL };

	RegisterClassEx(&wc);												//이 창을 시스템에 등록하는 함수의 호출, 이 창을 호출 함으로 우리는 이 창 클래스를 생성할수 있게 된다. 


	///Window 창을 생성하는 부분, CreateWindow함수는 주어진 매개변수에 따라 새로운 창을 생성하고, 그 창의 핸들을 반환한다.
	m_hWnd = CreateWindow(
		wc.lpszClassName,												// 앞서 등록한 창클래스의 이름. 이름을 통해 CreateWindow함수는 어떤 창 클래스를 사용하여 창을 생성할지 알수 있다.
		L"MangoWaffle Graphics",										// 생성될 창의 제목. 클래스의 이름과는 다르다.
		WS_POPUP,											// 창의 스타일을 설정한다. 무슨 창이 있는지는 가서 한번 봐봐
		m_WindowPosX,													// 윈도우 좌측 상단의 x 좌표	
		m_WindowPosY,													// 윈도우 좌측 상단의 y 좌표	
		m_ScreenWidth,													// 윈도우 가로 방향 해상도	
		m_ScreenHeight,													// 윈도우 세로 방향 해상도	
		NULL,
		NULL,
		wc.hInstance,													//앞서 GetModuleHandle(NULL) 함수를 통해 가져온 인스턴스 핸들
		NULL);

	if (!m_hWnd)
	{
		std::cout << "m_hWnd - CreateWindow Error\n";
		return E_FAIL;
	}

	// 실제로 그려지는 해상도를 설정하기 위해
	RECT wr = { m_WindowPosX, m_WindowPosY, m_ScreenWidth, m_ScreenHeight };	// Rect구조체를 사용하여 창의 크기를 결정한다. set the size, but not the position
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);							// AdjustWindowRect 함수를 호출하여 창의 크기를 조정, adjust the size 함수는 주어진 스타일(WS_OVERLAPPEDWINDOW)에 따라 창의 크기를 조정한다.

	ShowWindow(m_hWnd, SW_SHOWDEFAULT);											// ShowWindow 함수를 호출하여 창을 화면에 표시, hwnd는 앞서 CreateWindow 함수를 통해 생성된 창의 핸들, SW_SHOWDEFAULT는 창을 기본 상태로 표시하도록 지시
	UpdateWindow(m_hWnd);														// UpdateWindow 함수를 호출하여 창을 즉시 업데이트

	m_pGameEngine = new GameEngine(m_hWnd, m_ScreenWidth, m_ScreenHeight);
	m_pGameEngine->Initialize();
	return S_OK;
}

void GameProcess::Loop()
{
	while (true)
	{
		if (PeekMessage(&m_Msg, NULL, 0, 0, PM_REMOVE))
		{
			if (m_Msg.message == WM_QUIT) break;


			DispatchMessage(&m_Msg);
		}

		//게임엔진의 루프로 들어간다. 
		m_pGameEngine->GameLoop();
	}

}

void GameProcess::Finalize()
{
	delete m_pGameEngine;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//WndProc함수는 창에 대한 피드백만 주고 받게.
LRESULT CALLBACK GameProcess::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
	{
		return true;
	}

	switch (message)
	{

	case WM_SIZE:																//윈도우의 크기가 변경될 때 발생하는 메세지 
		// Reset and resize swapchain											// 이 메세지를 처리하여 스왑체인을 재설정한다.
		return 0;
	case WM_SYSCOMMAND:															//시스템 명령이 발생할 때 발생하는 메시지
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;
	case WM_KEYDOWN:
		if (wParam == 27)
		{
			PostQuitMessage(0);
		}
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}

	return ::DefWindowProc(hWnd, message, wParam, lParam);
}
