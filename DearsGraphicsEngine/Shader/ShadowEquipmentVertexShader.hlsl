#include "Common.hlsli"
cbuffer VertexConstantBuffer : register(b0)
{
    matrix world;
    matrix invWorld;
}
cbuffer TargetBoneConstantBuffer : register(b3)
{
    matrix targetboneTransform;
}
float4 main(VertexShaderInput input) : SV_POSITION
{
    float4 skinpos = mul(float4(input.pos.x, input.pos.y, input.pos.z, 1.0f), targetboneTransform);
    float4 pos = mul(skinpos, world);
    return mul(pos, viewproj);
}
