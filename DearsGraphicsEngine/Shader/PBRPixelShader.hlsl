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
    int useAOMap; // Ambient Occlusion
    
    int useMetallicMap;
    int useRoughnessMap;
    int dummy1;
    int dummy2;
    
    PBRMaterial material;
};

#define LIGHT_DIRECTIONAL 1
#define LIGHT_SPOT 2
static const float3 Fdielectric = 0.04; // 비금속(Dielectric) 재질의 F0
//사실 common.hlsli에 있는 함수랑 똑같은 함수다..
float3 PBRSchlickFresnel(float3 F0, float NdotV)
{
    float f = 1.0f - NdotV; 
    // 90도일 경우 1
    // 0도일 경우 0
    //즉, normal과 view벡터가 90에 가깝다 -> 가장자리다. 

    return F0 + (1.0f - F0) * pow(f, 5.0);
}

//노말맵에서 노말값을 가져온다.
float3 GetNormal(PBRPixelShaderInput input)
{
    float3 normalWorld = input.normal;
    float3 tangent = input.tangentWorld;
    if (useNormalMap)    //노말맵을 쓸것인가?
    {
        float3 normal = normalTex.Sample(linearWrapSampler, input.texcoord, 0.0).rgb; //mipmap - 0.0 (기본 Mip사용)
        normal = 2.0 * normal - 1.0; //[-1.0 ~ 1.0]으로 범위를 조절
        
        float3 N = normalWorld;     //노말
        float3 T = tangent;         
        //float3 T = normalize(tangent - dot(tangent, N) * N); //탄젠트

        float3 B = cross(N, T);     //바이 탄젠트
        
        float3x3 TBN = float3x3(T, B, N);
        
        normalWorld = normalize(mul(normal, TBN));
    }
    return normalWorld;
}

float3 DiffuseIBL(float3 albedo, float3 normalWorld, float3 pixelToEye,
                  float metallic)
{
    float3 F0 = lerp(Fdielectric, albedo, metallic);
    float3 F = PBRSchlickFresnel(F0, max(0.0, dot(normalWorld, pixelToEye)));
    float3 kd = lerp(1.0 - F, 0.0, metallic);
    //float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metallic);
    float3 irradiance = g_diffuseCube.Sample(linearClampSampler, normalWorld).rgb;
    
    return kd * albedo * irradiance;
}

