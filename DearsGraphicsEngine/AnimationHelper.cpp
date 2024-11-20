#include <queue>
#include "AnimationHelper.h"

#ifdef _DEBUG
#include <iostream>
#define DEBUG_LOG(message) std::cout<<message<<std::endl
#endif // DEBUG

AnimationHelper::AnimationHelper()
{

}

AnimationHelper::~AnimationHelper()
{

}

///선형보간, 두 점 사이의 직선 경로를 따라 보간, 벡터나 스칼라에서 사용
Vector3 AnimationHelper::Lerp(const Vector3& start, const Vector3& end, double t)
{
	return start + (end - start) * t;
}

///구면 선형보간, 두 점사이의 구면 경로를 따라 보간, 주로 쿼터니언에서 사용
Quaternion AnimationHelper::Slerp(const Quaternion& start, const Quaternion& end, double t)
{
	return Quaternion::Slerp(start, end, t);
}

///애니메이션의 시간 비율을 계산
double AnimationHelper::CalculateFactor(double startTime, double endTime, double currentTime)
{
	if (endTime - startTime == 0)
	{
		return 0;
	}
	return (currentTime - startTime) / (endTime - startTime);
}

void AnimationHelper::UpdateBoneConstant(Model* _targetModel, Animation* _targetAnimation, VSBoneConstantBufferData& _vsBoneConstantBuffer, double& _time)
{
	if (!_targetModel || !_targetAnimation)
	{
		DEBUG_LOG("error - UpdateBoneConstant, Not exist _targetModel or _targetAnimation - 1");
		return;
	}
	double animationTick = 0;


	if (_time * _targetAnimation->mTicksPerSecond <= _targetAnimation->mDuration)															///현재 애니메이션의 틱 수가 남아있는가?
	{
		animationTick = std::fmod(_time * _targetAnimation->mTicksPerSecond, _targetAnimation->mDuration);									//현재의 틱수를 알수 있다.
	}
	else																																	///틱수가 남아있지 않다면 고정된 값을 반환
	{
		animationTick = _targetAnimation->mDuration;
	}

	// 애니메이션 변환 행렬 계산
	CalculateTransforms(_targetModel->mRootNode, _targetAnimation, animationTick);

	for (unsigned int i = 0; i < _targetModel->mNumMesh; i++)
	{
		auto& mesh = _targetModel->mMeshData[i];

		for (unsigned int j = 0; j < mesh.mNumBones; j++)
		{
			auto& bone = mesh.mBone[j];
			auto finalTransform = bone.mOffsetMatrix * bone.mTargetNode->mTransformation * bone.mTargetNode->mWorldTransformation;

			_vsBoneConstantBuffer.bone[j] = finalTransform.Transpose();
		}
	}
}

bool AnimationHelper::UpdateBoneConstant(Model* _targetModel, Animation* _targetAnimation, Animation* _nextTargetAnimation, VSBoneConstantBufferData& _vsBoneConstantBuffer, double& _time, double& _nextTime)
{
	if (!_targetModel || !_targetAnimation || !_nextTargetAnimation)																		//데이터가 정상적으로 들어있는가?
	{
		DEBUG_LOG("error - UpdateBoneConstant, Not exist _targetModel or _targetAnimation - 2");
		return false;
	}

	if (_targetAnimation->mNumChannels != _nextTargetAnimation->mNumChannels)																//첫번째와 두번째의 mChannel값이 같은지? -> 노드 애니메이션 시 쓰는 멤버변수
	{
		DEBUG_LOG("UpdateBoneConstant ERROR - targetAnimations NumChannel is not equal");
		DEBUG_LOG("targetAnimation");
		for (int i = 0; i < _targetAnimation->mNumChannels; i++)
		{
			DEBUG_LOG(_targetAnimation->mChannels[i].mName);
		}
		DEBUG_LOG("NextTargetAnimation");
		for (int i = 0; i < _nextTargetAnimation->mNumChannels; i++)
		{
			DEBUG_LOG(_nextTargetAnimation->mChannels[i].mName);

		}
		return false;
	}

	double animationTick = 0;																												//현재의 애니메이션 시간
	double nextAnimationTick = 0;																											//다음의 애니메이션 시간
	if (_time * _targetAnimation->mTicksPerSecond <= _targetAnimation->mDuration)															///현재 애니메이션의 틱 수가 남아있는가?
	{
		animationTick = std::fmod(_time * _targetAnimation->mTicksPerSecond, _targetAnimation->mDuration);									//현재의 틱수를 알수 있다.
	}
	else																																	///틱수가 남아있지 않다면 고정된 값을 반환
	{
		animationTick = _targetAnimation->mDuration;
		UpdateBoneConstant(_targetModel, _nextTargetAnimation, _vsBoneConstantBuffer, _nextTime);
		return false;
	}

	nextAnimationTick = std::fmod(_nextTime * _nextTargetAnimation->mTicksPerSecond, _nextTargetAnimation->mDuration);						//다음의 틱수를 알 수 있다.


	if (_targetAnimation->mDuration - animationTick > _nextTargetAnimation->mDuration)														///만약 현재 애니메이션의 틱수가 다음 애니메이션의 전체 길이보다 길 경우
	{
		UpdateBoneConstant(_targetModel, _targetAnimation, _vsBoneConstantBuffer, _time);													//보간을 실행하지 않고 현재 애니메이션만 진행한다.
		_nextTime = 0;
		return true;																														//참을 반환한다.
	}

	// 애니메이션 변환 행렬 계산 -> 위의 함수와는 좀 달라야한다.
	CalculateTransforms(_targetModel->mRootNode, _targetAnimation, _nextTargetAnimation, animationTick, nextAnimationTick);					//애니메이션의 보간을 수행한다. ->Node안의 값을 계산

	for (unsigned int i = 0; i < _targetModel->mNumMesh; i++)
	{
		auto& mesh = _targetModel->mMeshData[i];

		for (unsigned int j = 0; j < mesh.mNumBones; j++)
		{
			auto& bone = mesh.mBone[j];
			auto finalTransform = bone.mOffsetMatrix * bone.mTargetNode->mTransformation * bone.mTargetNode->mWorldTransformation;
			_vsBoneConstantBuffer.bone[j] = finalTransform.Transpose();
		}
	}
	if (_nextTime * _nextTargetAnimation->mTicksPerSecond >= _nextTargetAnimation->mDuration)												//만약 다음의 애니메이션이 끝나면 false를 반환한다.
	{
		return false;
	}
	return true;

}

