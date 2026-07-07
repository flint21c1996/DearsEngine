#pragma once

#include "IScene.h"

class DearsGraphicsEngine;

class RenderDispatcher
{
public:
	explicit RenderDispatcher(DearsGraphicsEngine* graphicsEngine);

	void RenderShadowItem(const SceneRenderItem& item);
	void RenderMainItem(const SceneRenderItem& item);

private:
	DearsGraphicsEngine* m_pGraphicsEngine = nullptr;
};
