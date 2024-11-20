#pragma once
#include <directxtk/SimpleMath.h>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

class Camera
{
public:
	Camera(int _screenWidth, int _screenHeight);
	~Camera();

private:
	int mScreenWidth;	// 스크린 너비
	int mScreenHeight;	// 스크린 높이

	int mOrthoGraphicScreenWidth = 192;
	int mOrthoGraphicScreenHeight = 108;

	float mAspect;		// 화면 비율
	float mSpeed;		// 움직이는 속도

	int mPrevMouseX;	// 각 프레임마다 마우스가 움직이는 정도를 계산하기 위한 변수(마우스 x)
	int mPrevMouseY;	// 각 프레임마다 마우스가 움직이는 정도를 계산하기 위한 변수(마우스 y)
public:
	Vector3 mViewPos;	// 월드 좌표계에서 카메라의 위치

	Vector3 mViewDir;	// 카메라가 보는 방향, 걸어가는 방향
	Vector3 mViewUp;	// 현재 시각이 보는 방향의 위 벡터 
	Vector3 mRightDir;	// 현재 시각이 보는 방향의 오른쪽 벡터 (연산으로 구함)

	// roll, pitch, yaw
	// https://en.wikipedia.org/wiki/Aircraft_principal_axes
	float mPitch;			//x축 기준 회전
	float mYaw;				//y축 기준 회전
	float mRoll;			//z축 기준 회전

	float mProjFovAngleY;
	float mNearZ;
	float mFarZ;

	bool isPerspective;
public:
	//[화면 너비, 화면 높이] 화면 비율 세팅-> 게임 엔진에서 바꾸기 때문에 카메라 멤버 변수 값도 바꾼다.
	void SetAspectRatio(int _screenWidth, int _screenHeight);

	// [위치]로컬 위치를 세팅
	void SetEyePos(Vector3 _pos);

	// [속도]카메라의 속도 세팅
	void SetSpeed(float _speed);
	void SetPerspective();
	void SetOrthgraphic(float _scale);
	// [뷰 방향, 업 방향] 카메라의 뷰, 업, right 세팅
	void SetDirection(Vector3 _viewDir);
	void SetUpVec(Vector3 _viewUp);

	///이 친구 손 봐야..
	// 카메라의 뷰행렬 반환(xy축 움직임)
	Matrix GetViewRow();

	// 카메라의 투영행렬 반환(yz축 움직임)
	Matrix GetProjRow();

	// 카메라의 속도 반환
	float GetSpeed();

	//카메라의 월드 위치 반환
	Vector3 GetmViewPos();


	// [roll(θ), pitch(θ), yaw(θ)] 항공기 주축 값 세팅 // 여기에서 y : 상하 회전 ,  z : 좌우 회전 [쓸필요x]
	void SetAircraftAxes(float _yaw, float _pitch, float _roll);

	// [Y시야각, nearZ, farZ] 프로젝션 행렬 제작을 위한 세팅
	void ProjectionSettings(float _angle, float _nearZ, float _farZ);

public:
	// [위치x, 위치y] 마우스 포지션 업데이트
	void UpdateFrameMouse(float deltaX, float deltaY);
	// 회전된 mViewDir를 적용해 계산한다.
	void UpdateViewDir();
	// [델타 타임] 앞으로 이동
	void MoveForward(float dt);
	// [델타 타임] 위로 이동
	void MoveUp(float dt);
	// [델타 타임] 오른쪽으로 이동
	void MoveRight(float dt);

public:
	// 카메라의 현재 위치xyz와 보는 방향xyz 출력
	void PrintCamInfo();
	// [위치x, 위치y] 마우스에 따라 화면 각도 변경
	void OnMouseMove(int _mouseX, int _mouseY);

public:
	bool mIsFirstPersonMode;	// 1인칭 시점 사용 여부
};
