#include "RenderPipeline.h"

#include <utility>

#include "IDearsRenderer.h"

namespace
{
// RenderPassDesc(람다)로 등록한 패스를 IRenderPass로 감싸는 내부 구현이다.
// 파이프라인 바깥에서는 이 타입을 알 필요가 없으므로 cpp 안에 숨긴다.
class FunctionRenderPass final : public RenderPassBase
{
public:
	explicit FunctionRenderPass(RenderPassDesc desc)
		: RenderPassBase(desc.name, desc.type)
		, m_desc(std::move(desc))
	{
	}

	void Execute(IDearsRenderer& renderer) override
	{
		if (m_desc.shouldExecute && !m_desc.shouldExecute())
		{
			return;
		}

		if (m_desc.applyContext)
		{
			RenderContext context;
			context.passType = m_desc.type;
			context.camera = m_desc.camera;
			context.commonBuffer = m_desc.commonBuffer;
			context.io = m_desc.io ? &*m_desc.io : nullptr;
			renderer.ApplyRenderContext(context);
		}

		if (m_desc.execute)
		{
			m_desc.execute(renderer);
		}
	}

private:
	RenderPassDesc m_desc;
};

// 실행 중 예외가 나더라도 m_isExecuting이 true로 남지 않게 한다.
class ExecutingScope
{
public:
	explicit ExecutingScope(bool& flag)
		: m_flag(flag)
	{
		m_flag = true;
	}

	~ExecutingScope()
	{
		m_flag = false;
	}

private:
	bool& m_flag;
};
}

IRenderPass* RenderPipeline::AddPass(std::unique_ptr<IRenderPass> pass)
{
	return InsertAt(m_passes.size(), std::move(pass));
}

IRenderPass* RenderPipeline::AddPass(RenderPassDesc desc)
{
	return AddPass(std::make_unique<FunctionRenderPass>(std::move(desc)));
}

IRenderPass* RenderPipeline::InsertPassBefore(const std::string& anchorName, std::unique_ptr<IRenderPass> pass)
{
	return InsertRelative(anchorName, std::move(pass), false);
}

IRenderPass* RenderPipeline::InsertPassBefore(const std::string& anchorName, RenderPassDesc desc)
{
	return InsertPassBefore(anchorName, std::make_unique<FunctionRenderPass>(std::move(desc)));
}

IRenderPass* RenderPipeline::InsertPassAfter(const std::string& anchorName, std::unique_ptr<IRenderPass> pass)
{
	return InsertRelative(anchorName, std::move(pass), true);
}

IRenderPass* RenderPipeline::InsertPassAfter(const std::string& anchorName, RenderPassDesc desc)
{
	return InsertPassAfter(anchorName, std::make_unique<FunctionRenderPass>(std::move(desc)));
}

bool RenderPipeline::MovePassBefore(const std::string& name, const std::string& anchorName)
{
	return MoveRelative(name, anchorName, false);
}

bool RenderPipeline::MovePassAfter(const std::string& name, const std::string& anchorName)
{
	return MoveRelative(name, anchorName, true);
}

bool RenderPipeline::RemovePass(const std::string& name)
{
	if (m_isExecuting)
	{
		return false;
	}

	const size_t index = FindIndex(name);
	if (index == InvalidIndex)
	{
		return false;
	}

	m_passes.erase(m_passes.begin() + static_cast<std::ptrdiff_t>(index));
	return true;
}

bool RenderPipeline::Clear()
{
	if (m_isExecuting)
	{
		return false;
	}

	m_passes.clear();
	return true;
}

bool RenderPipeline::SetPassEnabled(const std::string& name, bool enabled)
{
	IRenderPass* pass = FindPass(name);
	if (!pass)
	{
		return false;
	}

	pass->SetEnabled(enabled);
	return true;
}

bool RenderPipeline::IsPassEnabled(const std::string& name) const
{
	const IRenderPass* pass = FindPass(name);
	return pass && pass->IsEnabled();
}

IRenderPass* RenderPipeline::FindPass(const std::string& name) const
{
	const size_t index = FindIndex(name);
	return index == InvalidIndex ? nullptr : m_passes[index].get();
}

