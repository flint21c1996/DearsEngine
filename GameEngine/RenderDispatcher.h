#pragma once

#include "IScene.h"

class DearsGraphicsEngine;
class Camera;
class RenderObject;

// SceneRenderItem 목록을 받아서 현재 그래픽스 엔진의 실제 렌더 함수로 보내는 중간 계층이다.
//
// GameEngine은 "어떤 렌더 패스를 실행할지"와 "어떤 씬 목록을 처리할지"까지만 알고,
// RenderDispatcher는 각 SceneRenderType이 어떤 렌더 경로로 이어지는지 판단한다.
//
// 지금은 DearsGraphicsEngine의 DX11 기반 함수들을 직접 호출하지만,
// RHI가 들어오면 이 클래스가 DX11/DX12/Vulkan 공통 draw command를 만들거나
// 선택된 백엔드로 렌더 요청을 넘기는 위치가 될 수 있다.
class RenderDispatcher
{
public:
	explicit RenderDispatcher(DearsGraphicsEngine* graphicsEngine);

	void RenderShadowItems(const std::vector<SceneRenderItem>& items);
	void RenderForwardItems(const std::vector<SceneRenderItem>& items);
	void RenderShadowItem(const SceneRenderItem& item);
	void RenderForwardItem(const SceneRenderItem& item);
	void RenderSelectedOutline(RenderObject* object, Camera* camera);

private:
	DearsGraphicsEngine* m_pGraphicsEngine = nullptr;
};
