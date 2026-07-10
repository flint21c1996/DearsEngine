#pragma once

#include <memory>
#include <vector>

#include <directxtk/SimpleMath.h>

class Camera;
class RenderObject;

using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

// 에디터에서 사용할 피킹 검사 방식이다.
//
// AABB:
// - 오브젝트를 월드 축에 정렬된 박스로 감싸고 검사한다.
// - 가장 빠르고 구현이 단순하지만, 오브젝트가 회전하면 박스가 커져서 빈 공간도 선택될 수 있다.
//
// OBB:
// - 마우스 ray를 오브젝트 로컬 공간으로 옮긴 뒤, 로컬 AABB와 검사한다.
// - 오브젝트 회전을 반영한 박스 피킹처럼 동작하므로 AABB보다 정확하다.
//
// Triangle:
// - ray와 실제 메시 삼각형을 검사한다.
// - 정적인 메시에는 가장 정확하지만, 현재 구현은 스키닝 후 변형된 정점까지는 반영하지 않는다.
enum class PickingMode
{
	AABB,
	OBB,
	Triangle,
};

struct PickingRay
{
	Vector3 origin = Vector3::Zero;
	Vector3 direction = Vector3::UnitZ;
};

struct PickingResult
{
	bool hit = false;
	int objectIndex = -1;
	float distance = 0.0f;
};

class PickingManager
{
public:
	PickingMode GetMode() const { return m_mode; }
	void SetMode(PickingMode mode) { m_mode = mode; }
	const PickingResult& GetLastResult() const { return m_lastResult; }

	PickingResult Pick(
		const std::vector<std::unique_ptr<RenderObject>>& objects,
		const Vector2& mousePosition,
		int viewportWidth,
		int viewportHeight,
		Camera* camera);

private:
	PickingRay BuildRayFromScreen(
		const Vector2& mousePosition,
		int viewportWidth,
		int viewportHeight,
		Camera* camera) const;

	bool PickObjectAABB(const PickingRay& worldRay, RenderObject& object, float& distance) const;
	bool PickObjectOBB(const PickingRay& worldRay, RenderObject& object, float& distance) const;
	bool PickObjectTriangle(const PickingRay& worldRay, RenderObject& object, float& distance) const;

	PickingMode m_mode = PickingMode::AABB;
	PickingResult m_lastResult;
};