std::vector<std::string> RenderPipeline::GetPassNames() const
{
	std::vector<std::string> names;
	names.reserve(m_passes.size());
	for (const std::unique_ptr<IRenderPass>& pass : m_passes)
	{
		names.push_back(pass->GetName());
	}
	return names;
}

void RenderPipeline::Execute(IDearsRenderer& renderer)
{
	// 패스 안에서 RenderFrame()을 다시 부르는 재진입은 무한 루프가 되므로 막는다.
	if (m_isExecuting)
	{
		return;
	}

	ExecutingScope scope(m_isExecuting);
	renderer.BeginShadowFrame();
	IRenderPass* shadowProducer = nullptr;

	// 실행 중에는 목록 구조 변경을 막아 두었으므로 인덱스 순회가 안전하다.
	// 켜고 끄기는 허용되기 때문에, 앞선 패스가 뒤의 패스를 끄면 같은 프레임에 바로 반영된다.
	for (size_t index = 0; index < m_passes.size(); ++index)
	{
		IRenderPass* pass = m_passes[index].get();
		// Direct SetEnabled(false) must invalidate maps before later consumers.
		if (shadowProducer && !shadowProducer->IsEnabled())
		{
			renderer.BeginShadowFrame();
			shadowProducer = nullptr;
		}
		if (pass->GetType() == RenderPassType::Shadow)
		{
			// Each producer supersedes the preceding one, even when skipped.
			shadowProducer = pass;
			renderer.BeginShadowProduction();
			try
			{
				if (pass->IsEnabled()) pass->Execute(renderer);
				renderer.EndShadowProduction(pass->IsEnabled());
			}
			catch (...)
			{
				renderer.EndShadowProduction(false);
				throw;
			}
		}
		else if (pass->IsEnabled())
		{
			pass->Execute(renderer);
		}
	}
}

size_t RenderPipeline::FindIndex(const std::string& name) const
{
	for (size_t index = 0; index < m_passes.size(); ++index)
	{
		if (m_passes[index]->GetName() == name)
		{
			return index;
		}
	}
	return InvalidIndex;
}

bool RenderPipeline::CanRegister(const IRenderPass* pass) const
{
	// 이름으로 패스를 찾기 때문에 빈 이름과 중복 이름은 받지 않는다.
	return !m_isExecuting &&
		pass &&
		!pass->GetName().empty() &&
		FindIndex(pass->GetName()) == InvalidIndex;
}

IRenderPass* RenderPipeline::InsertAt(size_t index, std::unique_ptr<IRenderPass> pass)
{
	if (!CanRegister(pass.get()) || index > m_passes.size())
	{
		return nullptr;
	}

	IRenderPass* registeredPass = pass.get();
	m_passes.insert(m_passes.begin() + static_cast<std::ptrdiff_t>(index), std::move(pass));
	return registeredPass;
}

IRenderPass* RenderPipeline::InsertRelative(const std::string& anchorName, std::unique_ptr<IRenderPass> pass, bool after)
{
	const size_t anchorIndex = FindIndex(anchorName);
	if (anchorIndex == InvalidIndex)
	{
		return nullptr;
	}

	return InsertAt(after ? anchorIndex + 1 : anchorIndex, std::move(pass));
}

bool RenderPipeline::MoveRelative(const std::string& name, const std::string& anchorName, bool after)
{
	if (m_isExecuting || name == anchorName)
	{
		return false;
	}

	const size_t sourceIndex = FindIndex(name);
	if (sourceIndex == InvalidIndex || FindIndex(anchorName) == InvalidIndex)
	{
		return false;
	}

	std::unique_ptr<IRenderPass> pass = std::move(m_passes[sourceIndex]);
	m_passes.erase(m_passes.begin() + static_cast<std::ptrdiff_t>(sourceIndex));

	// 원본을 뺀 뒤에 anchor 위치를 다시 구해야 인덱스가 한 칸 밀리는 경우를 따로 계산하지 않아도 된다.
	const size_t anchorIndex = FindIndex(anchorName);
	const size_t targetIndex = after ? anchorIndex + 1 : anchorIndex;
	m_passes.insert(m_passes.begin() + static_cast<std::ptrdiff_t>(targetIndex), std::move(pass));
	return true;
}
