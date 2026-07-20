#include "Common.hlsli"

// 메인 PBR Vertex Shader와 같은 Height Texture를 읽는다.
// 화면에 보이는 메시만 변형하고 Shadow Pass가 원본 메시를 사용하면
// 물체와 그림자의 실루엣이 서로 달라지므로 두 패스가 같은 변형식을 공유해야 한다.
Texture2D HeightTex : register(t0);

cbuffer PBRVertexConstantBuffer : register(b6)
{
    matrix world;
    matrix invWorld;
    int useHeightMap;
    float heightScale;
    float dummy1;
    float dummy2;
}

float4 main(VertexShaderInput input) : SV_POSITION
{
    float4 positionWorld = mul(float4(input.pos, 1.0f), world);

    // PBRVertexShader.hlsl과 동일한 방식으로 모델 노멀을 월드 노멀로 변환한다.
    // Height 값은 0~1 텍스처 범위를 -1~1로 바꾼 뒤 노멀 방향 변위로 사용한다.
    if (useHeightMap)
    {
        float3 normalWorld = normalize(mul(input.normal, invWorld).xyz);
        float height = HeightTex.SampleLevel(linearWrapSampler, input.texcoord, 0).r;
        height = height * 2.0f - 1.0f;
        positionWorld.xyz += normalWorld * height * heightScale;
    }

    // viewproj는 현재 Shadow Pass의 라이트 카메라 ViewProjection 행렬이다.
    // 따라서 변형된 월드 정점의 깊이가 그대로 해당 라이트의 Shadow Map에 기록된다.
    return mul(positionWorld, viewproj);
}
