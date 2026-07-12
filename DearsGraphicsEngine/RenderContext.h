#pragma once

#include "BufferData.h"

class Camera;

// 렌더러가 구분해서 다룰 수 있는 렌더 패스 종류이다.
//
// 이전에도 GameEngine::RenderShadowPass() 같은 함수 이름으로 패스가 구분되었지만,
// 그 정보는 "사람이 읽는 함수 이름"에만 있었다.
// RenderPassType을 두면 코드도 현재 패스가 Shadow인지, Scene인지 판단할 수 있다.
//
// 이 enum은 나중에 RHI에서 다음 작업을 결정하는 기준이 된다.
// 예:
// - Shadow: depth target에 쓰기, color target 불필요
// - Geometry: 불투명 오브젝트의 표면 정보를 G-Buffer에 기록
// - Lighting: G-Buffer를 읽어서 화면 단위로 조명을 계산
// - Forward: 투명/특수 오브젝트를 scene color target에 직접 렌더링
// - PostProcess: 이전 scene color를 읽고 최종 render target에 쓰기
// - Debug/UI: 메인 씬 이후 overlay 성격으로 그리기
enum class RenderPassType
{
	Unknown,
	Shadow,
	Geometry,
	Lighting,
	Forward,
	Particle,
	PostProcess,
	SelectionOutline,
	Debug,
	Ui,
};

// 디버깅이나 로그 출력에서 사용할 수 있도록 RenderPassType을 문자열로 바꾼다.
// 지금은 직접 로그를 찍지 않지만, PIX/RenderDoc marker나 자체 profiler를 붙일 때
// 이 이름을 그대로 pass marker로 사용할 수 있다.
inline const char* ToRenderPassName(RenderPassType passType)
{
	switch (passType)
	{
	case RenderPassType::Shadow:
		return "ShadowPass";
	case RenderPassType::Geometry:
		return "GeometryPass";
	case RenderPassType::Lighting:
		return "LightingPass";
	case RenderPassType::Forward:
		return "ForwardPass";
	case RenderPassType::Particle:
		return "ParticlePass";
	case RenderPassType::PostProcess:
		return "PostProcessPass";
	case RenderPassType::SelectionOutline:
		return "SelectionOutlinePass";
	case RenderPassType::Debug:
		return "DebugPass";
	case RenderPassType::Ui:
		return "UiPass";
	default:
		return "UnknownPass";
	}
}

// 한 렌더 패스가 사용할 공통 렌더 상태를 묶어 전달하는 구조체이다.
//
// 지금은 카메라와 CommonConstantBufferData만 담지만,
// 나중에는 render target, viewport, clear color, resource transition 정보가 여기에 같이 들어올 수 있다.
//
// DX12/Vulkan에서는 "지금 어떤 패스를 어떤 상태로 실행하는지"를 명확히 알아야 하므로,
// RenderContext는 RHI command list를 만들기 위한 출발점이 된다.
struct RenderContext
{
	RenderPassType passType = RenderPassType::Unknown;
	Camera* camera = nullptr;
	CommonConstantBufferData* commonBuffer = nullptr;

	const char* GetPassName() const
	{
		return ToRenderPassName(passType);
	}
};
