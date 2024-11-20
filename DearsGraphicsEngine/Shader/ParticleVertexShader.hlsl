#include "Common.hlsli"
#define MAX_INSTANCE 100

struct PSInput // GS�� �ִٸ� GSInput���� ����
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
    float opacity :OPACITY;
    unsigned int available : AVAILABLE;        //����� ���� �ø�ƽ
};

struct ParticleVertexInput
{
    matrix ScalexTrans;             // scale * transpose Matrix
    matrix Rot;                     // rotationMatrix
    float3 color;
    float Opacity;
};

struct ParticleIndexInputs
{
    unsigned int available;
};

StructuredBuffer<ParticleVertexInput> ParticleVertexInputs : register(t0);
StructuredBuffer<ParticleIndexInputs> AvilableIndexInputs : register(t1);


// VSInput�� ���� vertexID�� ���
PSInput main(VertexShaderInput input, uint instanceID : SV_InstanceID)
{

    // �ν��Ͻ� ��ġ�� �����Ͽ� ������ ���� ��ġ�� ���
    float4 worldPos = float4(input.pos, 1.0);

    Matrix viewNoTrans = invView;
    viewNoTrans[3] = float4(0, 0, 0, 1);
    worldPos = mul(worldPos, ParticleVertexInputs[instanceID].Rot);
    worldPos = mul(worldPos, viewNoTrans);

    worldPos = mul(worldPos, ParticleVertexInputs[instanceID].ScalexTrans);


    // ���� -> �� -> �������� ��ȯ
    worldPos = mul(worldPos, view);
    worldPos = mul(worldPos, proj);

    PSInput output;
    output.position = worldPos;
    output.available = AvilableIndexInputs[instanceID].available;
    output.opacity = ParticleVertexInputs[instanceID].Opacity;
    output.color = ParticleVertexInputs[instanceID].color;

    return output;

}
