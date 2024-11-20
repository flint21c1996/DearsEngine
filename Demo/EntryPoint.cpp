#include <windows.h>
#include "GameProcess.h"

///����� ����� ��� �ܼ�â�� ����.
#ifdef _DEBUG

#ifdef UNICODE                                                                                      
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console") 
#else                                                                                                    
#pragma comment(linker, "/entry:WinMainCRTStartup")   
#endif                                                                                                   

#endif                                                                                                   

/// ���α׷��� ������
int WINAPI wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	//�Ʒ��� ������ �����ʴ´�. 
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	/// ���� ������ �ô� Ŭ����, ���ο��� ���� ������ �޽��� �ݹ��� ó��. ���� ���ӿ��� ���� �����Ѵ�.
	GameProcess* pGameProcess = new GameProcess();

	pGameProcess->Initialize(hInstance);
	pGameProcess->Loop();
	pGameProcess->Finalize();

	delete pGameProcess;

}