void AnimationHelper::CalculateTransforms(Node* _node, const Animation* _animationData, double _animationTimeInSeconds)
{

	NodeAnimation* targetNodeAnimation = nullptr;
	for (unsigned int i = 0; i < _animationData->mNumChannels; i++)		//노드 애니메이션 채널을 돌며 타겟노드를 찾는다.
	{
		if (_node->mNodeName == _animationData->mChannels[i].mName)
		{
			targetNodeAnimation = &_animationData->mChannels[i];
			break;
		}
	}

	Matrix& localTransform = _node->mTransformation;
	Matrix& worldTransform = _node->mWorldTransformation;
	if (targetNodeAnimation)
	{
		auto [prevPosKey, nextPosKey] = FindAdjacentKeys(targetNodeAnimation->mPosKey, targetNodeAnimation->mNumPosKeys, _animationTimeInSeconds);
		double factorPos = CalculateFactor(prevPosKey.mTime, nextPosKey.mTime, _animationTimeInSeconds);
		Vector3 interpolatedPosition = Lerp(prevPosKey.mValue, nextPosKey.mValue, factorPos);

		auto [prevRotKey, nextRotKey] = FindAdjacentKeys(targetNodeAnimation->mRotKey, targetNodeAnimation->mNumRotKeys, _animationTimeInSeconds);
		double factorRot = CalculateFactor(prevRotKey.mTime, nextRotKey.mTime, _animationTimeInSeconds);
		Quaternion interpolatedRotation = Slerp(prevRotKey.mValue, nextRotKey.mValue, factorRot);

		auto [prevSclKey, nextSclKey] = FindAdjacentKeys(targetNodeAnimation->mScaKey, targetNodeAnimation->mNumScaKeys, _animationTimeInSeconds);
		double factorScl = CalculateFactor(prevSclKey.mTime, nextSclKey.mTime, _animationTimeInSeconds);
		Vector3 interpolatedScale = Lerp(prevSclKey.mValue, nextSclKey.mValue, factorScl);

		Matrix translationMatrix = Matrix::CreateTranslation(interpolatedPosition);
		Matrix rotationMatrix = Matrix::CreateFromQuaternion(interpolatedRotation);
		Matrix scaleMatrix = Matrix::CreateScale(interpolatedScale);

		localTransform = scaleMatrix * rotationMatrix * translationMatrix;
	}

	for (unsigned int i = 0; i < _node->mNumNextNode; i++)
	{
		_node->mNextNode[i].mWorldTransformation = localTransform * worldTransform;
		CalculateTransforms(&_node->mNextNode[i], _animationData, _animationTimeInSeconds);
	}


}

