#include "Common.hlsli"

cbuffer VertexConstantBuffer : register(b0)
{
    matrix world;
    matrix invWorld;
}

PBRPixelShaderInput main(VertexShaderInput input)
{
    float4 pos = float4(input.pos, 1.0f);

    PBRPixelShaderInput output;
    pos = mul(pos, world);
    output.posWorld = pos;
    pos = mul(pos, view);
    pos = mul(pos, proj);
    //pos = mul(pos, viewproj);   //x,y,z,w(Ŭ�� ����)�̰� w�� ������ ndc�������� �ٲ��.
    output.pos = pos;
    output.tangentWorld = input.tangentModel;
    output.normal = mul(input.normal, invWorld).xyz;
    output.normal = normalize(output.normal);
    
    //output.normal = normalize(mul((float3x3) invWorld, input.normal));
    output.texcoord.x = 2*input.texcoord.x -1;
    output.texcoord.y = input.texcoord.y;
    
    return output;
}
