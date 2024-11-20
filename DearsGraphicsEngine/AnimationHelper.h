#pragma once
#include "GraphicsResourceContainer.h"
#include<unordered_map>
 

class AnimationHelper
{
public:
	AnimationHelper();
	~AnimationHelper();

public:
	Vector3 Lerp(const Vector3& start, const Vector3& end, double t);

	Quaternion Slerp(const Quaternion& start, const Quaternion& end, double t);

	double CalculateFactor(double startTime, double endTime, double currentTime);

	template<typename TKey>
	std::pair<TKey, TKey> FindAdjacentKeys(TKey* keys, unsigned int keyCount, double currentTime);

	void UpdateBoneConstant(Model* _targetModel, Animation* _targetAnimation, VSBoneConstantBufferData& _vsBoneConstantBuffer, double &_time);
	bool UpdateBoneConstant(Model* _targetModel, Animation* _targetAnimation, Animation* _nextTargetAnimation ,VSBoneConstantBufferData& _vsBoneConstantBuffer, double& _time, double& _nextTIme);

	void CalculateTransforms(Node* _node, const Animation* _animationData, double _animationTimeInSeconds);
	void CalculateTransforms(Node* _node, const Animation* _animationData, const Animation* _nextAnimationData, double _animationTimeInSeconds, double _nextAnimationTimeInSeconds);

};

template<typename TKey>
std::pair<TKey, TKey> AnimationHelper::FindAdjacentKeys(TKey* keys, unsigned int keyCount, double currentTime)
{
	if (keyCount == 1) {
		// 키가 하나만 있는 경우, 해당 키를 이전과 다음 키로 사용
		return { keys[0], keys[0] };
	}

	TKey prevKey = keys[0];
	TKey nextKey = keys[1];

	for (unsigned int i = 0; i < keyCount - 1; ++i) {
		if (keys[i].mTime <= currentTime && keys[i + 1].mTime > currentTime) {
			prevKey = keys[i];
			nextKey = keys[i + 1];
			return { prevKey, nextKey };
		}
	}

	// 애니메이션이 루프하는 경우, 현재 시간이 마지막 키의 시간보다 크거나 마지막 키와 정확히 일치할 경우,
	// 마지막 키를 이전 키로, 첫 번째 키를 다음 키로 설정하여 애니메이션의 루프가 자연스럽게 이루어지도록 한다.
	if (currentTime >= keys[keyCount - 1].mTime || keys[0].mTime == currentTime) {
		prevKey = keys[keyCount-1];
		nextKey = keys[0];
	}

	return { prevKey, nextKey };
}