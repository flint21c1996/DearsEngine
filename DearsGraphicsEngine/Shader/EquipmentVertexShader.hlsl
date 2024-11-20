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

PixelShaderInput main(VertexShaderInput input)
{
   float4 skinpos = mul(float4(input.pos.x, input.pos.y, input.pos.z, 1.0f), targetboneTransform);
    PixelShaderInput output;

    skinpos = mul(skinpos, world);

    output.posWorld = skinpos.xyz;

    skinpos = mul(skinpos, view);
    skinpos = mul(skinpos, proj);
    
    output.pos = skinpos;

    output.normal = mul(input.normal, (float3x3)targetboneTransform);
    output.normal = normalize(mul(output.normal, invWorld)).xyz;

    output.texcoord = input.texcoord;
            
    return output;
}
