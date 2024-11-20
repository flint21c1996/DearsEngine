#pragma once

enum class KEY_STATE
{
	NONE,						//아무것도 눌리지 않은 상태
	TAP, 						//키가 방금 막 눌린 상태
	HOLD, 						//키가 계속 눌려있는 상태
	AWAY 						//키가 방금 막 떼진 상태
};

//인풋매니저의 키 배열과 동등한 위치를 가져야한다.
enum class KEY
{
	LEFT, RIGHT, UP, DOWN,
	Q, W, E, R, T, Y, U, I,
	O, P, A, S, D, F, G, Z,
	X, C, V, B, N, M,
	_1, _2, _3, _4, _5, _6, _7, _8, _9, _0,
	ALT, CTRL, SPACE, ENTER, ESC,
	LSHIFT, RSHIFT, LBUTTON, RBUTTON,
	TAB,

	LAST,

};