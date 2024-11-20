#include <windows.h>
#include "GameProcess.h"

///디버그 모드일 경우 콘솔창을 띄운다.
#ifdef _DEBUG

#ifdef UNICODE                                                                                      
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console") 
#else                                                                                                    
#pragma comment(linker, "/entry:WinMainCRTStartup")   
#endif                                                                                                   

#endif                                                                                                   

/// 프로그램의 진입점
int WINAPI wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	//아래의 변수는 쓰지않는다. 
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	/// 게임 진행을 맡는 클래스, 내부에서 윈도 생성과 메시지 콜백을 처리. 또한 게임엔진 등을 포함한다.
	GameProcess* pGameProcess = new GameProcess();

	pGameProcess->Initialize(hInstance);
	pGameProcess->Loop();
	pGameProcess->Finalize();

	delete pGameProcess;

}
