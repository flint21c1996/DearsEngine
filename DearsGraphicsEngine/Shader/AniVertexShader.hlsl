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
float3x3 InverseMatrix3x3(float3x3 m)
{
    // Calculate the adjugate matrix (cofactor matrix)
    float3x3 adj;
    adj[0][0] = m[1][1] * m[2][2] - m[1][2] * m[2][1];
    adj[0][1] = m[0][2] * m[2][1] - m[0][1] * m[2][2];
    adj[0][2] = m[0][1] * m[1][2] - m[0][2] * m[1][1];

    adj[1][0] = m[1][2] * m[2][0] - m[1][0] * m[2][2];
    adj[1][1] = m[0][0] * m[2][2] - m[0][2] * m[2][0];
    adj[1][2] = m[0][2] * m[1][0] - m[0][0] * m[1][2];

    adj[2][0] = m[1][0] * m[2][1] - m[1][1] * m[2][0];
    adj[2][1] = m[0][1] * m[2][0] - m[0][0] * m[2][1];
    adj[2][2] = m[0][0] * m[1][1] - m[0][1] * m[1][0];

    // Calculate the determinant
    float det = m[0][0] * adj[0][0] + m[0][1] * adj[1][0] + m[0][2] * adj[2][0];

    // Avoid division by zero (handle singular matrix)
    if (abs(det) < 1e-6) {
        return float3x3(0, 0, 0,   // Return zero matrix if determinant is near zero
                        0, 0, 0,
                        0, 0, 0);
    }

    // Calculate the inverse matrix
    return adj / det;
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
