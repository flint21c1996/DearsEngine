#pragma once

// 에디터 창을 기본 배치에서 어느 영역에 붙일지 나타낸다.
//
//   +---------------------------+-----------+
//   |                           |   Right   |
//   |          Center           +-----------+
//   |        (Viewport)         |  Right    |
//   +---------------------------+  Bottom   |
//   |          Bottom           |           |
//   +---------------------------+-----------+
//
// 이 값은 "처음 실행했을 때"와 "Window > Reset Layout"을 눌렀을 때의 자리만 정한다.
// 그 뒤에는 사용자가 창을 끌어 옮긴 대로 imgui.ini에 저장되고 다음 실행에 복원된다.
enum class EditorDockSlot
{
	Center,
	Right,
	RightBottom,
	Bottom,
	// 도킹하지 않고 떠 있는 창으로 둔다.
	Floating,
};
