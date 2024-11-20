#pragma once
#include <vector>
#include "InputEnum.h" 
#include "DirectxTK/SimpleMath.h"
using DirectX::SimpleMath::Vector2;

/// <summary>
/// 키보드 마우스의 input을 관리하는 매니저
/// </summary>
class InputManager
{
private:
	static InputManager* instance;
	static HWND m_hwnd;

public:							///이 부분 나중에 외부로 뺼지 고민을 좀 해봅시다.
	struct KeyInfo
	{
		KEY_STATE state;
		bool isPushed;
	};

private:						///싱글벙글톤
	InputManager();
	~InputManager();
public:							///싱글톤관련 함수는 코딩 컴벤션과 다르게 여기서 
	// 인스턴스에 접근하기 위한 static 메소드
	static InputManager* getInstance();

	//인스턴트를 지워주기 위한 함수
	static void destroyInstance();

	//싱글톤으로 바꾸면서 그냥 명시적으로 세팅을 해주자.
	void SetHwnd(HWND hwnd);

public:
	void Initialize();
	void Update();
	void Finalize();


	//enum class와 동일하게 들고 있어야 한다.
	std::vector<int> m_key;

private:
	std::vector<KeyInfo> m_keyInfo;

	//마우스 포지션
	Vector2 m_currentMousePos;

	//이전 프레임의 마우스 포지션
	Vector2 m_prevMousePos;

	//마우스의 움직임 정도
	DirectX::SimpleMath::Vector2 deltaMousePos;

private:
	//창의 포커싱 여부를 판별하여 포커싱 상태가 아니라면 모든 입력을 받지 않는다.
	void InFocus();

	//창이 아웃포커싱 되었을 때
	void OutFocus();

	//마우스의 위치
	void MousePos();

	//마우스가 움직인 정도, 나중에 그 길이 계산이 필요할 수도 있다.
	DirectX::SimpleMath::Vector2 GetDeltaMousePos();

public:

	///외부에 Input정보를 알려주는 함수들
	//키떠한 키가 입력되었는지를 Key_STATE로 리턴한다.
	KEY_STATE GetKeyState(KEY _Key);

	//마우스의 위치를 리턴한다.
	Vector2 GetMousePos();
};

