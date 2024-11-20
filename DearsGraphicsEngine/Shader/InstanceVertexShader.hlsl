#include "Common.hlsli"
#define MAX_INSTANCE 100

cbuffer InstanceData : register(b0)
{
	matrix worlds[MAX_INSTANCE];
	matrix invWorlds[MAX_INSTANCE];
}

//기본모델
struct InstanceVertexShaderInput
{
    float3 pos : POSITION; //모델 좌표계의 위치, position
    float3 normal : NORMAL; //모델 좌표계의 normal
    float2 texcoord : TEXCOORD;
    unsigned int instanceID : SV_InstanceID;        
    /// SV_INSTANCEID 이 시맨틱을 사용하기에 GPU가 자동으로 각 인스턴스에 대해 고유한 ID를 제공한다.
    ///즉, GPU가 자동으로 instanceID세팅을 하므로 내가 따로 해줄필요는 없다
};

PixelShaderInput main(InstanceVertexShaderInput input)
{
    float4 pos = float4(input.pos, 1.0f);

    PixelShaderInput output;
    pos = mul(pos, worlds[input.instanceID]);
    output.posWorld = pos;
    pos = mul(pos, view);
    pos = mul(pos, proj);
    //pos = mul(pos, viewproj);   //x,y,z,w(클립 공간)이고 w로 나눌시 ndc공간으로 바뀐다.
    output.ndcPos = pos.xyz / pos.w;
    output.pos = pos;

    output.normal = mul(input.normal, invWorlds[input.instanceID]).xyz;
    output.normal = normalize(output.normal);

    output.texcoord = input.texcoord;
    
    return output;
}