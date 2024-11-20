#pragma once
#include <vector>
#include "InputEnum.h" 
#include "DirectxTK/SimpleMath.h"
using DirectX::SimpleMath::Vector2;

/// <summary>
/// Ű���� ���콺�� input�� �����ϴ� �Ŵ���
/// </summary>
class InputManager
{
private:
	static InputManager* instance;
	static HWND m_hwnd;

public:							///�� �κ� ���߿� �ܺη� �E�� ����� �� �غ��ô�.
	struct KeyInfo
	{
		KEY_STATE state;
		bool isPushed;
	};

private:						///�̱ۺ�����
	InputManager();
	~InputManager();
public:							///�̱������ �Լ��� �ڵ� �ĺ��ǰ� �ٸ��� ���⼭ 
	// �ν��Ͻ��� �����ϱ� ���� static �޼ҵ�
	static InputManager* getInstance();

	//�ν���Ʈ�� �����ֱ� ���� �Լ�
	static void destroyInstance();

	//�̱������� �ٲٸ鼭 �׳� ��������� ������ ������.
	void SetHwnd(HWND hwnd);

public:
	void Initialize();
	void Update();
	void Finalize();


	//enum class�� �����ϰ� ��� �־�� �Ѵ�.
	std::vector<int> m_key;

private:
	std::vector<KeyInfo> m_keyInfo;

	//���콺 ������
	Vector2 m_currentMousePos;

	//���� �������� ���콺 ������
	Vector2 m_prevMousePos;

	//���콺�� ������ ����
	DirectX::SimpleMath::Vector2 deltaMousePos;

private:
	//â�� ��Ŀ�� ���θ� �Ǻ��Ͽ� ��Ŀ�� ���°� �ƴ϶�� ��� �Է��� ���� �ʴ´�.
	void InFocus();

	//â�� �ƿ���Ŀ�� �Ǿ��� ��
	void OutFocus();

	//���콺�� ��ġ
	void MousePos();

	//���콺�� ������ ����, ���߿� �� ���� ����� �ʿ��� ���� �ִ�.
	DirectX::SimpleMath::Vector2 GetDeltaMousePos();

public:

	///�ܺο� Input������ �˷��ִ� �Լ���
	//Ű���� Ű�� �ԷµǾ������� Key_STATE�� �����Ѵ�.
	KEY_STATE GetKeyState(KEY _Key);

	//���콺�� ��ġ�� �����Ѵ�.
	Vector2 GetMousePos();
};

