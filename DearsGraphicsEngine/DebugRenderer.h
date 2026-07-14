#pragma once

#include <directxtk/SimpleMath.h>
#include <memory>
#include <string>
#include <vector>

#include "ModelBuffer.h"
#include "ModelInfo.h"

class DearsGraphicsEngine;
struct Light;

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;

// 디버그용 박스, 구, 캡슐 같은 단순 형상을 렌더링하는 클래스이다.
//
// 기존에는 DearsGraphicsEngine이 디버그 모델 버퍼까지 직접 들고 있었기 때문에
// "그래픽스 엔진 전체 관리"와 "디버그 도형 그리기" 책임이 한 클래스 안에 섞여 있었다.
//
// 지금은 DX11 기반 DearsGraphicsEngine 함수를 그대로 사용하지만,
// 나중에 RHI를 붙일 때는 이 클래스가 "디버그 렌더 요청을 어떤 렌더 백엔드로 보낼지"를
// 분리하는 출발점이 된다.
class DebugRenderer
{
public:
	explicit DebugRenderer(DearsGraphicsEngine* graphicsEngine);

	void Initialize();

	void RenderBox(Vector3 size, Vector3 rotation, Vector3 translation);
	void RenderBox(Matrix size, Matrix rotation, Matrix translation);
	void RenderBox(Matrix size, Matrix rotation, Matrix translation, Matrix extraTransform);
	void RenderBox(AABB& aabb, Matrix scale, Matrix rotation, Matrix extraTransform = Matrix());

	void RenderSphere(Vector3 size, Vector3 rotation, Vector3 translation);
	void RenderCapsule(Vector3 size, Vector3 rotation, Vector3 translation);
	void RenderLightGizmo(
		const Light& light,
		bool drawShadowFrustum,
		float shadowAspect);

private:
	void PrepareDebugModel(const std::string& modelName);
	void AddLine(Vector3 start, Vector3 end, Vector4 color);
	void AddCircle(Vector3 center, Vector3 axisX, Vector3 axisY, float radius, Vector4 color, int segments = 32);
	void AddPerspectiveFrustum(
		Vector3 position,
		Vector3 direction,
		Vector3 right,
		Vector3 up,
		float fovYDegrees,
		float aspect,
		float nearDistance,
		float farDistance,
		Vector4 color);
	void AddOrthographicFrustum(
		Vector3 position,
		Vector3 direction,
		Vector3 right,
		Vector3 up,
		float width,
		float height,
		float nearDistance,
		float farDistance,
		Vector4 color);
	void FlushLines();

private:
	// 아직은 기존 DearsGraphicsEngine의 DX11 함수들을 빌려 쓴다.
	// 다음 단계에서 RHI가 들어오면 이 의존성을 더 작은 인터페이스로 줄일 수 있다.
	DearsGraphicsEngine* m_pGraphicsEngine = nullptr;

	// 디버그 도형은 매번 같은 ModelBuffer를 재사용한다.
	// 그릴 때마다 vertex/index buffer와 world matrix만 바꿔 끼우면 된다.
	std::unique_ptr<ModelBuffer> m_pDebugModelBuffer;

	struct LineVertex
	{
		Vector3 position;
		Vector4 color;
	};
	std::vector<LineVertex> m_lineVertices;
	ComPtr<ID3D11Buffer> m_pLineVertexBuffer;
	ComPtr<ID3D11VertexShader> m_pLineVertexShader;
	ComPtr<ID3D11PixelShader> m_pLinePixelShader;
	ComPtr<ID3D11InputLayout> m_pLineInputLayout;
};
