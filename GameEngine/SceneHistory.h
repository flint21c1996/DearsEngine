#pragma once

#include <string>
#include <utility>
#include <vector>

// 실행 취소(Ctrl+Z) / 다시 실행(Ctrl+Y)을 위한 씬 스냅샷 기록이다.
//
// "무엇을 바꿨는지"를 명령 단위로 기록하는 대신, 편집이 끝날 때마다 씬 전체를 문자열로 저장해 둔다.
// 이동, 삭제, 속성 변경, 프리셋 지정 등 어떤 편집이든 같은 방식으로 되돌릴 수 있고,
// 새 편집 기능을 추가할 때 되돌리는 코드를 따로 만들지 않아도 된다.
// 씬 파일과 같은 텍스트 형식을 쓰므로 오브젝트 수십 개 규모에서는 메모리 부담이 작다.
//
// 이 클래스는 문자열만 다루기 때문에 그래픽스 디바이스나 창 없이 테스트할 수 있다.
class SceneHistory
{
public:
	explicit SceneHistory(size_t capacity = 100)
		: m_capacity(capacity < 2 ? 2 : capacity)
	{
	}

	// 기록을 비우고 현재 상태 하나만 남긴다. 씬을 새로 열었을 때 호출한다.
	void Reset(std::string currentState)
	{
		m_states.clear();
		m_states.push_back(std::move(currentState));
		m_current = 0;
	}

	// 편집이 끝난 뒤의 상태를 기록한다. 직전 상태와 같으면(실제로 바뀐 것이 없으면) 기록하지 않는다.
	// 실행 취소를 몇 번 한 뒤에 새 편집을 하면, 그 뒤쪽의 "다시 실행" 기록은 버려진다.
	bool Commit(std::string state)
	{
		if (m_states.empty())
		{
			Reset(std::move(state));
			return true;
		}
		if (m_states[m_current] == state)
		{
			return false;
		}

		m_states.resize(m_current + 1);
		m_states.push_back(std::move(state));
		++m_current;

		// 오래된 기록부터 버려서 무한히 커지지 않게 한다.
		if (m_states.size() > m_capacity)
		{
			const size_t overflow = m_states.size() - m_capacity;
			m_states.erase(m_states.begin(), m_states.begin() + static_cast<std::ptrdiff_t>(overflow));
			m_current -= overflow;
		}
		return true;
	}

	bool CanUndo() const { return !m_states.empty() && m_current > 0; }
	bool CanRedo() const { return !m_states.empty() && m_current + 1 < m_states.size(); }

	// 한 단계 되돌리고, 복원해야 할 상태를 돌려준다. 더 되돌릴 것이 없으면 nullptr이다.
	// 돌려준 포인터는 다음 Commit/Reset 전까지만 유효하다.
	const std::string* Undo()
	{
		if (!CanUndo())
		{
			return nullptr;
		}
		--m_current;
		return &m_states[m_current];
	}

	const std::string* Redo()
	{
		if (!CanRedo())
		{
			return nullptr;
		}
		++m_current;
		return &m_states[m_current];
	}

	size_t GetUndoCount() const { return m_states.empty() ? 0 : m_current; }
	size_t GetRedoCount() const { return m_states.empty() ? 0 : m_states.size() - 1 - m_current; }

private:
	std::vector<std::string> m_states;
	size_t m_current = 0;
	size_t m_capacity;
};
