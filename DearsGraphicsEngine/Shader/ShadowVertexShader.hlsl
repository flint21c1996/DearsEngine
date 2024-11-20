#include "Common.hlsli"


cbuffer VertexConstantBuffer : register(b0)
{
    matrix world;
    matrix invWorld;
}
float4 main(VertexShaderInput input) : SV_POSITION
{
    float4 pos = mul(float4(input.pos, 1.0f), world);
    return mul(pos, viewproj);
}
