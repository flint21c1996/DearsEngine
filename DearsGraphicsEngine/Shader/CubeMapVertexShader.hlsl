#include "Common.hlsli"

cbuffer VertexConstantBuffer : register(b0)
{
    matrix world;
    matrix invWorld;
}


PixelShaderInput main(VertexShaderInput input)
{
    float4 pos = float4(input.pos, 1.0f);

    PixelShaderInput output;
    pos = mul(pos, world);
    output.posWorld = pos;
    pos = mul(pos, view);
    pos = mul(pos, proj);
    
    output.pos = pos;

    output.normal = normalize(mul(input.normal, invWorld).xyz);

    output.texcoord = input.texcoord;
    
    return output;
}
