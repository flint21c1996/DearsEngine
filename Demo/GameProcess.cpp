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
	//�ü������ â Ŭ������ �����ϰ� ����Ѵ�. WNDCLASSEX�� â Ŭ������ �Ӽ��� �����ϴ� ����ü�̴�. 
	WNDCLASSEX wc = {
		sizeof(WNDCLASSEX),												//����ü�� ũ�⸦ ��Ÿ����. 
		CS_CLASSDC,														//â Ŭ���� ��Ÿ���� �����Ѵ�.
		WndProc,														//â ���ν����� ����Ų��.  â ���ν����� â�� �޽����� ��� ó�������� �����ϴ� �Լ��̴�. 
		0,
		0,
		GetModuleHandle(NULL),											//�ν��Ͻ� �ڵ��� �����´�. NULL�� �� ���� �������� �ν��Ͻ��� �ڵ��� �����´�.
		NULL,
		NULL,
		NULL,
		NULL,
		L"MangoWaffle",													// lpszClassName, L-string, â Ŭ������ �̸��� �����Ѵ�. 
		NULL };

	RegisterClassEx(&wc);												//�� â�� �ý��ۿ� ����ϴ� �Լ��� ȣ��, �� â�� ȣ�� ������ �츮�� �� â Ŭ������ �����Ҽ� �ְ� �ȴ�. 


	///Window â�� �����ϴ� �κ�, CreateWindow�Լ��� �־��� �Ű������� ���� ���ο� â�� �����ϰ�, �� â�� �ڵ��� ��ȯ�Ѵ�.
	m_hWnd = CreateWindow(
		wc.lpszClassName,												// �ռ� ����� âŬ������ �̸�. �̸��� ���� CreateWindow�Լ��� � â Ŭ������ ����Ͽ� â�� �������� �˼� �ִ�.
		L"MangoWaffle Graphics",										// ������ â�� ����. Ŭ������ �̸����� �ٸ���.
		WS_POPUP,											// â�� ��Ÿ���� �����Ѵ�. ���� â�� �ִ����� ���� �ѹ� ����
		m_WindowPosX,													// ������ ���� ����� x ��ǥ	
		m_WindowPosY,													// ������ ���� ����� y ��ǥ	
		m_ScreenWidth,													// ������ ���� ���� �ػ�	
		m_ScreenHeight,													// ������ ���� ���� �ػ�	
		NULL,
		NULL,
		wc.hInstance,													//�ռ� GetModuleHandle(NULL) �Լ��� ���� ������ �ν��Ͻ� �ڵ�
		NULL);

	if (!m_hWnd)
	{
		std::cout << "m_hWnd - CreateWindow Error\n";
		return E_FAIL;
	}

	// ������ �׷����� �ػ󵵸� �����ϱ� ����
	RECT wr = { m_WindowPosX, m_WindowPosY, m_ScreenWidth, m_ScreenHeight };	// Rect����ü�� ����Ͽ� â�� ũ�⸦ �����Ѵ�. set the size, but not the position
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);							// AdjustWindowRect �Լ��� ȣ���Ͽ� â�� ũ�⸦ ����, adjust the size �Լ��� �־��� ��Ÿ��(WS_OVERLAPPEDWINDOW)�� ���� â�� ũ�⸦ �����Ѵ�.

	ShowWindow(m_hWnd, SW_SHOWDEFAULT);											// ShowWindow �Լ��� ȣ���Ͽ� â�� ȭ�鿡 ǥ��, hwnd�� �ռ� CreateWindow �Լ��� ���� ������ â�� �ڵ�, SW_SHOWDEFAULT�� â�� �⺻ ���·� ǥ���ϵ��� ����
	UpdateWindow(m_hWnd);														// UpdateWindow �Լ��� ȣ���Ͽ� â�� ��� ������Ʈ

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

		//���ӿ����� ������ ����. 
		m_pGameEngine->GameLoop();
	}

}

void GameProcess::Finalize()
{
	delete m_pGameEngine;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//WndProc�Լ��� â�� ���� �ǵ�鸸 �ְ� �ް�.
LRESULT CALLBACK GameProcess::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
	{
		return true;
	}

	switch (message)
	{

	case WM_SIZE:																//�������� ũ�Ⱑ ����� �� �߻��ϴ� �޼��� 
		// Reset and resize swapchain											// �� �޼����� ó���Ͽ� ����ü���� �缳���Ѵ�.
		return 0;
	case WM_SYSCOMMAND:															//�ý��� ����� �߻��� �� �߻��ϴ� �޽���
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
