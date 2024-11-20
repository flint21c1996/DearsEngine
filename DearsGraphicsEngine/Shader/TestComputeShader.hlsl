#include "Common.hlsli"
struct Particle
{
	float3 pos;			//ó�� ������
	float playTime;				//���� �ð�
	
	float3 color;				//����
	float lifeTime;				//�� �ð�
	
	float3 direction;			//����
	float velocity;				//�ӵ�
	
	float3 Rotation;			//�ʱ� ȸ����
	float Opacity;				//�ʱ� ����

	float3 Scale;				//�ʱ� ������
	float deltaOpacity;			//��Ÿ����

	float3 deltaRotation;		//��Ÿȸ����
	float ScleRandomFactor;		//�������� ��������
	
	float3 deltaScale;			//��Ÿ������

    float3 PosRandomFactor;		//�������� ��������. 
    float3 RotRandomFactor;		//ȸ���� ��������.
    float3 dirRandomFactor;
	
    float3 deltaColor;			//�÷��� ���ϴ� ��
    float gravity;				//�߷°��
	
    float3 colorRandomFactor ; //�÷��� ���� ����
    float dummy;			
};

struct ParticleVertexInput
{
	matrix ScalexTrans;
	matrix Rot;
    float3 color;
    float Opacity;
};


static float dt = 1 / 60.0; // ConstBuffer�� �޾ƿ� �� ����, ������ �׳� �ϵ�� �ڴ´�.
#define GRAVITY_ACCELERATION float3(0, -9.8, 0) // y�� ������ �߷� ���ӵ� (����: m/s^2)

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
		
		//����
        float3 dir = p.direction;
        dir += float3(Random(dtID.x + 8.1) * p.dirRandomFactor.x,
					  Random(dtID.x + 9.1) * p.dirRandomFactor.y,
					  Random(dtID.x + 10.1) * p.dirRandomFactor.z);
		
		float len = length(dir);
		dir = (len > 0.00001) ? normalize(dir): float3(0, 0, 0);

		// �߷� ����: �߷� ���ӵ��� �ӵ��� ���ϱ�
        float3 gravityEffect = GRAVITY_ACCELERATION * p.gravity * p.playTime; // �߷� ���ӵ� * �߷� ��� * �ð�
        float3 velocity = dir * p.velocity; // ���� �ӵ�
		
		// �ӵ��� �߷��� �ݿ�
        velocity += gravityEffect;
		
		//������
        p.pos += velocity * p.playTime;
        p.pos.x += Random(dtID.x + 1.1f) * p.PosRandomFactor.x;
        p.pos.y += Random(dtID.x + 2.1f) * p.PosRandomFactor.y;
        p.pos.z += Random(dtID.x + 3.1f) * p.PosRandomFactor.z;
		
		//ȸ��
		p.Rotation += p.deltaRotation * p.playTime;
        p.Rotation.x += Random(dtID.x + 4.1f) * 3.141592 * p.RotRandomFactor.x;
        p.Rotation.y += Random(dtID.x + 5.1f) * 3.141592 * p.RotRandomFactor.y;
        p.Rotation.z += Random(dtID.x + 6.1f) * 3.141592 * p.RotRandomFactor.z;
		
		//������
		p.Scale += p.deltaScale * p.playTime;
        p.Scale += Random(dtID.x + 7.1f) * p.ScleRandomFactor;
		
		//����
		//float tempOpacity = p.Opacity+(p.deltaOpacity * p.playTime);
		//p.Opacity += p.deltaOpacity * p.playTime;
		p.Opacity += p.deltaOpacity * p.playTime;
		//����
        p.color += float3(Random(dtID.x + 11.1) * p.colorRandomFactor.r,
						  Random(dtID.x + 12.1) * p.colorRandomFactor.g,
						  Random(dtID.x + 13.1) * p.colorRandomFactor.b);
		
        p.color += p.deltaColor * p.playTime;
		
		// ���� ��� ���: ������ -> ȸ�� -> �̵� ������ ����
		float4x4 scaleMatrix = ScaleMatrix(p.Scale);
		float4x4 rotationMatrix = RotationMatrix(p.Rotation);
		float4x4 translationMatrix = TranslationMatrix(p.pos);
	 
		// VertexInput�� ���� �� ������ ��� ����
        InputVertexShader[dtID.x].ScalexTrans = mul(scaleMatrix, translationMatrix);;	//������ * ������ ���� ��Ʈ������ ��ȯ�Ѵ�.	
        InputVertexShader[dtID.x].Rot = rotationMatrix;									//ȸ�� ��ȯ.
        InputVertexShader[dtID.x].Opacity = p.Opacity;									//����
        InputVertexShader[dtID.x].color = p.color;										//��
		
		outputParticles[dtID.x].playTime += dt;
		
		if(outputParticles[dtID.x].playTime > outputParticles[dtID.x].lifeTime)
		{
			particlState[dtID.x] = 0;
		}

    }
	///���߿� ��! �Ʒ� �Լ��� ã�Ƽ� ��������. ������ ����ȭ�� ���� �Ŵ�...
	// GroupMemoryBarrier()
	// DeviceMemoryBarrier()
	// AllMemoryBarrier()
    // GroupMemoryBarrierWithGroupSync();
    // DeviceMemoryBarrierWithGroupSync();
	// AllMemoryBarrierWithGroupSync();
}
