#pragma once
#include <windows.h>

class GameEngine;

class GameProcess
{
public:
	GameProcess();
	~GameProcess();
private:
	// �������� �»�� - �̰����� â�� ��ġ�� �����Ҽ� �ִ�.
	int m_WindowPosX;
	int m_WindowPosY;

	// ȭ���� ũ��
	int m_ScreenWidth;
	int m_ScreenHeight;

	// ���� ����
	HWND m_hWnd;
	MSG m_Msg;

	//���ӿ���
	GameEngine* m_pGameEngine;
public:
	//���� ���μ���
	HRESULT Initialize(HINSTANCE hInstance);		//���ӿ��� �ʱ�ȭ
	void Loop();
	void Finalize();

	// �޽��� �ڵ鷯 (���� �ݹ�)
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

