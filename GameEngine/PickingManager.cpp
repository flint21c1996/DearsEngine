#include "PickingManager.h"

#include <algorithm>
#include <cfloat>
#include <cmath>

#include "Camera.h"
#include "ModelBuffer.h"
#include "ModelInfo.h"
#include "RenderObject.h"

using DirectX::SimpleMath::Matrix;

namespace
{
constexpr float RayEpsilon = 0.000001f;

bool IsValidAABB(const AABB& aabb)
{
	return aabb.mMin.x <= aabb.mMax.x &&
		aabb.mMin.y <= aabb.mMax.y &&
		aabb.mMin.z <= aabb.mMax.z;
}

Matrix GetObjectWorldMatrix(const RenderObject& object)
{
	return object.ObjectScl * object.ObjectRot * object.ObjectPos;
}

bool GetObjectLocalAABB(RenderObject& object, AABB& outAABB)
{
	ModelBuffer* modelBuffer = object.GetModelBuffer();
	if (!modelBuffer || !modelBuffer->mpTargetModel)
	{
		return false;
	}

	Model* model = modelBuffer->mpTargetModel;
	if (!model->mMeshData || model->mNumMesh == 0)
	{
		return false;
	}

	AABB mergedAABB;
	for (unsigned int meshIndex = 0; meshIndex < model->mNumMesh; ++meshIndex)
	{
		const AABB& meshAABB = model->mMeshData[meshIndex].mAABB;
		if (!IsValidAABB(meshAABB))
		{
			continue;
		}

		mergedAABB.mMin.x = (std::min)(mergedAABB.mMin.x, meshAABB.mMin.x);
		mergedAABB.mMin.y = (std::min)(mergedAABB.mMin.y, meshAABB.mMin.y);
		mergedAABB.mMin.z = (std::min)(mergedAABB.mMin.z, meshAABB.mMin.z);

		mergedAABB.mMax.x = (std::max)(mergedAABB.mMax.x, meshAABB.mMax.x);
		mergedAABB.mMax.y = (std::max)(mergedAABB.mMax.y, meshAABB.mMax.y);
		mergedAABB.mMax.z = (std::max)(mergedAABB.mMax.z, meshAABB.mMax.z);
	}

	if (!IsValidAABB(mergedAABB))
	{
		return false;
	}

	outAABB = mergedAABB;
	return true;
}

bool IntersectRayAABB(const PickingRay& ray, const AABB& aabb, float& distance)
{
	float tMin = 0.0f;
	float tMax = FLT_MAX;

	const float origin[3] = { ray.origin.x, ray.origin.y, ray.origin.z };
	const float direction[3] = { ray.direction.x, ray.direction.y, ray.direction.z };
	const float minValue[3] = { aabb.mMin.x, aabb.mMin.y, aabb.mMin.z };
	const float maxValue[3] = { aabb.mMax.x, aabb.mMax.y, aabb.mMax.z };

	for (int axis = 0; axis < 3; ++axis)
	{
		if (std::fabs(direction[axis]) < RayEpsilon)
		{
			if (origin[axis] < minValue[axis] || origin[axis] > maxValue[axis])
			{
				return false;
			}

			continue;
		}

		const float invDirection = 1.0f / direction[axis];
		float t1 = (minValue[axis] - origin[axis]) * invDirection;
		float t2 = (maxValue[axis] - origin[axis]) * invDirection;

		if (t1 > t2)
		{
			std::swap(t1, t2);
		}

		tMin = (std::max)(tMin, t1);
		tMax = (std::min)(tMax, t2);

		if (tMin > tMax)
		{
			return false;
		}
	}

	distance = tMin;
	return true;
}

AABB BuildWorldAABB(const AABB& localAABB, const Matrix& world)
{
	const Vector3 corners[8] =
	{
		{ localAABB.mMin.x, localAABB.mMin.y, localAABB.mMin.z },
		{ localAABB.mMax.x, localAABB.mMin.y, localAABB.mMin.z },
		{ localAABB.mMin.x, localAABB.mMax.y, localAABB.mMin.z },
		{ localAABB.mMax.x, localAABB.mMax.y, localAABB.mMin.z },
		{ localAABB.mMin.x, localAABB.mMin.y, localAABB.mMax.z },
		{ localAABB.mMax.x, localAABB.mMin.y, localAABB.mMax.z },
		{ localAABB.mMin.x, localAABB.mMax.y, localAABB.mMax.z },
		{ localAABB.mMax.x, localAABB.mMax.y, localAABB.mMax.z },
	};

	AABB worldAABB;
	for (const Vector3& corner : corners)
	{
		const Vector3 transformed = Vector3::Transform(corner, world);
		worldAABB.mMin.x = (std::min)(worldAABB.mMin.x, transformed.x);
		worldAABB.mMin.y = (std::min)(worldAABB.mMin.y, transformed.y);
		worldAABB.mMin.z = (std::min)(worldAABB.mMin.z, transformed.z);

		worldAABB.mMax.x = (std::max)(worldAABB.mMax.x, transformed.x);
		worldAABB.mMax.y = (std::max)(worldAABB.mMax.y, transformed.y);
		worldAABB.mMax.z = (std::max)(worldAABB.mMax.z, transformed.z);
	}

	return worldAABB;
}

PickingRay TransformRay(const PickingRay& ray, const Matrix& transform)
{
	PickingRay result;
	result.origin = Vector3::Transform(ray.origin, transform);
	result.direction = Vector3::TransformNormal(ray.direction, transform);
	result.direction.Normalize();
	return result;
}

bool IntersectRayTriangle(
	const PickingRay& ray,
	const Vector3& v0,
	const Vector3& v1,
	const Vector3& v2,
	float& distance)
{
	const Vector3 edge1 = v1 - v0;
	const Vector3 edge2 = v2 - v0;
	const Vector3 p = ray.direction.Cross(edge2);
	const float determinant = edge1.Dot(p);

	if (std::fabs(determinant) < RayEpsilon)
	{
		return false;
	}

	const float invDeterminant = 1.0f / determinant;
	const Vector3 t = ray.origin - v0;
	const float u = t.Dot(p) * invDeterminant;

	if (u < 0.0f || u > 1.0f)
	{
		return false;
	}

	const Vector3 q = t.Cross(edge1);
	const float v = ray.direction.Dot(q) * invDeterminant;

	if (v < 0.0f || u + v > 1.0f)
	{
		return false;
	}

	const float hitDistance = edge2.Dot(q) * invDeterminant;
	if (hitDistance < 0.0f)
	{
		return false;
	}

	distance = hitDistance;
	return true;
}
}

