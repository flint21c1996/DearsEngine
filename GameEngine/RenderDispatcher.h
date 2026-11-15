#pragma once

#include "IScene.h"

class IDearsRenderer;
class Camera;
class RenderObject;

// SceneRenderItem 목록을 받아서 현재 그래픽스 엔진의 실제 렌더 함수로 보내는 중간 계층이다.
//
// GameEngine은 "어떤 렌더 패스를 실행할지"와 "어떤 씬 목록을 처리할지"까지만 알고,
// RenderDispatcher는 각 SceneRenderType이 어떤 렌더 경로로 이어지는지 판단한다.
//
// DX11 구현 클래스가 아니라 IDearsRenderer 인터페이스에만 의존한다.
// SceneRenderType을 MeshDrawMode/ShadowCasterMode로 바꿔서 넘기기만 하므로,
// 백엔드가 바뀌거나 테스트용 가짜 렌더러를 넣어도 이 클래스는 그대로 쓸 수 있다.
class RenderDispatcher
{
public:
	explicit RenderDispatcher(IDearsRenderer* renderer);

	void RenderShadowItems(const std::vector<SceneRenderItem>& items);
	void RenderGeometryItems(const std::vector<SceneRenderItem>& items);
	void RenderForwardItems(const std::vector<SceneRenderItem>& items);
	void RenderShadowItem(const SceneRenderItem& item);
	void RenderGeometryItem(const SceneRenderItem& item);
	void RenderForwardItem(const SceneRenderItem& item);
	void RenderSelectedOutline(RenderObject* object, Camera* camera);

private:
	IDearsRenderer* m_pRenderer = nullptr;
};
