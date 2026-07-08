#pragma once

class DearsGraphicsEngine;
struct ModelBuffer;

// 일반 메시 렌더링 경로를 담당하는 클래스이다.
//
// DearsGraphicsEngine은 엔진 초기화, 리소스 접근, 렌더러 소유 같은 큰 책임을 맡고,
// MeshRenderer는 "어떤 종류의 메시를 어떤 PSO로 그릴지"를 담당한다.
//
// 지금은 DX11 기반 Renderer를 직접 호출하지만,
// RHI가 들어오면 이 클래스의 함수들이 백엔드 독립적인 draw command를 만드는 위치로
// 자연스럽게 이동할 수 있다.
class MeshRenderer
{
public:
	explicit MeshRenderer(DearsGraphicsEngine* graphicsEngine);

	void RenderAnimatedModel(ModelBuffer* modelBuffer);
	void RenderStaticModel(ModelBuffer* modelBuffer);
	void RenderPbrModel(ModelBuffer* modelBuffer);
	void RenderThinFilmModel(ModelBuffer* modelBuffer);
	void RenderEquipmentModel(ModelBuffer* modelBuffer);
	void RenderOpacityModel(ModelBuffer* modelBuffer);
	void RenderEdgeModel(ModelBuffer* modelBuffer);
	void RenderWater(ModelBuffer* modelBuffer);

	void RenderDepthMap(ModelBuffer* modelBuffer);
	void RenderAnimatedDepthMap(ModelBuffer* modelBuffer);
	void RenderEquipmentDepthMap(ModelBuffer* modelBuffer);

	void RenderInstancedModels(ModelBuffer* modelBuffer);
	void RenderBillboard(ModelBuffer* modelBuffer);
	void RenderCubeMap(ModelBuffer* modelBuffer);

	void SetOpacityFactor(float blendFactor[4]);

private:
	// 아직은 기존 DearsGraphicsEngine을 통해 PSO와 Renderer에 접근한다.
	// 다음 단계에서는 이 의존성을 IRHI 또는 RenderCommandList 같은 인터페이스로 줄이는 것이 목표다.
	DearsGraphicsEngine* m_pGraphicsEngine = nullptr;
};
