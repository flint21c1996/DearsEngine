#include "Common.hlsli"
#include "PBRLighting.hlsli"

Texture2D albedoTex : register(t1);
Texture2D normalTex : register(t2);
Texture2D aoTex : register(t3);
Texture2D metallicTex : register(t4);
Texture2D roughnessTex : register(t5);

cbuffer PixelConstantBuffer : register(b2)
{
    float maxLights;
    int useAlbedoMap;
    int useNormalMap;
    int useAOMap;

    int useMetallicMap;
    int useRoughnessMap;
    int dummy1;
    int dummy2;

    PBRMaterial material;
};

// Forward PBR은 정점 셰이더가 전달한 탄젠트 공간 정보를 이용해 월드 노멀을 만든다.
// Deferred 경로는 Geometry Pass에서 이 결과를 G-Buffer에 저장하므로,
// Lighting Pass에서는 노멀맵을 다시 읽지 않고 저장된 월드 노멀만 복원한다.
float3 GetNormal(PBRPixelShaderInput input)
{
    float3 normalWorld = normalize(input.normal);
    if (useNormalMap)
    {
        float3 tangentNormal =
            normalTex.Sample(linearWrapSampler, input.texcoord).rgb * 2.0f - 1.0f;

        // 보간 후 Tangent와 Normal이 완벽히 직교하지 않을 수 있으므로
        // Gram-Schmidt 방식으로 Tangent를 한 번 보정한 뒤 TBN 행렬을 만든다.
        float3 tangent = normalize(
            input.tangentWorld - dot(input.tangentWorld, normalWorld) * normalWorld);
        float3 bitangent = normalize(cross(normalWorld, tangent));
        normalWorld = normalize(mul(
            tangentNormal,
            float3x3(tangent, bitangent, normalWorld)));
    }
    return normalWorld;
}

float4 main(PBRPixelShaderInput input) : SV_TARGET0
{
    float3 normalWorld = GetNormal(input);
    float3 pixelToEye = normalize(eyeWorld - input.posWorld);

    float3 albedo = useAlbedoMap
        ? albedoTex.Sample(linearWrapSampler, input.texcoord).rgb
        : material.albedo;
    float ao = useAOMap
        ? aoTex.SampleLevel(linearWrapSampler, input.texcoord, 0.0f).r
        : 1.0f;
    float metallic = useMetallicMap
        ? metallicTex.Sample(linearWrapSampler, input.texcoord).r
        : material.metallic;
    float roughness = useRoughnessMap
        ? roughnessTex.Sample(linearWrapSampler, input.texcoord).r
        : material.roughness;

    // Forward와 Deferred가 같은 공통 함수로 조명을 계산해야
    // 렌더 경로만 바꿨을 때 재질의 밝기와 그림자 모양이 달라지지 않는다.
    float3 finalColor = EvaluateScenePbrLighting(
        input.posWorld,
        normalWorld,
        pixelToEye,
        albedo,
        metallic,
        roughness,
        ao);

    return float4(clamp(finalColor, 0.0f, 1000.0f), 1.0f);
}