PickingResult PickingManager::Pick(
	const std::vector<std::unique_ptr<RenderObject>>& objects,
	const Vector2& mousePosition,
	int viewportWidth,
	int viewportHeight,
	Camera* camera)
{
	m_lastResult = {};

	if (!camera || viewportWidth <= 0 || viewportHeight <= 0)
	{
		return m_lastResult;
	}

	if (mousePosition.x < 0.0f || mousePosition.y < 0.0f ||
		mousePosition.x > static_cast<float>(viewportWidth) ||
		mousePosition.y > static_cast<float>(viewportHeight))
	{
		return m_lastResult;
	}

	const PickingRay worldRay = BuildRayFromScreen(mousePosition, viewportWidth, viewportHeight, camera);

	float closestDistance = FLT_MAX;
	int closestIndex = -1;

	for (int objectIndex = 0; objectIndex < static_cast<int>(objects.size()); ++objectIndex)
	{
		RenderObject* object = objects[objectIndex].get();
		if (!object)
		{
			continue;
		}

		float distance = 0.0f;
		bool hit = false;

		switch (m_mode)
		{
		case PickingMode::OBB:
			hit = PickObjectOBB(worldRay, *object, distance);
			break;
		case PickingMode::Triangle:
			hit = PickObjectTriangle(worldRay, *object, distance);
			break;
		case PickingMode::AABB:
		default:
			hit = PickObjectAABB(worldRay, *object, distance);
			break;
		}

		if (hit && distance < closestDistance)
		{
			closestDistance = distance;
			closestIndex = objectIndex;
		}
	}

	if (closestIndex >= 0)
	{
		m_lastResult.hit = true;
		m_lastResult.objectIndex = closestIndex;
		m_lastResult.distance = closestDistance;
	}

	return m_lastResult;
}

PickingRay PickingManager::BuildRayFromScreen(
	const Vector2& mousePosition,
	int viewportWidth,
	int viewportHeight,
	Camera* camera) const
{
	const Matrix projection = camera->GetProjRow();
	const float ndcX = (2.0f * mousePosition.x / static_cast<float>(viewportWidth)) - 1.0f;
	const float ndcY = 1.0f - (2.0f * mousePosition.y / static_cast<float>(viewportHeight));

	// 카메라가 실제로 들고 있는 현재 basis 벡터로 ray를 만든다.
	// view/projection 행렬을 역변환하는 방식은 행렬 전치/row-major 규칙이 조금만 어긋나도
	// 렌더 화면과 CPU 피킹 ray가 달라질 수 있다.
	//
	// 여기서는 projection의 _11/_22로 view space의 x/y 기울기를 구하고,
	// 그 값을 현재 카메라의 right/up/forward 축에 직접 섞는다.
	// 그래서 RMB로 카메라를 회전/이동한 뒤에도 ray가 현재 카메라 방향을 그대로 따른다.
	Vector3 rayDirection =
		camera->mRightDir * (ndcX / projection._11) +
		camera->mViewUp * (ndcY / projection._22) +
		camera->mViewDir;

	PickingRay ray;
	ray.origin = camera->GetmViewPos();
	ray.direction = rayDirection;
	ray.direction.Normalize();
	return ray;
}

