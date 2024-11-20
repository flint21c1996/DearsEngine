#pragma once
#include "directxTk/SimpleMath.h"
using DirectX::SimpleMath::Vector3;

/// 재질
struct MaterialProperties
{
	Vector3 ambient = Vector3(0.1f);		// 12byte
	float shininess = 0.01f;					// 4byte	/광택
	Vector3 diffuse = Vector3(0.5f);		// 12byte	
	float dummy1 = 0.0f;					// 4byte	/더미 데이터
	Vector3 specular = Vector3(0.5f);		// 12byte
	float dummy2 = 0.0f;					// 4byte	/더미 데이터
	//Vector3 fresnel = Vector3(0.5f, 0.5f, 0.5f);			 //기본값으로 대충 무난한걸 적어놓음
	//Vector3 fresnel = Vector3(1.0f, 0.71f, 0.29f);		 //gold
	Vector3 fresnel = Vector3(0.02, 0.02, 0.02);		 //water
	float dummy3; //16바이트
};

///9.9.2024 DJ 01:17
///diffuse와 specular을 vector3로 하는 이유는 RGB값에 따라 각 성분을 따로 다루기 위함이다.


///Schlick 근사값(approximation) : 9.17 "리얼타임렌더링"
//_frenel은 물질의 고유성질
//Water   :(0.02, 0.02, 0.02) 
//Glass   :(0.08, 0.08, 0.08) 
//Plastic :(0.05, 0.05, 0.05) 
//Gold    :( 1.0, 0.71, 0.29) 
//Silver  :(0.95, 0.93, 0.88) 
//Copper  :(0.95, 0.64, 0.54)