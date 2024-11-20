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

///��������, �� �� ������ ���� ��θ� ���� ����, ���ͳ� ��Į�󿡼� ���
Vector3 AnimationHelper::Lerp(const Vector3& start, const Vector3& end, double t)
{
	return start + (end - start) * t;
}

///���� ��������, �� �������� ���� ��θ� ���� ����, �ַ� ���ʹϾ𿡼� ���
Quaternion AnimationHelper::Slerp(const Quaternion& start, const Quaternion& end, double t)
{
	return Quaternion::Slerp(start, end, t);
}

///�ִϸ��̼��� �ð� ������ ���
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


	if (_time * _targetAnimation->mTicksPerSecond <= _targetAnimation->mDuration)															///���� �ִϸ��̼��� ƽ ���� �����ִ°�?
	{
		animationTick = std::fmod(_time * _targetAnimation->mTicksPerSecond, _targetAnimation->mDuration);									//������ ƽ���� �˼� �ִ�.
	}
	else																																	///ƽ���� �������� �ʴٸ� ������ ���� ��ȯ
	{
		animationTick = _targetAnimation->mDuration;
	}

	// �ִϸ��̼� ��ȯ ��� ���
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
	if (!_targetModel || !_targetAnimation || !_nextTargetAnimation)																		//�����Ͱ� ���������� ����ִ°�?
	{
		DEBUG_LOG("error - UpdateBoneConstant, Not exist _targetModel or _targetAnimation - 2");
		return false;
	}

	if (_targetAnimation->mNumChannels != _nextTargetAnimation->mNumChannels)																//ù��°�� �ι�°�� mChannel���� ������? -> ��� �ִϸ��̼� �� ���� �������
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

	double animationTick = 0;																												//������ �ִϸ��̼� �ð�
	double nextAnimationTick = 0;																											//������ �ִϸ��̼� �ð�
	if (_time * _targetAnimation->mTicksPerSecond <= _targetAnimation->mDuration)															///���� �ִϸ��̼��� ƽ ���� �����ִ°�?
	{
		animationTick = std::fmod(_time * _targetAnimation->mTicksPerSecond, _targetAnimation->mDuration);									//������ ƽ���� �˼� �ִ�.
	}
	else																																	///ƽ���� �������� �ʴٸ� ������ ���� ��ȯ
	{
		animationTick = _targetAnimation->mDuration;
		UpdateBoneConstant(_targetModel, _nextTargetAnimation, _vsBoneConstantBuffer, _nextTime);
		return false;
	}

	nextAnimationTick = std::fmod(_nextTime * _nextTargetAnimation->mTicksPerSecond, _nextTargetAnimation->mDuration);						//������ ƽ���� �� �� �ִ�.


	if (_targetAnimation->mDuration - animationTick > _nextTargetAnimation->mDuration)														///���� ���� �ִϸ��̼��� ƽ���� ���� �ִϸ��̼��� ��ü ���̺��� �� ���
	{
		UpdateBoneConstant(_targetModel, _targetAnimation, _vsBoneConstantBuffer, _time);													//������ �������� �ʰ� ���� �ִϸ��̼Ǹ� �����Ѵ�.
		_nextTime = 0;
		return true;																														//���� ��ȯ�Ѵ�.
	}

	// �ִϸ��̼� ��ȯ ��� ��� -> ���� �Լ��ʹ� �� �޶���Ѵ�.
	CalculateTransforms(_targetModel->mRootNode, _targetAnimation, _nextTargetAnimation, animationTick, nextAnimationTick);					//�ִϸ��̼��� ������ �����Ѵ�. ->Node���� ���� ���

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
	if (_nextTime * _nextTargetAnimation->mTicksPerSecond >= _nextTargetAnimation->mDuration)												//���� ������ �ִϸ��̼��� ������ false�� ��ȯ�Ѵ�.
	{
		return false;
	}
	return true;

}

void AnimationHelper::CalculateTransforms(Node* _node, const Animation* _animationData, double _animationTimeInSeconds)
{

	NodeAnimation* targetNodeAnimation = nullptr;
	for (unsigned int i = 0; i < _animationData->mNumChannels; i++)		//��� �ִϸ��̼� ä���� ���� Ÿ�ٳ�带 ã�´�.
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