bool PickingManager::PickObjectAABB(const PickingRay& worldRay, RenderObject& object, float& distance) const
{
	AABB localAABB;
	if (!GetObjectLocalAABB(object, localAABB))
	{
		return false;
	}

	// AABB 모드는 오브젝트의 로컬 박스를 월드 공간으로 펼친 뒤,
	// 다시 월드 축에 맞는 큰 박스로 감싼다.
	// 회전된 물체는 실제보다 박스가 커질 수 있지만, 빠른 1차 선택에는 유용하다.
	const AABB worldAABB = BuildWorldAABB(localAABB, GetObjectWorldMatrix(object));
	return IntersectRayAABB(worldRay, worldAABB, distance);
}

bool PickingManager::PickObjectOBB(const PickingRay& worldRay, RenderObject& object, float& distance) const
{
	AABB localAABB;
	if (!GetObjectLocalAABB(object, localAABB))
	{
		return false;
	}

	// OBB 검사는 박스를 월드에서 회전시키는 대신 ray를 오브젝트 로컬 공간으로 옮긴다.
	// 이렇게 하면 회전된 박스도 로컬에서는 평범한 AABB라서 같은 slab 검사를 재사용할 수 있다.
	Matrix inverseWorld = GetObjectWorldMatrix(object);
	inverseWorld.Invert();

	const PickingRay localRay = TransformRay(worldRay, inverseWorld);
	float localDistance = 0.0f;
	if (!IntersectRayAABB(localRay, localAABB, localDistance))
	{
		return false;
	}

	const Vector3 localHit = localRay.origin + localRay.direction * localDistance;
	const Vector3 worldHit = Vector3::Transform(localHit, GetObjectWorldMatrix(object));
	distance = Vector3::Distance(worldRay.origin, worldHit);
	return true;
}

bool PickingManager::PickObjectTriangle(const PickingRay& worldRay, RenderObject& object, float& distance) const
{
	ModelBuffer* modelBuffer = object.GetModelBuffer();
	if (!modelBuffer || !modelBuffer->mpTargetModel)
	{
		return false;
	}

	Matrix inverseWorld = GetObjectWorldMatrix(object);
	inverseWorld.Invert();

	const PickingRay localRay = TransformRay(worldRay, inverseWorld);
	Model* model = modelBuffer->mpTargetModel;

	float closestLocalDistance = FLT_MAX;
	bool hitAnyTriangle = false;

	for (unsigned int meshIndex = 0; meshIndex < model->mNumMesh; ++meshIndex)
	{
		const Mesh& mesh = model->mMeshData[meshIndex];
		if (!mesh.mVertices || !mesh.mIndices || mesh.mNumIndices < 3)
		{
			continue;
		}

		for (unsigned int index = 0; index + 2 < mesh.mNumIndices; index += 3)
		{
			const Vector3& v0 = mesh.mVertices[mesh.mIndices[index]].mPosition;
			const Vector3& v1 = mesh.mVertices[mesh.mIndices[index + 1]].mPosition;
			const Vector3& v2 = mesh.mVertices[mesh.mIndices[index + 2]].mPosition;

			float triangleDistance = 0.0f;
			if (IntersectRayTriangle(localRay, v0, v1, v2, triangleDistance) &&
				triangleDistance < closestLocalDistance)
			{
				closestLocalDistance = triangleDistance;
				hitAnyTriangle = true;
			}
		}
	}

	if (!hitAnyTriangle)
	{
		return false;
	}

	// Triangle 피킹도 로컬 공간에서 맞은 지점을 다시 월드로 옮겨서
	// 다른 오브젝트와 같은 기준의 거리로 비교한다.
	const Vector3 localHit = localRay.origin + localRay.direction * closestLocalDistance;
	const Vector3 worldHit = Vector3::Transform(localHit, GetObjectWorldMatrix(object));
	distance = Vector3::Distance(worldRay.origin, worldHit);
	return true;
}

