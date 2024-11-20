#include "Common.hlsli"

cbuffer VertexConstantBuffer : register(b0)
{
    matrix world;
    matrix invWorld;
}

cbuffer VertexConstantBuffer : register(b4)
{
    matrix edgeScaleMatrix;
}

PixelShaderInput main(VertexShaderInput input)
{
    float4 pos = float4(input.pos, 1.0f);
    
    PixelShaderInput output;
    
    float3 averageNormal = normalize(mul(input.normal, invWorld).xyz);
    
//  pos = pos + float4(averageNormal * 3.f, 0.0f);
    
    pos = mul(pos, edgeScaleMatrix);
    pos = mul(pos, world);
    output.posWorld = pos;
    pos = mul(pos, view);
    
    pos = mul(pos, proj);
    
    
    output.normal = averageNormal;
    output.ndcPos = pos.xyz / pos.w;

    output.pos = pos;
    output.texcoord = input.texcoord;
    
    return output;
}
