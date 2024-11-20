#pragma once
//모듈로 빼야함
#include "directxTk/SimpleMath.h"
#include "Light.h"
#include "Material.h"
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector4;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Quaternion;



struct CommonConstantBufferData
{
	Matrix view;
	Matrix proj;
	Matrix viewProj;

	Matrix invView;			//뷰의 역행렬
	Matrix invProj;			//프로젝션의 역행렬
	Matrix invViewProj;		//Proj -> World

	Vector3 eyeWorld;
	float strengthIBL;

	Light light[MAX_LIGHTS];

	UINT lightNum;
	Vector3 dummy;
};

struct VSConstantBufferData
{
	Matrix world;		   //월드변환
	Matrix invWorld;	   //의 역변환
};
struct VSEdgeConstantBufferData
{
	Matrix  EdgeScaleMatrix;
};

struct PSConstantBufferData
{
	float maxLights = MAX_LIGHTS;			//12, Light의 총 개수
	Vector3 dummy;					//4
	MaterialProperties material;
};

struct PsShadowConstantBufferData
{
	Vector3 lightDirection;
	Vector3 lightColor;
	float ambientStrength;
	float dummy;
};

struct PSEdgeConstantBufferData
{
	Vector3 color;
	float dummy;
};


#define MAX_BONES 30
struct VSBoneConstantBufferData
{
	Matrix bone[MAX_BONES];
};

struct VSTargetBoneConstantBufferData
{
	Matrix targrtBoneMatrix;
};

struct VSShadowConstantBufferData
{
	Matrix ShadowView;
	Matrix ShadowProjection;
};

#define MAX_INSTANCE 100
struct VSInstantConstantBufferData
{
	Matrix world[MAX_INSTANCE];		   //월드변환
	Matrix invWorld[MAX_INSTANCE];	   //의 역변환
};

struct VSWaterConstantBufferData
{
	float time;     //deltatime;
	float speed;    //moveSpeed
	Vector2 dummy;
};

struct VSParticalConstantBufferData
{
	Vector4 RGBA;
	Matrix world;
};

#define MAX_PARTICLE 1024
struct CSParticleData
{
	Vector3 position = { 0,0,0 };			//처음 포지션
	float playTime = 0;						//진행 시간
	
	Vector3 color = { 1,1,1 };				//색상
	float lifeTime = 0;						//총 시간
	
	Vector3 direction = { 0,0,0 };			//방향
	float velocity = 0;						//속도
	
	Vector3 Rotation = { 0,0,0 };			//초기 회전값
	float opacity = 1;						//초기 투명도

	Vector3 Scale = { 1,1,1 };				//초기 스케일
	float deltaOpacity = 0;					//델타투명도

	Vector3 deltaRotation = { 0,0,0 };		//델타회전값
	float SclRandomFactor = 0;				//스케일의 랜덤 정도

	Vector3 deltaScale = { 0,0,0 };			//텔타스케일

	Vector3 PosRandomFactor = {0, 0, 0};	//포지션으로부터의 랜덤정도
	Vector3 RotRandomFactor = {0, 0, 0};	//회전의 랜덤정도
	Vector3 dirRandomFactor = { 0, 0, 0 };	//방향의 랜덤정도
	
	Vector3 deltaColor = { 0,0,0 };			//컬러의 변하는 값
	float gravity = 0;						//중력 계수

	Vector3 colorRandomFactor = { 0, 0, 0 };	//컬러의 랜덤 정도
	float dummy;
};

struct ParticleVertexInput
{
	Matrix ScalexTrans;
	Matrix Rot;
	Vector3 color;
	float Opacity;
};