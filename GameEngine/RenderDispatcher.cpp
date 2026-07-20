#include "RenderDispatcher.h"

#include "Camera.h"
#include "DearsGraphicsEngine.h"
#include "ModelBuffer.h"
#include "RenderObject.h"

namespace
{
bool IsDrawableModelBuffer(ModelBuffer* modelBuffer)
{
	return modelBuffer &&
		modelBuffer->m_pVertexBuffer &&
		modelBuffer->m_pIndexBuffer &&
		modelBuffer->mNumIndices > 0;
}

Vector3 GetObjectWorldPosition(const RenderObject& object)
{
	// SimpleMath의 translation matrix는 _41/_42/_43에 이동값을 둔다.
	// 외곽선 두께를 카메라 거리 기준으로 조절하기 위해 오브젝트의 월드 위치를 읽는다.
	return Vector3(
		object.ObjectPos._41,
		object.ObjectPos._42,
		object.ObjectPos._43);
}

float CalculateDistanceBasedOutlineScale(const RenderObject& object, Camera* camera)
{
	if (!camera)
	{
		return 1.03f;
	}

	const float distance = Vector3::Distance(camera->GetmViewPos(), GetObjectWorldPosition(object));

	// 현재 외곽선은 화면 픽셀 단위가 아니라 모델을 몇 배 키울지로 결정된다.
	// 멀리 있는 물체는 같은 1.03배라도 화면에서는 1픽셀 미만으로 줄어들 수 있으므로,
	// 카메라에서 멀수록 확대 비율을 조금씩 키운다.
	const float outlineAmount = (std::max)(0.04f, (std::min)(distance * 0.002f, 0.35f));
	return 1.0f + outlineAmount;
}

bool IsDeferredDefaultLitPbr(const SceneRenderItem& item)
{
	// 현재 Deferred Lighting Shader가 해석할 수 있는 G-Buffer 형식은
	// Default Lit PBR Material로 한정되어 있다. Geometry와 Forward 필터가
	// 서로 다른 조건을 사용하면 오브젝트가 두 번 그려지거나 아예 사라질 수 있으므로
	// 하나의 판정 함수를 두 패스에서 함께 사용한다.
	return item.renderPath == SceneRenderPath::Deferred &&
		item.renderType == SceneRenderType::PbrMesh &&
		item.object &&
		item.object->GetShadingModel() == MaterialShadingModel::DefaultLit;
}
}

RenderDispatcher::RenderDispatcher(DearsGraphicsEngine* graphicsEngine)
	: m_pGraphicsEngine(graphicsEngine)
{
}

void RenderDispatcher::RenderShadowItems(const std::vector<SceneRenderItem>& items)
{
	// 목록 순회는 디스패처가 맡는다.
	// GameEngine은 "shadow pass에서 이 목록을 처리해줘"라고 요청만 하고,
	// 각 item이 어떤 렌더 함수로 이어지는지는 이 클래스 안에 모아둔다.
	for (const SceneRenderItem& item : items)
	{
		RenderShadowItem(item);
	}
}

void RenderDispatcher::RenderGeometryItems(const std::vector<SceneRenderItem>& items)
{
	// 첫 디퍼드 대상은 표면 데이터가 이미 준비된 PBR 메시로 제한한다.
	for (const SceneRenderItem& item : items)
	{
		if (IsDeferredDefaultLitPbr(item))
		{
			RenderGeometryItem(item);
		}
	}
}

void RenderDispatcher::RenderForwardItems(const std::vector<SceneRenderItem>& items)
{
	// Geometry + Lighting Pass에서 이미 최종 색상을 만든 Deferred PBR은 제외한다.
	// 같은 오브젝트를 여기서 다시 그리면 Deferred 결과가 Forward 결과로 덮여서
	// G-Buffer 조명이 잘못되어도 정상처럼 보이는 문제가 생긴다.
	for (const SceneRenderItem& item : items)
	{
		if (IsDeferredDefaultLitPbr(item))
		{
			continue;
		}
		RenderForwardItem(item);
	}
}

