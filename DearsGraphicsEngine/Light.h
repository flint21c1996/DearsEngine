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
	float fallOffStart;				// 4  //빛이 끝나는 곳
	float fallOffEnd;               // 4  //빛이 끝나는 곳
	Vector3 direction;				// 12 //방향 : 단위 벡터로 지정하세요.
	UINT lightType;					// 4 //빛의 밝기가 어두워지기 시작하는 곳.
	Vector3 position;				// 12 //빛의 위치
	float spotPower;                // 4  //빛의 밀집도
	Vector3 lightColor;				// 12 //빛의 색 조정
	UINT type;					// 4 //더미

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
		viewProj = Matrix();
		invProj = Matrix();
	};

	Light& operator=(Light& _other)
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
		viewProj = _other.viewProj;
		invProj = _other.invProj;
		return *this;
	}
};