#include "InputManager.h"

//싱글톤 인스턴트 초기화
InputManager* InputManager::instance = nullptr;
HWND InputManager::m_hwnd = 0;
InputManager::InputManager()
{
	Initialize();
}

InputManager::~InputManager()
{

}

InputManager* InputManager::getInstance()
{
	if (instance == nullptr)				// 인스턴스가 아직 생성되지 않았다면
	{
		instance = new InputManager();		// 인스턴스 생성
		instance->Initialize();
	}
	return instance;						// 생성된 인스턴스 반환
}

void InputManager::destroyInstance()
{
	if (instance != nullptr)
	{
		delete instance;
		instance = nullptr;
	}
}

void InputManager::SetHwnd(HWND _hwnd)
{
	m_hwnd = _hwnd;
}

void InputManager::Initialize()
{
	m_key =
	{
		VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN,
		'Q','W','E','R','T','Y','U','I',
		'O','P','A','S','D','F','G','Z',
		'X','C','V','B', 'N', 'M',
		'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
		VK_MENU, VK_CONTROL, VK_SPACE, VK_RETURN, VK_ESCAPE,
		VK_LSHIFT, VK_RSHIFT, VK_LBUTTON, VK_RBUTTON, VK_TAB
	};

	for (int i = 0; i < static_cast<int>(KEY::LAST); ++i)
		m_keyInfo.push_back(KeyInfo{ KEY_STATE::NONE,false });
}

void InputManager::Update()
{
	if (nullptr != GetFocus())
	{
		MousePos();
		InFocus();
	}
	else
	{
		OutFocus();
	}
}

void InputManager::Finalize()
{

}

///창이 포커스 되어 있을 떄
void InputManager::InFocus()
{
	for (int i = 0; i < static_cast<int>(KEY::LAST); ++i)
	{
		if (GetAsyncKeyState(m_key[i]) & 0x8000)
		{
			if (m_keyInfo[i].isPushed)
			{
				m_keyInfo[i].state = KEY_STATE::HOLD;

			}
			else
			{
				m_keyInfo[i].state = KEY_STATE::TAP;
				//디버그 모드용
				//std::cout << std::endl;
				//std::cout << static_cast<char>(m_key[i]) << std::endl;
				//std::cout << "마우스 위치 : " << m_currentMousePos.x << " / " << m_currentMousePos.y << std::endl;
			}

			m_keyInfo[i].isPushed = true;
		}
		else
		{
			if (m_keyInfo[i].isPushed)
			{
				m_keyInfo[i].state = KEY_STATE::AWAY;
			}
			else
			{
				m_keyInfo[i].state = KEY_STATE::NONE;
			}


			m_keyInfo[i].isPushed = false;
		}
	}
}

void InputManager::OutFocus()
{
	for (int i = 0; i < static_cast<int>(KEY::LAST); ++i)
	{
		m_keyInfo[i].isPushed = false;
		if (KEY_STATE::TAP == m_keyInfo[i].state ||
			KEY_STATE::HOLD == m_keyInfo[i].state)
		{
			m_keyInfo[i].state = KEY_STATE::AWAY;
		}
		else if (KEY_STATE::AWAY == m_keyInfo[i].state)
		{
			m_keyInfo[i].state = KEY_STATE::NONE;
		}
	}
}

//마우스의 현재포지션을 업데이트한다.
void InputManager::MousePos()
{
	m_prevMousePos = m_currentMousePos;					   // 이전 프레임의 마우스의 위치를 업데이트한다.
	POINT temp;
	GetCursorPos(&temp);
	ScreenToClient(m_hwnd, &temp);
	m_currentMousePos.x = static_cast<float>(temp.x);							   // 현재의 마우스의 위치를 업데이트한다.
	m_currentMousePos.y = static_cast<float>(temp.y);							   // 현재의 마우스의 위치를 업데이트한다.

}

//마우스의 움직인 정도를 계산한다. (현재 프레임의 마우스 위치 - 이전 프레임의 마우스 위치)
DirectX::SimpleMath::Vector2 InputManager::GetDeltaMousePos()
{
	deltaMousePos.x = static_cast<float>(m_currentMousePos.x - m_prevMousePos.x);
	deltaMousePos.y = static_cast<float>(m_currentMousePos.y - m_prevMousePos.y);

	return deltaMousePos;
}

KEY_STATE InputManager::GetKeyState(KEY _Key)
{
	return m_keyInfo[static_cast<int>(_Key)].state;
}

Vector2 InputManager::GetMousePos()
{
	return m_currentMousePos;
}