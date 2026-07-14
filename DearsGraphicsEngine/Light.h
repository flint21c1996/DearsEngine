#pragma once
#include<directxTK/SimpleMath.h>
#include "LightEnum.h"
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Matrix;

#define MAX_LIGHTS 3

/// 조명
struct Light
{
	float strength;                 // 12BYTE 빛의 세기 - 빛은 1, 빛을 발광하지 않는 물체는 0
	float dummy;					// 4  //무언가 나중에 쓸 것 같다.
	float fallOffStart = 0.0f;		// 4  //빛이 끝나는 곳
	float fallOffEnd = 10.f;        // 4  //빛이 끝나는 곳
	Vector3 direction;				// 12 //방향 : 단위 벡터로 지정하세요.
	UINT lightType;					// 4 //빛의 밝기가 어두워지기 시작하는 곳.
	Vector3 position;				// 12 //빛의 위치
	float spotPower;                // 4  //빛의 밀집도
	Vector3 lightColor;				// 12 //빛의 색 조정
	UINT type;						// 4 타입

	// 그림자 카메라 설정도 라이트가 소유한다.
	// 네 개의 float를 한 묶음으로 두어 HLSL constant buffer의 16바이트 정렬을 유지한다.
	float shadowNear;
	float shadowFar;
	float shadowFovY;
	float shadowWidth;
	// 라이트 오브젝트의 로컬 +Y를 월드로 회전한 벡터다.
	// direction만으로 Up을 추측하면 수직 방향 근처에서 기준축이 바뀌며 절두체가 튈 수 있다.
	Vector3 shadowUp;
	float shadowPadding;

	Matrix viewProj;				//그림자 렌더링에 필요
	Matrix invProj;
	Light()
	{
		strength = 0.0f;
		dummy = 0.0f;
		fallOffStart = 0.0f;
		fallOffEnd = 0.0f;
		direction = Vector3(0.0f, 0.0f, 0.0f);
		lightType = 0;
		position = Vector3(0.0f, 0.0f, 0.0f);
		spotPower = 0.0f;
		lightColor = Vector3(1.0f, 1.0f, 1.0f);
		type = 0;
		shadowNear = 0.1f;
		shadowFar = 100.0f;
		shadowFovY = 70.0f;
		shadowWidth = 30.0f;
		shadowUp = Vector3::UnitY;
		shadowPadding = 0.0f;
		viewProj = Matrix();
		invProj = Matrix();
	};

	Light& operator=(const Light& _other)
	{
		if (this == &_other)
		{
			return *this;
		}
		strength = _other.strength;
		dummy = _other.dummy;
		fallOffStart = _other.fallOffStart;
		fallOffEnd = _other.fallOffEnd;
		direction = _other.direction;
		lightType = _other.lightType;
		position = _other.position;
		spotPower = _other.spotPower;
		lightColor = _other.lightColor;
		type = _other.type;
		shadowNear = _other.shadowNear;
		shadowFar = _other.shadowFar;
		shadowFovY = _other.shadowFovY;
		shadowWidth = _other.shadowWidth;
		shadowUp = _other.shadowUp;
		shadowPadding = _other.shadowPadding;
		viewProj = _other.viewProj;
		invProj = _other.invProj;
		return *this;
	}
};

// HLSL Common.hlsli의 Light도 16바이트 레지스터 13개를 사용한다.
// 필드 추가 시 한쪽 구조체만 고쳐 GPU가 잘못된 행렬 오프셋을 읽는 사고를 컴파일 단계에서 막는다.
static_assert(sizeof(Light) == 224, "C++ Light와 HLSL Light의 constant buffer 배치를 함께 수정해야 합니다.");
