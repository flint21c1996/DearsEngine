#pragma once

enum class KEY_STATE
{
	NONE,						//�ƹ��͵� ������ ���� ����
	TAP, 						//Ű�� ��� �� ���� ����
	HOLD, 						//Ű�� ��� �����ִ� ����
	AWAY 						//Ű�� ��� �� ���� ����
};

//��ǲ�Ŵ����� Ű �迭�� ������ ��ġ�� �������Ѵ�.
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