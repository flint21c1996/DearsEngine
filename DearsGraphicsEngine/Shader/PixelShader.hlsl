#include "Common.hlsli"

Texture2D g_texture0 : register(t0);

cbuffer PixelConstantBuffer : register(b0)
{
    float maxLights;
    float3 dummy;
    Material material;
};

#define LIGHT_DIRECTIONAL 1
#define LIGHT_SPOT 2
float3 LightRadiance(Light light, float3 posWorld, Texture2D shadowMap)
{
     // 월드 좌표에서 라이트 뷰-투영 좌표로 변환
    float4 lightSpacePos = mul(float4(posWorld, 1.0f), light.viewProj);

    // 클립 공간 좌표에서 NDC 좌표로 변환
    lightSpacePos.xy /= float2(1920.0f, 1080.0f);

    //float2 screenPos = input.pos.xy / float2(1920.0f, 1080.0f);
    
    // 텍스처 좌표로 변환 (0, 1 범위)
    float2 shadowTexCoord = lightSpacePos.xy * 0.5f + 0.5f;

    // 그림자 맵에서 깊이 값을 샘플링 (현재 좌표의 깊이)
    float shadowMapDepth = shadowMap.Sample(shadowPointSampler, shadowTexCoord).r;

    // 그림자 맵에서의 z 값과 비교
    float bias = 0.005f; // 그림자 경계를 부드럽게 하기 위한 바이어스
    float shadowFactor = (lightSpacePos.z - bias > shadowMapDepth) ? 0.0f : 1.0f;

    // 빛의 색과 그림자 효과 적용
    return light.lightColor * shadowFactor;
}




//
//
//
float4 main(PixelShaderInput input) : SV_TARGET0
{
    float3 normalWorld = normalize(input.normal);
    float3 toEye = normalize(eyeWorld - input.posWorld);
    float3 directLighting = float3(0.0f, 0.0f, 0.0f);

    // Scene Hierarchy에서 수집한 라이트를 하나씩 계산한다.
    // 그림자 맵을 만든 lights[0]의 결과만 어둡게 해야 다른 라이트의 직접광은 그대로 남는다.
    [loop]
    for (uint lightIndex = 0; lightIndex < min(lightNum, (uint)MAX_LIGHTS); ++lightIndex)
    {
        Light light = lights[lightIndex];
        float3 lightContribution = float3(0.0f, 0.0f, 0.0f);
        if (light.lightType == 1)
        {
            lightContribution = ComputeDirectionalLight(light, material, normalWorld, toEye);
        }
        else if (light.lightType == 2)
        {
            lightContribution = ComputePointLight(
                light, material, input.posWorld, normalWorld, toEye);
        }
        else if (light.lightType == 3)
        {
            lightContribution = ComputeSpotLight(
                light, material, input.posWorld, normalWorld, toEye);
        }

        // 각 라이트는 자신의 lightIndex와 같은 Shadow Map 배열 슬라이스를 사용한다.
        // 따라서 첫 번째 라이트뿐 아니라 현재 루프의 모든 라이트에 그림자를 적용한다.
        lightContribution *= CalculateShadowFactor(
            light, lightIndex, input.posWorld, normalWorld);
        directLighting += lightContribution;
    }

    // 일반 메시의 환경광은 그림자 여부와 관계없이 남긴다.
    // 그림자 계수는 위 루프에서 해당 직접광에만 적용되어 있으므로 최종 색 전체에 다시 곱하지 않는다.
    float4 baseTexture = g_texture0.Sample(linearWrapSampler, input.texcoord);
    float3 directLight = directLighting * baseTexture.rgb;
    float3 ambient = material.ambient * material.diffuse * baseTexture.rgb;
    return float4(ambient + directLight, baseTexture.a);
}

// /혹시 메인 RT0에 깊이맵을 보고싶을경우.
// 깊이 값을 선형화하는 함수
// float LinearizeDepth(float depth, float nearPlane, float farPlane)
// {
//     return (2.0 * nearPlane) / (farPlane + nearPlane - depth * (farPlane - nearPlane));
// }
// 
// float4 main(PixelShaderInput input) : SV_TARGET
// {
//     float2 screenPos = input.pos.xy / float2(1920.0f, 1080.0f); // 입력 좌표를 해상도로 나누어 -> NDC 좌표로 변환
//      
//     float4 texcoord = mul(float4(input.posWorld, 1.0f), viewproj);
//     float2 screen = texcoord.xy / texcoord.w;                
//     screen.y *= -1;
//     screen = screen *0.5f +0.5f;
// 
//     // 깊이 값을 텍스처 좌표를 이용해 샘플링
//     //float depthValue = shadowMaps[0].Sample(shadowPointSampler, screenPos).r;
//     float depthValue = shadowMaps[0].Sample(shadowPointSampler, screen).r;
//     
//     //색상의 변화를 뚜렷히 하기 위해서.
//     depthValue = LinearizeDepth(depthValue, 0.1, 100);
// 
//     // 깊이 값을 그레이스케일로 변환 (0.0 ~ 1.0)
//     return float4(depthValue, depthValue, depthValue, 1.0f); // RGB에 동일한 값을 넣어 그레이스케일로 출력
// }

