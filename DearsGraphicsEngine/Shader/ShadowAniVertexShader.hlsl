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

float4 main(AniVertexShaderInput input): SV_POSITION
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

    float4 pos = mul(skinpos, world);
    //float4 pos = mul(float4(input.pos.x, input.pos.y, input.pos.z, 1.0f), world);
    return mul(pos, viewproj);
}
