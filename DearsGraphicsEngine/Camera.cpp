#pragma once
#include "Camera.h"
#include <iostream>

Camera::Camera(int _screenWidth, int _screenHeight)
{
	mScreenWidth = _screenWidth;
	mScreenHeight = _screenHeight;
	mAspect = static_cast<float>(mScreenWidth) / static_cast<float>(mScreenHeight);
	mSpeed = 0.0f;
	mViewPos = Vector3(0,0,0);
	mViewDir = Vector3(0,0,1);
	mViewUp = Vector3(0,1,0);
	mRightDir = Vector3(1,0,0);
	mRoll = 0.0f;
	mPitch = 0.0f;
	mYaw = 0.0f;
	mProjFovAngleY = 0.0f;
	mNearZ = 10.0f;
	mFarZ = 100.0f;
	mIsFirstPersonMode = false;
	mPrevMouseX = 0;
	mPrevMouseY = 0;
	isPerspective = true;
}

Camera::~Camera()
{
}

void Camera::SetEyePos(Vector3 _pos)
{
	mViewPos = _pos;
}


void Camera::SetDirection(Vector3 _viewDir)
{
	// 뷰 방향 벡터를 정규화하여 설정
   _viewDir.Normalize();
   mViewDir = _viewDir;

///이 일련의 과정은 GPU에서 알아서 한다. 따라서 그냥 월드의 upvec만 넣어주면 된다.
//  // 업 벡터도 정규화하여 설정
//  _viewUp.Normalize();
//  mViewUp = _viewUp;
// 
//  // 업 벡터와 뷰 방향 벡터의 외적(Cross Product)으로 오른쪽 방향 벡터를 계산하고 정규화
//  mRightDir = _viewUp.Cross(_viewDir);	//오른손 좌표계
//  //mRightDir = _viewDir.Cross(_viewUp);	///왼손 좌표계
// 
// 	// 오른쪽 벡터와 뷰 방향 벡터의 외적을 사용하여 정확한 업 벡터를 다시 계산
// 	mViewUp = mViewDir.Cross(mRightDir);
// 	mViewUp.Normalize();
}

void Camera::SetUpVec(Vector3 _viewUp)
{
	_viewUp.Normalize();
	mViewUp = _viewUp;
}

// xy축 움직임으로 이해하면 편함
Matrix Camera::GetViewRow()
{
	// 방향이나 회전 정보도 포함하는 뷰행렬을 반환한다.
 	Vector3 targetPos = mViewDir;
 	Matrix viewMatrix = XMMatrixLookToLH(mViewPos, targetPos, mViewUp);
	//std::cout << "mViewPos = " << mViewPos.x <<" / " << mViewPos.y << " / " << mViewPos.z << std::endl;
	//std::cout << "targetPos = " << targetPos.x << " / " << targetPos.y << " / " << targetPos.z << std::endl;
	//std::cout << "mViewUp = " << mViewUp.x << " / " << mViewUp.y << " / " << mViewUp.z << std::endl;
 	return viewMatrix;
}

Matrix Camera::GetProjRow()
{
	if (isPerspective)
	{
	return DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(mProjFovAngleY), mAspect, mNearZ, mFarZ);
	}
	else
	{
	return DirectX::XMMatrixOrthographicLH(mOrthoGraphicScreenWidth, mOrthoGraphicScreenHeight, mNearZ, mFarZ);
	}
}

void Camera::SetSpeed(float _speed)
{
	mSpeed = _speed;
}

void Camera::SetPerspective()
{
	isPerspective = true;
}

void Camera::SetOrthgraphic(float _scale)
{
	mOrthoGraphicScreenWidth = mScreenWidth * _scale;
	mOrthoGraphicScreenHeight = mScreenHeight * _scale;
	isPerspective = false;
}

float Camera::GetSpeed()
{
	return mSpeed;
}

Vector3 Camera::GetmViewPos()
{
	return mViewPos;
}

