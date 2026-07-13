#include "Common.hlsli"

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

// MRT 순서는 C++ GBuffer::Target 순서와 반드시 일치해야 한다.
struct GBufferOutput
{
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
    float4 material : SV_Target2;
};

float3 GetWorldNormal(PBRPixelShaderInput input)
{
    float3 normalWorld = normalize(input.normal);
    if (useNormalMap)
    {
        // 노멀맵의 0~1 값을 방향 벡터의 -1~1 범위로 복원한다.
        float3 tangentNormal = normalTex.Sample(linearWrapSampler, input.texcoord).rgb * 2.0f - 1.0f;
        // 탄젠트를 노멀과 직교하도록 보정한 뒤 tangent-space 노멀을 world-space로 옮긴다.
        float3 tangent = normalize(input.tangentWorld - dot(input.tangentWorld, normalWorld) * normalWorld);
        float3 bitangent = normalize(cross(normalWorld, tangent));
        normalWorld = normalize(mul(tangentNormal, float3x3(tangent, bitangent, normalWorld)));
    }
    return normalWorld;
}

GBufferOutput main(PBRPixelShaderInput input)
{
    GBufferOutput output;
    float3 albedo = useAlbedoMap ? albedoTex.Sample(linearWrapSampler, input.texcoord).rgb : material.albedo;
    float ao = useAOMap ? aoTex.Sample(linearWrapSampler, input.texcoord).r : 1.0f;
    float metallic = useMetallicMap ? metallicTex.Sample(linearWrapSampler, input.texcoord).r : material.metallic;
    float roughness = useRoughnessMap ? roughnessTex.Sample(linearWrapSampler, input.texcoord).r : material.roughness;

    // 디버그 패널에서도 방향을 색으로 볼 수 있도록 -1~1 노멀을 0~1로 인코딩한다.
    // Lighting Pass에서는 encodedNormal * 2 - 1 계산으로 다시 복원하면 된다.
	output.albedo = float4(albedo, 1.0f);
    // ImGui 미리보기는 텍스처의 Alpha로 블렌딩한다.
    // Alpha가 0이면 데이터가 정상이어도 완전히 투명하게 보이므로 1로 저장한다.
    output.normal = float4(GetWorldNormal(input) * 0.5f + 0.5f, 1.0f);
    output.material = float4(metallic, roughness, ao, 1.0f);
    return output;
}
