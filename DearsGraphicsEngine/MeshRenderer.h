#pragma once

#include <string>
#include <unordered_map>

#include "PipelineStateObject.h"

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
	void RenderDeferredGeometry(ModelBuffer* modelBuffer);
	void RenderThinFilmModel(ModelBuffer* modelBuffer);
	void RenderEquipmentModel(ModelBuffer* modelBuffer);
	void RenderOpacityModel(ModelBuffer* modelBuffer);
	void RenderEdgeModel(ModelBuffer* modelBuffer);
	void RenderWater(ModelBuffer* modelBuffer);

	void RenderDepthMap(ModelBuffer* modelBuffer);
	void RenderPbrDepthMap(ModelBuffer* modelBuffer);
	void RenderAnimatedDepthMap(ModelBuffer* modelBuffer);
	void RenderEquipmentDepthMap(ModelBuffer* modelBuffer);

	void RenderInstancedModels(ModelBuffer* modelBuffer);
	void RenderBillboard(ModelBuffer* modelBuffer);
	void RenderCubeMap(ModelBuffer* modelBuffer);

	void SetOpacityFactor(float blendFactor[4]);

	// ShaderCatalog의 이름으로 Vertex/Pixel Shader를 골라서 그린다.
	// 에디터에서 오브젝트마다 셰이더 조합을 바꿀 수 있게 하는 경로이며,
	// 이름이 없거나 두 셰이더의 계열이 달라 조합할 수 없으면 그리지 않고 false를 돌려준다.
	bool RenderWithShaders(
		ModelBuffer* modelBuffer,
		const std::string& vertexShader,
		const std::string& pixelShader);

private:
	void EnsureDeferredGeometryPipeline();
	// 이름으로 고른 조합의 PSO를 만든다. 실패하면 false를 돌려준다.
	bool BuildShaderPipeline(
		const std::string& vertexShader,
		const std::string& pixelShader,
		PipelineStateObject& pipeline);

	// 한 번 만든 조합은 "VertexShader|PixelShader" 키로 보관해 매 프레임 다시 만들지 않는다.
	std::unordered_map<std::string, PipelineStateObject> m_shaderPipelines;

	// 아직은 기존 DearsGraphicsEngine을 통해 PSO와 Renderer에 접근한다.
	// 다음 단계에서는 이 의존성을 IRHI 또는 RenderCommandList 같은 인터페이스로 줄이는 것이 목표다.
	DearsGraphicsEngine* m_pGraphicsEngine = nullptr;
	PipelineStateObject m_deferredGeometryPSO;
	bool m_isDeferredGeometryPipelineInitialized = false;
	// Height Map 변형을 Shadow Pass에도 적용하는 PBR 전용 Depth Pipeline이다.
	// 일반 StaticMesh용 Shadow Pipeline과 분리해 b6/t0 리소스가 다른 타입으로 새는 것을 막는다.
	PipelineStateObject m_pbrShadowPSO;
	bool m_isPbrShadowPipelineInitialized = false;
};
