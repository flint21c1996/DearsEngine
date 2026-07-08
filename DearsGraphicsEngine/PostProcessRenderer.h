#pragma once

#include <memory>

#include "ModelBuffer.h"

class DearsGraphicsEngine;

// 후처리(Post Processing) 렌더링을 담당하는 클래스이다.
//
// 후처리는 일반 메시 렌더링과 다르게, 이미 그려진 화면 텍스처를 다시 샘플링해서
// 최종 화면에 효과를 입히는 단계이다.
//
// 지금은 DearsGraphicsEngine과 DX11 Renderer를 그대로 사용하지만,
// 후처리용 화면 사각형 버퍼와 렌더 순서를 이 클래스로 모아두면
// 나중에 Bloom, Blur, Tone Mapping 같은 패스를 추가하기 쉬워진다.
class PostProcessRenderer
{
public:
	explicit PostProcessRenderer(DearsGraphicsEngine* graphicsEngine);

	void Initialize();
	void Render();

private:
	DearsGraphicsEngine* m_pGraphicsEngine = nullptr;

	// 후처리는 화면 전체를 덮는 사각형을 그리며 진행한다.
	// 현재는 BillBoardSquare 메시를 화면 사각형처럼 재사용한다.
	std::unique_ptr<ModelBuffer> m_pPostProcessingBuffer;
};
