#pragma once

#include <functional>
#include <optional>
#include <string>
#include <utility>

#include "RenderContext.h"
#include "RenderPassIO.h"

class IDearsRenderer;

// 한 프레임을 구성하는 렌더 패스 하나의 계약이다.
//
// 그래픽스 엔진은 "패스가 몇 개이고 어떤 순서인지"를 더 이상 직접 알지 않는다.
// 게임 엔진이 RenderPipeline에 IRenderPass를 등록하면, 그래픽스 엔진은 등록된 순서대로
// Execute()만 호출한다. 패스 안에서는 IDearsRenderer가 제공하는 함수만 사용하므로
// 패스 코드가 DX11 구현 클래스에 묶이지 않는다.
//
// 재사용할 패스는 이 인터페이스(또는 RenderPassBase)를 상속해서 클래스로 만들고,
// 한 번 쓰고 말 패스는 RenderPassDesc에 람다를 담아 RenderPipeline::AddPass()로 등록한다.
class IRenderPass
{
public:
	virtual ~IRenderPass() = default;

	// 파이프라인 안에서 패스를 찾고, 끼워 넣고, 켜고 끄는 기준이 되는 고유 이름이다.
	virtual const std::string& GetName() const = 0;
	virtual RenderPassType GetType() const = 0;

	virtual bool IsEnabled() const = 0;
	virtual void SetEnabled(bool enabled) = 0;

	virtual void Execute(IDearsRenderer& renderer) = 0;
};

// 이름/타입/활성 상태처럼 모든 패스가 똑같이 갖는 부분을 미리 구현해 둔 기반 클래스이다.
// 클래스형 패스는 Execute()만 구현하면 된다.
class RenderPassBase : public IRenderPass
{
public:
	RenderPassBase(std::string name, RenderPassType type)
		: m_name(std::move(name))
		, m_type(type)
	{
	}

	const std::string& GetName() const override { return m_name; }
	RenderPassType GetType() const override { return m_type; }
	bool IsEnabled() const override { return m_isEnabled; }
	void SetEnabled(bool enabled) override { m_isEnabled = enabled; }

private:
	std::string m_name;
	RenderPassType m_type = RenderPassType::Custom;
	bool m_isEnabled = true;
};

// 람다로 패스를 정의할 때 사용하는 설명 구조체이다.
//
//   pipeline.AddPass({
//       .name = "Forward",
//       .type = RenderPassType::Forward,
//       .camera = mainCamera,
//       .commonBuffer = &sceneBuffer,
//       .execute = [this](IDearsRenderer& renderer) { ... },
//   });
struct RenderPassDesc
{
	std::string name;

	// 패스의 성격을 나타내는 분류이다. 디버그 마커 이름으로 쓰이고,
	// 아래 io를 비워 두었을 때 적용할 기본 입출력 구성(GetDefaultRenderPassIO)을 고른다.
	// 예: Geometry는 G-Buffer에 쓰고, Lighting은 G-Buffer를 읽어 백 버퍼에 쓴다.
	// Custom은 기본 구성이 없으므로 직전 패스의 렌더 타깃을 그대로 이어서 쓴다.
	RenderPassType type = RenderPassType::Custom;

	// 이 패스의 View/Projection과 공통 상수 버퍼이다. 소유권은 등록한 쪽에 있다.
	// nullptr이면 해당 항목은 직전 패스의 상태를 유지한다.
	Camera* camera = nullptr;
	CommonConstantBufferData* commonBuffer = nullptr;

	// 이 패스가 어디에 쓰고 무엇을 읽을지 직접 지정한다. 값을 넣으면 type의 기본 구성 대신 이것을 쓴다.
	//
	//   .io = RenderPassIO{
	//       .color = PassColorOutput::Offscreen,
	//       .offscreenTarget = minimapTarget,
	//       .clearColor = true,
	//       .clearDepth = true,
	//   },
	std::optional<RenderPassIO> io;

	// true면 execute 직전에 { type, camera, commonBuffer, io }로 ApplyRenderContext()를 호출한다.
	// Shadow처럼 한 패스 안에서 카메라와 타깃을 여러 번 바꾸는 패스는 false로 두고
	// execute 안에서 직접 준비한다.
	bool applyContext = true;

	// 비어 있지 않으면 매 프레임 호출해서 false일 때 이 패스를 건너뛴다.
	// SetPassEnabled()가 "사용자가 끈 상태"라면, 이것은 "이번 프레임에 그릴 것이 없는 상태"이다.
	std::function<bool()> shouldExecute;

	std::function<void(IDearsRenderer&)> execute;
};
