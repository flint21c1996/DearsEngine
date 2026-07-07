#include "RenderDispatcher.h"

#include "DearsGraphicsEngine.h"
#include "RenderObject.h"

RenderDispatcher::RenderDispatcher(DearsGraphicsEngine* graphicsEngine)
	: m_pGraphicsEngine(graphicsEngine)
{
}

void RenderDispatcher::RenderShadowItem(const SceneRenderItem& item)
{
	if (!m_pGraphicsEngine || !item.object)
	{
		return;
	}

	// Shadow pass에서는 오브젝트의 "역할 이름"보다
	// 어떤 셰이더 경로로 depth를 그려야 하는지가 중요하다.
	// StaticMesh는 일반 depth, SkinnedMesh는 bone buffer가 필요한 depth,
	// EquipmentMesh는 target bone matrix가 필요한 depth 경로를 탄다.
	//
	// 지금은 여기서 DX11용 DearsGraphicsEngine 함수를 직접 부르지만,
	// RHI를 도입하면 이 클래스가 "공통 shadow draw command"를 만들거나
	// 선택된 백엔드에 draw 요청을 전달하는 위치로 바뀔 가능성이 높다.
	switch (item.renderType)
	{
	case SceneRenderType::SkinnedMesh:
		m_pGraphicsEngine->RenderAniDepthMap(item.object->GetModelBuffer());
		break;
	case SceneRenderType::EquipmentMesh:
		m_pGraphicsEngine->RenderEquipDepthMap(item.object->GetModelBuffer());
		break;
	default:
		m_pGraphicsEngine->RenderDepthMap(item.object->GetModelBuffer());
		break;
	}
}

void RenderDispatcher::RenderMainItem(const SceneRenderItem& item)
{
	if (!m_pGraphicsEngine || !item.object)
	{
		return;
	}

	// Main pass에서는 씬이 넘긴 renderType에 따라 현재 DX11 렌더 함수를 고른다.
	// 이 switch는 지금 당장은 "렌더 타입 -> DX11 렌더 함수" 매핑이지만,
	// 나중에는 "렌더 타입 -> RHI pipeline / draw command" 매핑으로 옮겨갈 부분이다.
	switch (item.renderType)
	{
	case SceneRenderType::CubeMap:
		m_pGraphicsEngine->Rend_CubeMap(item.object->GetModelBuffer());
		break;
	case SceneRenderType::Billboard:
		m_pGraphicsEngine->Rend_BillBoard(item.object->GetModelBuffer());
		break;
	case SceneRenderType::PbrMesh:
		m_pGraphicsEngine->Rend_PBR(item.object->GetModelBuffer());
		break;
	case SceneRenderType::SkinnedMesh:
		m_pGraphicsEngine->Rend_AnimateModel(item.object->GetModelBuffer());
		break;
	case SceneRenderType::EquipmentMesh:
		m_pGraphicsEngine->Rend_EquipmentModel(item.object->GetModelBuffer());
		break;
	default:
		m_pGraphicsEngine->Rend_Model(item.object->GetModelBuffer());
		break;
	}
}
