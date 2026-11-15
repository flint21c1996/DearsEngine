#pragma once

#include <string>
#include <vector>

#include "RenderContext.h"

// 게임 엔진이 만든 오프스크린 렌더 타깃을 가리키는 값이다.
// DX11 포인터 대신 이 핸들을 주고받기 때문에 패스 코드가 그래픽스 API 타입을 알 필요가 없다.
// 기본값(id 0)은 "유효하지 않음"을 뜻한다.
struct RenderTargetHandle
{
	unsigned int id = 0;

	bool IsValid() const { return id != 0; }
	bool operator==(const RenderTargetHandle& other) const { return id == other.id; }
	bool operator!=(const RenderTargetHandle& other) const { return id != other.id; }
};

enum class RenderTargetFormat
{
	// 일반 색상(0~1). 미니맵, 거울, UI에 띄울 장면 등에 쓴다.
	Rgba8Unorm,
	// HDR 색상. 톤 매핑/블룸 전 단계의 Scene Color처럼 1을 넘는 값이 필요할 때 쓴다.
	Rgba16Float,
};

struct RenderTargetDesc
{
	// FindRenderTarget()으로 다시 찾을 때 쓰는 고유 이름이다. 같은 이름으로 다시 만들면 기존 핸들을 돌려준다.
	std::string name;
	// 0이면 백 버퍼와 같은 크기로 만든다.
	unsigned int width = 0;
	unsigned int height = 0;
	RenderTargetFormat format = RenderTargetFormat::Rgba8Unorm;
	// true면 이 타깃 전용 깊이 버퍼를 함께 만든다.
	bool withDepth = true;
};

// 패스가 색상을 어디에 쓸지 고른다.
enum class PassColorOutput
{
	// 직전 패스가 묶어 둔 출력 대상(색상+깊이)을 그대로 이어서 쓴다.
	Keep,
	// 화면에 표시되는 최종 버퍼.
	BackBuffer,
	// 디퍼드용 Albedo/Normal/Material MRT. 묶을 때마다 G-Buffer 전체와 그 깊이를 지운다.
	GBuffer,
	// CreateRenderTarget()으로 만든 타깃. RenderPassIO::offscreenTarget에 핸들을 넣는다.
	Offscreen,
};

// 패스가 어떤 깊이 버퍼로 깊이 검사/기록을 할지 고른다.
enum class PassDepthOutput
{
	// 색상 출력에 딸린 기본 깊이: BackBuffer는 메인 깊이, GBuffer는 G-Buffer 깊이,
	// Offscreen은 그 타깃의 전용 깊이(없으면 깊이 없음).
	Default,
	// Geometry 패스가 만든 깊이를 공유한다. 디퍼드 결과 위에 포워드 물체를 올바른 앞뒤 관계로 그릴 때 쓴다.
	// 색상 타깃이 백 버퍼와 같은 크기일 때만 가능하다.
	GBufferDepth,
	// 깊이 버퍼 없이 그린다. 화면 전체 삼각형 같은 후처리용.
	None,
};

// 오프스크린 타깃을 픽셀 셰이더 입력으로 읽을 때의 연결 정보이다.
struct RenderTargetInput
{
	RenderTargetHandle target;
	// HLSL의 register(tN) 번호. 엔진이 쓰는 슬롯(0~5 재질, 10~13 IBL, 15~16 그림자, 20~23 G-Buffer)은 피한다.
	unsigned int shaderSlot = 0;
};

// 한 패스의 입출력 구성이다.
//
// 예전에는 RenderPassType마다 "어디에 쓰고 무엇을 읽는지"가 그래픽스 엔진 안의 switch에 고정되어 있었다.
// 이제는 패스를 등록하는 쪽이 이 구조체로 직접 지정한다. 지정하지 않은 패스는
// GetDefaultRenderPassIO()의 타입별 기본 구성을 쓰고, 기본 구성도 없으면 아무것도 바꾸지 않는다.
struct RenderPassIO
{
	PassColorOutput color = PassColorOutput::Keep;
	// color가 Offscreen일 때만 사용한다.
	RenderTargetHandle offscreenTarget;
	PassDepthOutput depth = PassDepthOutput::Default;

	// true면 G-Buffer(Albedo/Normal/Material/Depth)를 t20~t23에 연결한다.
	// false면 이전 패스가 연결해 둔 G-Buffer 입력을 해제한다.
	bool readGBuffer = false;

	// 이 패스가 읽을 오프스크린 타깃들이다. 다음 번에 입출력을 지정한 패스가 시작될 때 자동으로 해제된다.
	std::vector<RenderTargetInput> inputs;

	// BackBuffer/Offscreen 출력을 묶은 직후에 지울지 여부이다. (GBuffer 출력은 항상 지운다.)
	bool clearColor = false;
	float clearColorValue[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	bool clearDepth = false;
};

// 입출력을 직접 지정하지 않은 패스에 적용할 타입별 기본 구성을 돌려준다.
// 기본 구성이 없는 타입(Ui, Debug, Custom 등)은 false를 반환하며, 그 패스는 직전 상태를 그대로 이어받는다.
inline bool GetDefaultRenderPassIO(RenderPassType passType, RenderPassIO& io)
{
	io = RenderPassIO();
	switch (passType)
	{
	case RenderPassType::Geometry:
		io.color = PassColorOutput::GBuffer;
		return true;
	case RenderPassType::Lighting:
		io.color = PassColorOutput::BackBuffer;
		io.readGBuffer = true;
		return true;
	case RenderPassType::Forward:
		io.color = PassColorOutput::BackBuffer;
		io.depth = PassDepthOutput::GBufferDepth;
		return true;
	default:
		return false;
	}
}

// 그래픽스 API가 허용하지 않는 조합을 실행 가능한 조합으로 고친다.
// 잘못된 구성을 그대로 바인딩하면 DX11이 경고와 함께 일부 바인딩을 조용히 NULL로 만들어서
// "아무것도 안 그려지는" 찾기 어려운 버그가 되므로, 규칙을 한 곳에 모아 둔다.
inline RenderPassIO NormalizeRenderPassIO(RenderPassIO io)
{
	// 핸들이 없으면 그릴 곳을 알 수 없으므로 직전 출력 대상을 유지한다.
	if (io.color == PassColorOutput::Offscreen && !io.offscreenTarget.IsValid())
	{
		io.color = PassColorOutput::Keep;
	}

	// 같은 텍스처를 쓰면서(RTV/DSV) 동시에 읽을(SRV) 수는 없다.
	if (io.color == PassColorOutput::GBuffer)
	{
		io.readGBuffer = false;
		io.depth = PassDepthOutput::Default;
	}

	if (io.readGBuffer)
	{
		// G-Buffer를 입력으로 돌리려면 출력에서 먼저 떼어야 하므로 "직전 출력 유지"가 성립하지 않는다.
		if (io.color == PassColorOutput::Keep)
		{
			io.color = PassColorOutput::BackBuffer;
		}
		// G-Buffer 깊이는 t23으로 읽히는 중이라 깊이 버퍼로 다시 묶을 수 없다.
		if (io.depth == PassDepthOutput::GBufferDepth)
		{
			io.depth = PassDepthOutput::None;
		}
	}

	return io;
}