void Camera::SetAircraftAxes(float _yaw, float _pitch, float _roll)
{
	mYaw = DirectX::XMConvertToRadians(_yaw);
	mPitch = DirectX::XMConvertToRadians(_pitch);
	mRoll = DirectX::XMConvertToRadians(_roll);
}

void Camera::ProjectionSettings(float _angle, float _nearZ, float _farZ)
{
	mProjFovAngleY = _angle;
	mNearZ = _nearZ;
	mFarZ = _farZ;
}

void Camera::PrintCamInfo()
{
	std::cout << "Camera - localTransform : "  << mViewPos.x << ", " << mViewPos.y << ", " << mViewPos.z << std::endl;
	std::cout << "Camera - m_viewEyeDir : " << mViewDir.x << ", " << mViewDir.y << ", " << mViewDir.z << std::endl;
	std::cout << "Camera - m_axis : " << DirectX::XMConvertToDegrees(mRoll) << ", " << DirectX::XMConvertToDegrees(mPitch) << ", " << DirectX::XMConvertToDegrees(mYaw) << std::endl;
}

void Camera::OnMouseMove(int _mouseX, int _mouseY)
{
	// 이전 프레임의 마우스 위치와 현재 프레임의 마우스 위치의 차이 계산
	float deltaX = (_mouseX - mPrevMouseX) * 0.001f; // 스케일 조정
	float deltaY = (_mouseY - mPrevMouseY) * 0.001f;
	// 카메라 시점 회전
	if (mIsFirstPersonMode)
	{
		// 카메라 시점 회전 업데이트
		UpdateFrameMouse(deltaX, deltaY);
	}
	// 현재 마우스 위치를 이전 위치로 저장
	mPrevMouseX = _mouseX;
	mPrevMouseY = _mouseY;
}

void Camera::UpdateFrameMouse(float deltaX, float deltaY)
{
	// 얼마나 회전할지를 범위를 한정
	mYaw += deltaX * DirectX::XM_2PI;     // 좌우 360도
	mPitch += deltaY * DirectX::XM_PIDIV2; // 위 아래 90도

	// 피치 각도를 제한하여 카메라가 수직으로 회전하지 않도록 함
	if (mPitch > DirectX::XM_PIDIV2)
	{
		mPitch = DirectX::XM_PIDIV2;
	}
	if (mPitch < -DirectX::XM_PIDIV2)
	{
		mPitch = -DirectX::XM_PIDIV2;
	}

	// 이동할 때 기준은 정면/ 오른쪽 방향 기준으로 계산
	UpdateViewDir();
}

void Camera::UpdateViewDir()
{
	// X축 (Pitch) 회전 행렬 생성
	Matrix rotationX = Matrix::CreateRotationX(mPitch);

	// Y축 (Yaw) 회전 행렬 생성
	Matrix rotationY = Matrix::CreateRotationY(mYaw);

	// 이동할 때 기준이 되는 정면/오른쪽 방향 계산
	mViewDir = Vector3::Transform(Vector3(0.0f, 0.0f, 1.0f), rotationX * rotationY);
	mRightDir = mViewUp.Cross(mViewDir);

}

void Camera::MoveForward(float dt)
{
	mViewPos += mViewDir * mSpeed * dt;
}

// 위로 이동
void Camera::MoveUp(float dt)
{
	// 이동후의_위치 = 현재_위치 + 이동방향 * 속도 * 시간차이;
	mViewPos += mViewUp * mSpeed * dt;
}

//오른쪽으로 이동
void Camera::MoveRight(float dt)
{
	mViewPos += mRightDir * mSpeed * dt;
}

void Camera::SetAspectRatio(int _screenWidth, int _screenHeight)
{
	mScreenWidth = _screenWidth;
	mScreenHeight = _screenHeight;
	mAspect = static_cast<float>(mScreenWidth) / static_cast<float>(mScreenHeight);
}
