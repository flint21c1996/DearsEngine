#include "Common.hlsli"

cbuffer VertexConstantBuffer : register(b0)
{
    matrix world;
    matrix invWorld;
}

// 본 변환 행렬을 저장하는 상수 버퍼 추가
cbuffer BoneTransforms : register(b2)
{
    matrix boneTransforms[MAX_BONES];
}

PixelShaderInput main(AniVertexShaderInput input)
{
matrix skinTransform = 
        input.boneWeights1.x * boneTransforms[input.boneIndices1.x] +
        input.boneWeights1.y * boneTransforms[input.boneIndices1.y] +
        input.boneWeights1.z * boneTransforms[input.boneIndices1.z] +
        input.boneWeights1.w * boneTransforms[input.boneIndices1.w] +
        input.boneWeights2.x * boneTransforms[input.boneIndices2.x] +
        input.boneWeights2.y * boneTransforms[input.boneIndices2.y] +
        input.boneWeights2.z * boneTransforms[input.boneIndices2.z] +
        input.boneWeights2.w * boneTransforms[input.boneIndices2.w];

   float4 skinpos = mul(float4(input.pos.x, input.pos.y, input.pos.z, 1.0f), skinTransform);
    PixelShaderInput output;

    skinpos = mul(skinpos, world);
    output.posWorld = skinpos;
    skinpos = mul(skinpos, view);
    skinpos = mul(skinpos, proj);
    
    output.pos = skinpos;

    output.normal = mul(input.normal, (float3x3)skinTransform);
    output.normal = normalize(mul(output.normal, invWorld).xyz);

    output.texcoord = input.texcoord;
            
    return output;
}
