#include "Common.hlsli"
#define MAX_INSTANCE 100

cbuffer InstanceData : register(b0)
{
	matrix worlds[MAX_INSTANCE];
	matrix invWorlds[MAX_INSTANCE];
}

//�⺻��
struct InstanceVertexShaderInput
{
    float3 pos : POSITION; //�� ��ǥ���� ��ġ, position
    float3 normal : NORMAL; //�� ��ǥ���� normal
    float2 texcoord : TEXCOORD;
    unsigned int instanceID : SV_InstanceID;        
    /// SV_INSTANCEID �� �ø�ƽ�� ����ϱ⿡ GPU�� �ڵ����� �� �ν��Ͻ��� ���� ������ ID�� �����Ѵ�.
    ///��, GPU�� �ڵ����� instanceID������ �ϹǷ� ���� ���� �����ʿ�� ����
};

PixelShaderInput main(InstanceVertexShaderInput input)
{
    float4 pos = float4(input.pos, 1.0f);

    PixelShaderInput output;
    pos = mul(pos, worlds[input.instanceID]);
    output.posWorld = pos;
    pos = mul(pos, view);
    pos = mul(pos, proj);
    //pos = mul(pos, viewproj);   //x,y,z,w(Ŭ�� ����)�̰� w�� ������ ndc�������� �ٲ��.
    output.ndcPos = pos.xyz / pos.w;
    output.pos = pos;

    output.normal = mul(input.normal, invWorlds[input.instanceID]).xyz;
    output.normal = normalize(output.normal);

    output.texcoord = input.texcoord;
    
    return output;
}