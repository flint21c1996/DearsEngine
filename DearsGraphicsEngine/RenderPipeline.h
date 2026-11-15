#pragma once

#include <memory>
#include <string>
#include <vector>

#include "IRenderPass.h"

class IDearsRenderer;

// 한 프레임에 실행할 렌더 패스의 "순서 있는 목록"이다.
//
// 예전에는 패스 순서가 GameEngine::Render() 안의 함수 호출 순서로 고정되어 있어서,
// 패스를 추가하거나 끄거나 순서를 바꾸려면 엔진 코드를 직접 고쳐야 했다.
// 이제는 게임 엔진이 초기화 시점에 이 목록을 구성하고, 실행 중에도 이름으로
// 패스를 끼워 넣거나 끌 수 있다.
//
// 규칙:
// - 패스 이름은 파이프라인 안에서 고유해야 한다. 중복 이름은 등록에 실패한다.
// - Execute() 도중에는 목록 구조(추가/삭제/이동/Clear)를 바꿀 수 없다. 실패로 처리한다.
//   켜고 끄기(SetPassEnabled)는 실행 중에도 허용된다.
// - 실패한 Add/Insert는 nullptr을 반환하고 넘겨받은 패스를 파괴한다.
class RenderPipeline
{
public:
	RenderPipeline() = default;
	RenderPipeline(const RenderPipeline&) = delete;
	RenderPipeline& operator=(const RenderPipeline&) = delete;

	// 목록의 끝에 추가한다.
	IRenderPass* AddPass(std::unique_ptr<IRenderPass> pass);
	IRenderPass* AddPass(RenderPassDesc desc);

	// anchorName 패스의 바로 앞/뒤에 끼워 넣는다. anchor가 없으면 실패한다.
	IRenderPass* InsertPassBefore(const std::string& anchorName, std::unique_ptr<IRenderPass> pass);
	IRenderPass* InsertPassBefore(const std::string& anchorName, RenderPassDesc desc);
	IRenderPass* InsertPassAfter(const std::string& anchorName, std::unique_ptr<IRenderPass> pass);
	IRenderPass* InsertPassAfter(const std::string& anchorName, RenderPassDesc desc);

	// 이미 등록된 패스를 다른 패스의 앞/뒤로 옮긴다.
	bool MovePassBefore(const std::string& name, const std::string& anchorName);
	bool MovePassAfter(const std::string& name, const std::string& anchorName);

	bool RemovePass(const std::string& name);
	bool Clear();

	bool SetPassEnabled(const std::string& name, bool enabled);
	bool IsPassEnabled(const std::string& name) const;

	IRenderPass* FindPass(const std::string& name) const;
	bool HasPass(const std::string& name) const { return FindPass(name) != nullptr; }
	size_t GetPassCount() const { return m_passes.size(); }
	// 에디터 패널이나 디버그 출력에서 현재 순서를 보여줄 때 사용한다.
	std::vector<std::string> GetPassNames() const;

	// 활성화된 패스를 등록 순서대로 실행한다.
	void Execute(IDearsRenderer& renderer);
	bool IsExecuting() const { return m_isExecuting; }

private:
	static const size_t InvalidIndex = static_cast<size_t>(-1);

	size_t FindIndex(const std::string& name) const;
	bool CanRegister(const IRenderPass* pass) const;
	IRenderPass* InsertAt(size_t index, std::unique_ptr<IRenderPass> pass);
	IRenderPass* InsertRelative(const std::string& anchorName, std::unique_ptr<IRenderPass> pass, bool after);
	bool MoveRelative(const std::string& name, const std::string& anchorName, bool after);

	std::vector<std::unique_ptr<IRenderPass>> m_passes;
	bool m_isExecuting = false;
};
