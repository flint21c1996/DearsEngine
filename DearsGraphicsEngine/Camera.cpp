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
	// �� ���� ���͸� ����ȭ�Ͽ� ����
   _viewDir.Normalize();
   mViewDir = _viewDir;

///�� �Ϸ��� ������ GPU���� �˾Ƽ� �Ѵ�. ���� �׳� ������ upvec�� �־��ָ� �ȴ�.
//  // �� ���͵� ����ȭ�Ͽ� ����
//  _viewUp.Normalize();
//  mViewUp = _viewUp;
// 
//  // �� ���Ϳ� �� ���� ������ ����(Cross Product)���� ������ ���� ���͸� ����ϰ� ����ȭ
//  mRightDir = _viewUp.Cross(_viewDir);	//������ ��ǥ��
//  //mRightDir = _viewDir.Cross(_viewUp);	///�޼� ��ǥ��
// 
// 	// ������ ���Ϳ� �� ���� ������ ������ ����Ͽ� ��Ȯ�� �� ���͸� �ٽ� ���
// 	mViewUp = mViewDir.Cross(mRightDir);
// 	mViewUp.Normalize();
}

void Camera::SetUpVec(Vector3 _viewUp)
{
	_viewUp.Normalize();
	mViewUp = _viewUp;
}

// xy�� ���������� �����ϸ� ����
Matrix Camera::GetViewRow()
{
	// �����̳� ȸ�� ������ �����ϴ� ������� ��ȯ�Ѵ�.
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
	// ���� �������� ���콺 ��ġ�� ���� �������� ���콺 ��ġ�� ���� ���
	float deltaX = (_mouseX - mPrevMouseX) * 0.001f; // ������ ����
	float deltaY = (_mouseY - mPrevMouseY) * 0.001f;
	// ī�޶� ���� ȸ��
	if (mIsFirstPersonMode)
	{
		// ī�޶� ���� ȸ�� ������Ʈ
		UpdateFrameMouse(deltaX, deltaY);
	}
	// ���� ���콺 ��ġ�� ���� ��ġ�� ����
	mPrevMouseX = _mouseX;
	mPrevMouseY = _mouseY;
}

void Camera::UpdateFrameMouse(float deltaX, float deltaY)
{
	// �󸶳� ȸ�������� ������ ����
	mYaw += deltaX * DirectX::XM_2PI;     // �¿� 360��
	mPitch += deltaY * DirectX::XM_PIDIV2; // �� �Ʒ� 90��

	// ��ġ ������ �����Ͽ� ī�޶� �������� ȸ������ �ʵ��� ��
	if (mPitch > DirectX::XM_PIDIV2)
	{
		mPitch = DirectX::XM_PIDIV2;
	}
	if (mPitch < -DirectX::XM_PIDIV2)
	{
		mPitch = -DirectX::XM_PIDIV2;
	}

	// �̵��� �� ������ ����/ ������ ���� �������� ���
	UpdateViewDir();
}

void Camera::UpdateViewDir()
{
	// X�� (Pitch) ȸ�� ��� ����
	Matrix rotationX = Matrix::CreateRotationX(mPitch);

	// Y�� (Yaw) ȸ�� ��� ����
	Matrix rotationY = Matrix::CreateRotationY(mYaw);

	// �̵��� �� ������ �Ǵ� ����/������ ���� ���
	mViewDir = Vector3::Transform(Vector3(0.0f, 0.0f, 1.0f), rotationX * rotationY);
	mRightDir = mViewUp.Cross(mViewDir);

}

void Camera::MoveForward(float dt)
{
	mViewPos += mViewDir * mSpeed * dt;
}

// ���� �̵�
void Camera::MoveUp(float dt)
{
	// �̵�����_��ġ = ����_��ġ + �̵����� * �ӵ� * �ð�����;
	mViewPos += mViewUp * mSpeed * dt;
}

//���������� �̵�
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