void RenderDispatcher::RenderShadowItem(const SceneRenderItem& item)
{
	if (!m_pGraphicsEngine || !item.object)
	{
		return;
	}

	ModelBuffer* modelBuffer = item.object->GetModelBuffer();
	if (!IsDrawableModelBuffer(modelBuffer))
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
	case SceneRenderType::PbrMesh:
		// PBR 메시는 Height Map이 실루엣을 바꿀 수 있으므로 전용 Shadow VS가 필요하다.
		// 원본 정점만 그리면 화면의 돌출된 표면과 그림자 모양이 서로 달라진다.
		m_pGraphicsEngine->RenderPbrDepthMap(modelBuffer);
		break;
	case SceneRenderType::SkinnedMesh:
		m_pGraphicsEngine->RenderAniDepthMap(modelBuffer);
		break;
	case SceneRenderType::EquipmentMesh:
		m_pGraphicsEngine->RenderEquipDepthMap(modelBuffer);
		break;
	default:
		m_pGraphicsEngine->RenderDepthMap(modelBuffer);
		break;
	}
}

void RenderDispatcher::RenderGeometryItem(const SceneRenderItem& item)
{
	if (!m_pGraphicsEngine || !item.object)
	{
		return;
	}

	ModelBuffer* modelBuffer = item.object->GetModelBuffer();
	if (!IsDrawableModelBuffer(modelBuffer))
	{
		return;
	}

	// 최종 조명 색상 대신 Albedo/Normal/Material G-Buffer만 기록한다.
	// 이 항목의 최종 색상은 뒤의 Fullscreen Deferred Lighting Pass에서 만들어진다.
	m_pGraphicsEngine->Rend_DeferredGeometry(modelBuffer);
}

void RenderDispatcher::RenderForwardItem(const SceneRenderItem& item)
{
	if (!m_pGraphicsEngine || !item.object)
	{
		return;
	}

	ModelBuffer* modelBuffer = item.object->GetModelBuffer();
	if (!IsDrawableModelBuffer(modelBuffer))
	{
		return;
	}

	// Forward pass에서는 씬이 넘긴 renderType에 따라 현재 DX11 렌더 함수를 고른다.
	// 이 switch는 지금 당장은 "렌더 타입 -> DX11 렌더 함수" 매핑이지만,
	// 나중에는 "렌더 타입 -> RHI pipeline / draw command" 매핑으로 옮겨갈 부분이다.
	switch (item.renderType)
	{
	case SceneRenderType::CubeMap:
		m_pGraphicsEngine->Rend_CubeMap(modelBuffer);
		break;
	case SceneRenderType::Billboard:
		m_pGraphicsEngine->Rend_BillBoard(modelBuffer);
		break;
	case SceneRenderType::PbrMesh:
		// Thin Film은 별도의 프레임 Pass가 아니라 Forward Pass 안에서 선택하는
		// PBR 계열 Shading Model이다. 메시/텍스처는 그대로 사용하고 PSO의
		// Pixel Shader와 b3 상수 버퍼만 Thin Film 경로로 교체한다.
		if (item.object->GetShadingModel() == MaterialShadingModel::ThinFilm)
		{
			m_pGraphicsEngine->Rend_ThinFilm(modelBuffer);
		}
		else
		{
			m_pGraphicsEngine->Rend_PBR(modelBuffer);
		}
		break;
	case SceneRenderType::SkinnedMesh:
		m_pGraphicsEngine->Rend_AnimateModel(modelBuffer);
		break;
	case SceneRenderType::EquipmentMesh:
		m_pGraphicsEngine->Rend_EquipmentModel(modelBuffer);
		break;
	default:
		m_pGraphicsEngine->Rend_Model(modelBuffer);
		break;
	}
}

void RenderDispatcher::RenderSelectedOutline(RenderObject* object, Camera* camera)
{
	if (!m_pGraphicsEngine || !object)
	{
		return;
	}

	ModelBuffer* modelBuffer = object->GetModelBuffer();
	if (!IsDrawableModelBuffer(modelBuffer))
	{
		return;
	}

	// 피킹된 오브젝트는 기존 외곽선 셰이더를 한 번 더 통과시켜 강조한다.
	// 선택 상태는 DemoScene이 들고 있지만, "어떤 렌더 함수로 강조할지"는
	// 렌더 디스패처가 알고 있는 편이 GameEngine을 단순하게 유지한다.
	object->ConfigureOutline(CalculateDistanceBasedOutlineScale(*object, camera), Vector3(1.0f, 0.45f, 0.0f));
	m_pGraphicsEngine->Rend_EdgeModel(modelBuffer);
}