float3 SpecularIBL(float3 albedo, float3 normalWorld, float3 pixelToEye,
                   float metallic, float roughness)
{
    float2 specularBRDF = g_BRDFTex.Sample(linearClampSampler, float2(dot(normalWorld, pixelToEye), roughness)).rg;
    
    //밉맵 -> 거칠기가 거칠수록 low밉맵을 쓴다
    float3 specularIrradiance = g_specularCube.SampleLevel(linearWrapSampler,
                                reflect(-pixelToEye, normalWorld), roughness * 11/*밉맵의 개수- 현재 큐브맵의 밉맵개수를 그냥 하드 코딩으로 박았다 후에는 contant값을 받아 사용하도 록한다.*/).rgb;

    float3 F0 = lerp(Fdielectric, albedo, metallic);

    return (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;

    //여기서 specularBRDF.x는 D*G의 값을 나타낸다. 
    // D - 미세표면들의 법선분포를 나타낸다. roughness에 따라 표면의 거칠기를 표현
    // G - 표면의 마이크로패싯(미세면)의 셰도잉과 마스킹에 따른 가리는 효과를 나타낸다.
    // 이것을 미리 계산해서 LUT로 만들어 성능 최적화가 가능하게 한다. ->샘플링만으로 바로 사용 가능하게
    //https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf ->p6을 보면 설명이 나와있다.
}

float3 AmbientLightingByIBL(float3 albedo, float3 normalW, float3 pixelToEye, float ao,
                            float metallic, float roughness)
{
    float3 diffuseIBL = DiffuseIBL(albedo, normalW, pixelToEye, metallic);
    float3 specularIBL = SpecularIBL(albedo, normalW, pixelToEye, metallic, roughness);
    
    return (diffuseIBL + specularIBL)* ao;
}
#define PI 3.141592
float NdfGGX(float NdotH, float roughness)
{
    float a = roughness * roughness; 
    float a2 = a * a;
    float b = (NdotH * NdotH) * (a2 - 1.f) + 1.f;
    
    return a2 / (PI * b * b);

}

float SchlickGsub(float NdotV, float k)
{
    return NdotV / (NdotV * (1.f - k) + k);
}

float SchlickGGX(float NdotL, float NdotV, float roughness)
{
    float a = roughness + 1.f;
    float K_direct = (a * a) / 8.0f;       ///K_IBL의 경우 (roughness^2)/2이다
    return SchlickGsub(NdotL, K_direct) * SchlickGsub(NdotV, K_direct);

}

// Shadow Pass에서 만든 depth texture와 현재 픽셀의 light-space depth를 비교한다.
// 1.0이면 빛을 직접 받고, 0.7이면 다른 물체에 가려진 영역이다.
// 현재는 기존 일반 메시 셰이더와 결과를 맞추기 위해 동일한 상수와 단일 샘플을 사용한다.
float CalculateShadowFactor(float3 positionWorld)
{
    float4 lightSpacePosition = mul(float4(positionWorld, 1.0f), lights[0].viewProj);
    lightSpacePosition.xyz /= lightSpacePosition.w;
    lightSpacePosition.y *= -1.0f;

    float2 shadowTexCoord = lightSpacePosition.xy * 0.5f + 0.5f;
    float shadowMapDepth = shadowMaps[0].Sample(shadowPointSampler, shadowTexCoord).r;
    const float shadowBias = 0.003f;

    return lightSpacePosition.z - shadowBias > shadowMapDepth ? 0.7f : 1.0f;
}

// 라이트 종류에 따라 표면에서 라이트로 향하는 벡터와 감쇠율을 계산한다.
// Directional은 위치와 무관하고, Point/Spot은 거리 범위의 영향을 받는다.
float3 EvaluatePbrLight(
    Light light,
    float3 positionWorld,
    float3 normalWorld,
    float3 pixelToEye,
    float3 albedo,
    float metallic,
    float roughness)
{
    float3 lightVector = 0.0f;
    float attenuation = 1.0f;

    if (light.lightType == LIGHT_DIRECTIONAL)
    {
        lightVector = normalize(-light.direction);
    }
    else
    {
        float3 toLight = light.position - positionWorld;
        float distanceToLight = length(toLight);
        if (distanceToLight <= 0.0001f || distanceToLight >= light.fallOffEnd)
        {
            return float3(0.0f, 0.0f, 0.0f);
        }

        lightVector = toLight / distanceToLight;
        attenuation = CalcAttenuation(distanceToLight, light.fallOffStart, light.fallOffEnd);

        if (light.lightType == LIGHT_SPOT)
        {
            float spotAmount = max(dot(normalize(light.direction), -lightVector), 0.0f);
            attenuation *= pow(spotAmount, light.spotpower);
        }
    }

    float NdotL = max(dot(normalWorld, lightVector), 0.0f);
    float NdotV = max(dot(normalWorld, pixelToEye), 0.0f);
    if (NdotL <= 0.0f || NdotV <= 0.0f)
    {
        return float3(0.0f, 0.0f, 0.0f);
    }

    float3 halfway = normalize(pixelToEye + lightVector);
    float NdotH = max(dot(normalWorld, halfway), 0.0f);
    float3 F0 = lerp(Fdielectric, albedo, metallic);
    float3 F = PBRSchlickFresnel(F0, max(dot(halfway, pixelToEye), 0.0f));
    float3 kd = (1.0f - F) * (1.0f - metallic);
    float3 diffuseBrdf = kd * albedo / PI;
    float D = NdfGGX(NdotH, roughness);
    float G = SchlickGGX(NdotL, NdotV, roughness);
    float3 specularBrdf = F * D * G / max(1e-5f, 4.0f * NdotL * NdotV);

    return (diffuseBrdf + specularBrdf) * light.lightColor * light.strength * attenuation * NdotL;
}

float4 main(PBRPixelShaderInput input) : SV_TARGET0
{
    float3 pixelToEye = normalize(eyeWorld - input.posWorld);
    float3 normalWorld = GetNormal(input);
    
    float3 albedo = useAlbedoMap ? albedoTex.Sample(linearWrapSampler, input.texcoord).rgb 
                                 : material.albedo;
    float ao = useAOMap ? aoTex.SampleLevel(linearWrapSampler, input.texcoord, 0.0).r : 1.0;
    float metallic = useMetallicMap ? metallicTex.Sample(linearWrapSampler, input.texcoord).r 
                                    : material.metallic; 
    float roughness = useRoughnessMap ? roughnessTex.Sample(linearWrapSampler, input.texcoord).r 
                                      : material.roughness;
    //GI
    float3 ambientLighting = AmbientLightingByIBL(albedo, normalWorld, pixelToEye, ao,
                                                  metallic, roughness);
    //Diffuse + Specular
    float3 directLighting = float3(0, 0, 0);
    //---------우선 directionLighting만! 후에 Point도, Spot도 추가해보자!!-----------------
    //[unroll], for문.. 어쩌고..
#if 0 // 이전 lights[0] 전용 실험 코드: Scene 라이트 반복 계산으로 교체되어 컴파일에서 제외한다.
    float3 lightVec = normalize(lights[0].position - input.posWorld);
    float3 halfway = normalize(pixelToEye + lightVec);
    float3 reflectVec = reflect(-lightVec, normalWorld);
    
    float NdotL = max(0.0, dot(normalWorld, lightVec));
    float NdotH = max(0.0, dot(normalWorld, halfway));
    float NdotV = max(0.0, dot(normalWorld, pixelToEye));
    
    float3 F0 = lerp(Fdielectric, albedo, metallic);
    //float3 F0 = lerp(Fdielectric, (0.05,0.05,0.05), metallic); //프레넬을 확인할때 요긴하다.
    //float3 F = PBRSchlickFresnel(F0, max(0.0, dot(normalWorld, pixelToEye)));
    float3 F = PBRSchlickFresnel(F0, max(0.0, dot(halfway, pixelToEye)));
    
    float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metallic);
    float3 ks = 1 - kd;
    //float3 kd = 1 - ks;
    //Diffuse BRDF를 구한다. Pi를 생략해도 별 문제가 되지 않는다. ->조명 단위와 BRDF 구현의 목적에 따라 π를 생략할 수도 있다
    //https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
    float3 diffuseBRDF = kd * albedo / PI;      
    
    
    float D = NdfGGX(NdotH, roughness);
    float G = SchlickGGX(NdotL, NdotV, roughness);
    
    //Cook_Torrence Specular BRDF를 구한다.
    float3 specularBRDF = ((F * D * G) / max(1e-5, 4.0 * NdotL * NdotV)) * ks;
    float3 radiance = lights[0].strength * saturate((lights[0].fallOffEnd - length(lightVec))
                    / (lights[0].fallOffEnd - lights[0].fallOffStart));
    
    directLighting += (diffuseBRDF + specularBRDF) * 1 * NdotL;
#endif
    //------------------여기까지 for문 끝-------------------------------------------
    
    // 환경광(IBL)은 그림자 안에서도 남겨두고, 광원에서 직접 오는 빛에만 shadow를 적용한다.
    // Deferred 오브젝트는 Lighting Pass가 완성되기 전까지 Forward PBR로도 한 번 그려지므로
    // 현재 단계에서는 Forward/Deferred로 만든 PBR 오브젝트 모두 이 결과를 사용한다.
    // 위쪽의 과거 단일 라이트 계산 결과를 버리고 Scene에서 수집한 실제 라이트를 순회한다.
    directLighting = float3(0.0f, 0.0f, 0.0f);
    [loop]
    for (uint lightIndex = 0; lightIndex < min(lightNum, (uint)MAX_LIGHTS); ++lightIndex)
    {
        directLighting += EvaluatePbrLight(
            lights[lightIndex], input.posWorld, normalWorld, pixelToEye,
            albedo, metallic, roughness);
    }

    float shadowFactor = lightNum > 0 ? CalculateShadowFactor(input.posWorld) : 1.0f;
    float4 finalColor = float4(ambientLighting + directLighting * shadowFactor, 1);
    finalColor = clamp(finalColor, 0.0, 1000.f);
   
    return float4(finalColor);
   
    //Fresnel이 보고싶을때
    
    
    //return float4(directLighting,1.0f);
}
    
