#include "Common.hlsli"

cbuffer VertexConstantBuffer : register(b0)
{
    matrix world;
    matrix invWorld;
}

cbuffer WaterVertexConstantBuffer : register(b5)
{
    float time;     //deltatime;
    float speed;    //moveSpeed
    float2 dummy;
}

PixelShaderInput main(VertexShaderInput input)
{
    float4 pos = float4(input.pos, 1.0f);
    
    PixelShaderInput output;
    float3 averageNormal = normalize(mul(input.normal, invWorld).xyz);
    
    pos = mul(pos, world);
    output.posWorld = pos;
    
    pos = mul(pos, view);
    pos = mul(pos, proj);
    
    output.pos = pos;
    float2 moveCoord = input.texcoord;
    moveCoord.x += time * speed;
    output.normal = averageNormal;

    output.texcoord = moveCoord;
    
    return output;
}