void AnimationHelper::CalculateTransforms(Node* _node, const Animation* _animationData, const Animation* _nextAnimationData, double _animationTimeInSeconds, double _nextAnimationTimeInSeconds)
{
	NodeAnimation* targetNodeAnimation = nullptr;
	NodeAnimation* nextTargetNodeAnimation = nullptr;

	for (unsigned int i = 0; i < _animationData->mNumChannels; i++)
	{
		if (_node->mNodeName == _animationData->mChannels[i].mName)
		{
			targetNodeAnimation = &_animationData->mChannels[i];
			nextTargetNodeAnimation = &_nextAnimationData->mChannels[i];
			break;
		}
	}

	Matrix& localTransform = _node->mTransformation;
	Matrix& worldTransform = _node->mWorldTransformation;
	Matrix tempMatrix;
	Matrix tempNextMatrix;
	double prevAniWeighted = 0;
	double nextAniWeighted = 0;

	if (targetNodeAnimation)
	{
		auto [prevPosKey, nextPosKey] = FindAdjacentKeys(targetNodeAnimation->mPosKey, targetNodeAnimation->mNumPosKeys, _animationTimeInSeconds);
		double factorPos = CalculateFactor(prevPosKey.mTime, nextPosKey.mTime, _animationTimeInSeconds);
		Vector3 interpolatedPosition = Lerp(prevPosKey.mValue, nextPosKey.mValue, factorPos);

		auto [prevRotKey, nextRotKey] = FindAdjacentKeys(targetNodeAnimation->mRotKey, targetNodeAnimation->mNumRotKeys, _animationTimeInSeconds);
		double factorRot = CalculateFactor(prevRotKey.mTime, nextRotKey.mTime, _animationTimeInSeconds);
		Quaternion interpolatedRotation = Slerp(prevRotKey.mValue, nextRotKey.mValue, factorRot);

		auto [prevSclKey, nextSclKey] = FindAdjacentKeys(targetNodeAnimation->mScaKey, targetNodeAnimation->mNumScaKeys, _animationTimeInSeconds);
		double factorScl = CalculateFactor(prevSclKey.mTime, nextSclKey.mTime, _animationTimeInSeconds);
		Vector3 interpolatedScale = Lerp(prevSclKey.mValue, nextSclKey.mValue, factorScl);

		Matrix translationMatrix = Matrix::CreateTranslation(interpolatedPosition);
		Matrix rotationMatrix = Matrix::CreateFromQuaternion(interpolatedRotation);
		Matrix scaleMatrix = Matrix::CreateScale(interpolatedScale);

		tempMatrix = scaleMatrix * rotationMatrix * translationMatrix;

		prevAniWeighted = _animationData->mDuration - _animationTimeInSeconds;
	}

	if (nextTargetNodeAnimation)
	{
		auto [prevPosKey, nextPosKey] = FindAdjacentKeys(nextTargetNodeAnimation->mPosKey, nextTargetNodeAnimation->mNumPosKeys, _nextAnimationTimeInSeconds);
		double factorPos = CalculateFactor(prevPosKey.mTime, nextPosKey.mTime, _nextAnimationTimeInSeconds);
		Vector3 interpolatedPosition = Lerp(prevPosKey.mValue, nextPosKey.mValue, factorPos);

		auto [prevRotKey, nextRotKey] = FindAdjacentKeys(nextTargetNodeAnimation->mRotKey, nextTargetNodeAnimation->mNumRotKeys, _nextAnimationTimeInSeconds);
		double factorRot = CalculateFactor(prevRotKey.mTime, nextRotKey.mTime, _nextAnimationTimeInSeconds);
		Quaternion interpolatedRotation = Slerp(prevRotKey.mValue, nextRotKey.mValue, factorRot);

		auto [prevSclKey, nextSclKey] = FindAdjacentKeys(nextTargetNodeAnimation->mScaKey, nextTargetNodeAnimation->mNumScaKeys, _nextAnimationTimeInSeconds);
		double factorScl = CalculateFactor(prevSclKey.mTime, nextSclKey.mTime, _nextAnimationTimeInSeconds);
		Vector3 interpolatedScale = Lerp(prevSclKey.mValue, nextSclKey.mValue, factorScl);

		Matrix translationMatrix = Matrix::CreateTranslation(interpolatedPosition);
		Matrix rotationMatrix = Matrix::CreateFromQuaternion(interpolatedRotation);
		Matrix scaleMatrix = Matrix::CreateScale(interpolatedScale);

		tempNextMatrix = scaleMatrix * rotationMatrix * translationMatrix;
		nextAniWeighted = _nextAnimationTimeInSeconds;

	}
	if (prevAniWeighted + nextAniWeighted <= 0)
	{
		localTransform = Matrix();
	}
	else
	{
		localTransform = (tempMatrix * (prevAniWeighted / (prevAniWeighted + nextAniWeighted))) + (tempNextMatrix * (nextAniWeighted / (prevAniWeighted + nextAniWeighted)));
	}


	for (unsigned int i = 0; i < _node->mNumNextNode; i++)
	{
		_node->mNextNode[i].mWorldTransformation = localTransform * worldTransform;
		CalculateTransforms(&_node->mNextNode[i], _animationData, _nextAnimationData, _animationTimeInSeconds, _nextAnimationTimeInSeconds);
	}
}
