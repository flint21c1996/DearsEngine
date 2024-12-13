#include "Common.hlsli"

Texture2D albedoTex : register(t1);
Texture2D normalTex : register(t2);
Texture2D aoTex : register(t3);
Texture2D metallicTex : register(t4);
Texture2D roughnessTex : register(t5);

cbuffer PixelConstantBuffer : register(b1)
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
#define Fdielectric  0.04  // 비금속(Dielectric) 재질의 F0
//사실 common.hlsli에 있는 함수랑 똑같은 함수다..
float3 PBRSchlickFresnel(float3 F0, float NdotH)
{
    float f = 1.0f - NdotH;
    return F0 + (1.0f - F0) * pow(f, 5.0);
}

//노말맵에서 노말값을 가져온다.
float GetNormal(PBRPixelShaderInput input)
{
    float3 normalWorld = input.normal;
    float3 tangent = input.tangentWorld;
    
    if (useNormalMap)    //노말맵을 쓸것인가?
    {
        float3 normal = normalTex.Sample(linearWrapSampler, input.texcoord, 0.0).rgb; //mipmap - 0.0 (기본 Mip사용)
        normal = 2.0 * normal - 1.0; //[-1.0 ~ 1.0]으로 범위를 조절
        
        float3 N = normalWorld;     //노말
        float3 T = tangent;         //탄젠트
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
    
    // 앞에서 사용했던 방법과 동일
    // float3 irradiance = ... TODO
    
    return kd * albedo;
}

float3 SpecularIBL(float3 albedo, float3 normalWorld, float3 pixelToEye,
                   float metallic, float roughness)
{
    // TODO: 슬라이드 Environment BRDF
    // float2 specularBRDF = brdfTex.Sample(clampSampler, float2(... , ...)).rg;
    
    // 앞에서 사용했던 방법과 동일
    // float3 specularIrradiance = specularIBLTex.SampleLevel(linearSampler, TODO, roughness * 10.0f).rgb;
    float3 F0 = lerp(Fdielectric, albedo, metallic);

    return float3(1, 1, 1);

}

float3 AmbientLightingByIBL(float3 albedo, float3 normalW, float3 pixelToEye, float ao,
                            float metallic, float roughness)
{
    float3 diffuseIBL = DiffuseIBL(albedo, normalW, pixelToEye, metallic);
    float3 specularIBL = SpecularIBL(albedo, normalW, pixelToEye, metallic, roughness);
    
    return (diffuseIBL + specularIBL) * ao;
}

float NdfGGX(float NdotH, float roughness)
{
    // TODO: 방정식 (3)
    return 1.0;
}

// TODO: 방정식 (4)
float SchlickGGX(float NdotI, float NdotO, float roughness)
{
    return 1.0;
}

float4 main(PBRPixelShaderInput input) : SV_TARGET0
{
    float pixelToEye = normalize(eyeWorld - input.posWorld);
    float3 normalWorld = GetNormal(input);
    
    float albedo = useAlbedoMap ? albedoTex.Sample(linearWrapSampler, input.texcoord).rgb 
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
    float lightVec = lights[0].position - input.posWorld;
    float halfway = normalize(pixelToEye + lightVec);
    
    float NdotL = max(0.0, dot(normalWorld, lightVec));
    float NdotH = max(0.0, dot(normalWorld, halfway));
    float NdotV = max(0.0, dot(normalWorld, pixelToEye));
    
    float3 F0 = lerp(Fdielectric, albedo, metallic);
    float3 F = PBRSchlickFresnel(F0, max(0.0, dot(halfway, pixelToEye)));
    float3 kd = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metallic);
    
    //Diffuse BRDF를 구한다.
    float3 diffuseBRDF = kd * albedo;
    
    float D = NdfGGX(NdotH, roughness);
    float3 G = SchlickGGX(NdotL, NdotV, roughness);
    
    //Cook_Torrence Specular BRDF를 구한다.
    float3 specularBRDF = (F * D * G) / max(1e-5, 4.0 * NdotL * NdotV);
    float3 radiance = lights[0].strength * saturate((lights[0].fallOffEnd - length(lightVec))
                    / (lights[0].fallOffEnd - lights[0].fallOffStart));
    
    directLighting += (diffuseBRDF + specularBRDF) * radiance * NdotL;
    //------------------여기까지 for문 끝-------------------------------------------
    
    float4 finalColor = ((ambientLighting + directLighting), 1.0f);
    finalColor = clamp(finalColor, 0.0, 1000.f);
    return finalColor;
}
    