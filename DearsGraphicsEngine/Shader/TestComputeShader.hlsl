#include "Common.hlsli"
struct Particle
{
	float3 pos;			//처음 포지션
	float playTime;				//진행 시간
	
	float3 color;				//색상
	float lifeTime;				//총 시간
	
	float3 direction;			//방향
	float velocity;				//속도
	
	float3 Rotation;			//초기 회전값
	float Opacity;				//초기 투명도

	float3 Scale;				//초기 스케일
	float deltaOpacity;			//델타투명도

	float3 deltaRotation;		//델타회전값
	float ScleRandomFactor;		//스케일의 랜덤정도
	
	float3 deltaScale;			//델타스케일

    float3 PosRandomFactor;		//포지션의 랜덤정도. 
    float3 RotRandomFactor;		//회전의 랜덤정도.
    float3 dirRandomFactor;
	
    float3 deltaColor;			//컬러의 변하는 값
    float gravity;				//중력계수
	
    float3 colorRandomFactor ; //컬러의 랜덤 정도
    float dummy;			
};

struct ParticleVertexInput
{
	matrix ScalexTrans;
	matrix Rot;
    float3 color;
    float Opacity;
};


static float dt = 1 / 60.0; // ConstBuffer로 받아올 수 있음, 지금은 그냥 하드로 박는다.
#define GRAVITY_ACCELERATION float3(0, -9.8, 0) // y축 방향의 중력 가속도 (단위: m/s^2)

RWStructuredBuffer<Particle> outputParticles : register(u0);
RWStructuredBuffer<unsigned int> particlState : register(u1);
RWStructuredBuffer<ParticleVertexInput> InputVertexShader : register(u2);

[numthreads(256, 1, 1)]
void main(int3 gID : SV_GroupID,                     //(0 ~ 3, 0, 0)
          int3 gtID : SV_GroupThreadID,              //(0~255, 0, 0)
          uint3 dtID : SV_DispatchThreadID)          //(0~1023, 0, 0)
{

	if(particlState[dtID.x] == 1)
	{
		Particle p = outputParticles[dtID.x];
		
		//방향
        float3 dir = p.direction;
        dir += float3(Random(dtID.x + 8.1) * p.dirRandomFactor.x,
					  Random(dtID.x + 9.1) * p.dirRandomFactor.y,
					  Random(dtID.x + 10.1) * p.dirRandomFactor.z);
		
		float len = length(dir);
		dir = (len > 0.00001) ? normalize(dir): float3(0, 0, 0);

		// 중력 적용: 중력 가속도를 속도에 더하기
        float3 gravityEffect = GRAVITY_ACCELERATION * p.gravity * p.playTime; // 중력 가속도 * 중력 계수 * 시간
        float3 velocity = dir * p.velocity; // 기존 속도
		
		// 속도에 중력을 반영
        velocity += gravityEffect;
		
		//포지션
        p.pos += velocity * p.playTime;
        p.pos.x += Random(dtID.x + 1.1f) * p.PosRandomFactor.x;
        p.pos.y += Random(dtID.x + 2.1f) * p.PosRandomFactor.y;
        p.pos.z += Random(dtID.x + 3.1f) * p.PosRandomFactor.z;
		
		//회전
		p.Rotation += p.deltaRotation * p.playTime;
        p.Rotation.x += Random(dtID.x + 4.1f) * 3.141592 * p.RotRandomFactor.x;
        p.Rotation.y += Random(dtID.x + 5.1f) * 3.141592 * p.RotRandomFactor.y;
        p.Rotation.z += Random(dtID.x + 6.1f) * 3.141592 * p.RotRandomFactor.z;
		
		//스케일
		p.Scale += p.deltaScale * p.playTime;
        p.Scale += Random(dtID.x + 7.1f) * p.ScleRandomFactor;
		
		//투명도
		//float tempOpacity = p.Opacity+(p.deltaOpacity * p.playTime);
		//p.Opacity += p.deltaOpacity * p.playTime;
		p.Opacity += p.deltaOpacity * p.playTime;
		//색깔
        p.color += float3(Random(dtID.x + 11.1) * p.colorRandomFactor.r,
						  Random(dtID.x + 12.1) * p.colorRandomFactor.g,
						  Random(dtID.x + 13.1) * p.colorRandomFactor.b);
		
        p.color += p.deltaColor * p.playTime;
		
		// 월드 행렬 계산: 스케일 -> 회전 -> 이동 순서로 적용
		float4x4 scaleMatrix = ScaleMatrix(p.Scale);
		float4x4 rotationMatrix = RotationMatrix(p.Rotation);
		float4x4 translationMatrix = TranslationMatrix(p.pos);
	 
		// VertexInput에 월드 및 역월드 행렬 저장
        InputVertexShader[dtID.x].ScalexTrans = mul(scaleMatrix, translationMatrix);;	//포지션 * 스케일 값을 매트릭스로 변환한다.	
        InputVertexShader[dtID.x].Rot = rotationMatrix;									//회전 변환.
        InputVertexShader[dtID.x].Opacity = p.Opacity;									//투명도
        InputVertexShader[dtID.x].color = p.color;										//색
		
		outputParticles[dtID.x].playTime += dt;
		
		if(outputParticles[dtID.x].playTime > outputParticles[dtID.x].lifeTime)
		{
			particlState[dtID.x] = 0;
		}

    }
	///나중에 꼭! 아래 함수를 찾아서 공부하자. 쓰레드 동기화에 대한 거니...
	// GroupMemoryBarrier()
	// DeviceMemoryBarrier()
	// AllMemoryBarrier()
    // GroupMemoryBarrierWithGroupSync();
    // DeviceMemoryBarrierWithGroupSync();
	// AllMemoryBarrierWithGroupSync();
}
