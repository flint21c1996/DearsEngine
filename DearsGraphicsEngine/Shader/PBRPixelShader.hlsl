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
        //float3 T = tangent;         
        float3 T = normalize(tangent - dot(tangent, N) * N); //탄젠트

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
    float2 specularBRDF = g_BRDFTex.Sample(linearClampSampler, float2(dot(normalWorld, pixelToEye), 1.f - roughness)).rg;
    
    //밉맵 -> 거칠기가 거칠수록 low밉맵을 쓴다
    float3 specularIrradiance = g_specularCube.SampleLevel(linearWrapSampler,
                                reflect(-pixelToEye, normalWorld), roughness * 11/*밉맵의 개수- 현재 큐브맵의 밉맵개수를 그냥 하드 코딩으로 박았다 후에는 contant값을 받아 사용하도 록한다.*/).rgb;

    float3 F0 = lerp(Fdielectric, albedo, metallic);

    return (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;

}

float3 AmbientLightingByIBL(float3 albedo, float3 normalW, float3 pixelToEye, float ao,
                            float metallic, float roughness)
{
    float3 diffuseIBL = DiffuseIBL(albedo, normalW, pixelToEye, metallic);
    float3 specularIBL = SpecularIBL(albedo, normalW, pixelToEye, metallic, roughness);
    
    return (diffuseIBL + specularIBL) * ao;
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
    float3 lightVec = normalize(lights[0].position - input.posWorld);
    float3 halfway = normalize(pixelToEye + lightVec);
    float3 reflectVec = reflect(-lightVec, normalWorld);
    
    float NdotL = max(0.0, dot(normalWorld, lightVec));
    float NdotH = max(0.0, dot(normalWorld, halfway));
    float NdotV = max(0.0, dot(normalWorld, pixelToEye));
    
    float3 F0 = lerp(Fdielectric, albedo, metallic);
    //float3 F0 = lerp(Fdielectric, (0,0,0), metallic); //프레넬을 확인할때 요긴하다.
    float3 F = PBRSchlickFresnel(F0, max(0.0, dot(normalWorld, pixelToEye)));
    
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
    //------------------여기까지 for문 끝-------------------------------------------
    
    float4 finalColor = float4((ambientLighting), 1.0f);
    finalColor = clamp(finalColor, 0.0, 1000.f);
   
    return float4(finalColor);
    return float4(F, 1.0);
    //return float4(directLighting,1.0f);
}
    