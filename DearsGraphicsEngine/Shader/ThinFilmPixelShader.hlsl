#include "Common.hlsli"

Texture2D albedoTex : register(t1);
Texture2D normalTex : register(t2);
Texture2D aoTex : register(t3);
Texture2D metallicTex : register(t4);
Texture2D roughnessTex : register(t5);

cbuffer ThinFilmConstantBuffer : register(b3)
{
	//n1 - 매질1의 굴절율 
    float n1;
    //n2 - 매질2의 굴절율
    float n2;
    //시간
    float time;
    //시작 두께
    int d;
    
    float maxLights;
    int useAlbedoMap;
    int useNormalMap;
    int useAOMap; // Ambient Occlusion
    
    int useMetallicMap;
    int useRoughnessMap;
    int dummy1;
    int dummy2;              //16
    
    PBRMaterial material;
}

static const float3 Fdielectric = (0.04, 0.04, 0.04); // 비금속(Dielectric) 재질의 F0

//노말맵에서 노말값을 가져온다.
float3 GetNormal(PBRPixelShaderInput input)
{
    float3 normalWorld = input.normal;
    float3 tangent = input.tangentWorld;
    if (useNormalMap)    //노말맵을 쓸것인가?
    {
        float3 normal = normalTex.Sample(linearWrapSampler, input.texcoord, 0.0).rgb; //mipmap - 0.0 (기본 Mip사용)
        normal = 2.0 * normal - 1.0; //[-1.0 ~ 1.0]으로 범위를 조절
        
        float3 N = normalWorld; //노말
        float3 T = tangent;
        float3 B = cross(N, T); //바이 탄젠트
        
        float3x3 TBN = float3x3(T, B, N);
        
        normalWorld = normalize(mul(normal, TBN));
    }
    return normalWorld;
}

float3 PBRSchlickFresnel(float3 F0, float NdotV)
{
    float f = 1.0f - NdotV;
    // 90도일 경우 1
    // 0도일 경우 0
    //즉, normal과 view벡터가 90에 가깝다 -> 가장자리다. 
    return F0 + (1.0f - F0) * pow(f, 5.0);
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
                                reflect(-pixelToEye, normalWorld), roughness * 11 /*밉맵의 개수- 현재 큐브맵의 밉맵개수를 그냥 하드 코딩으로 박았다 후에는 contant값을 받아 사용하도 록한다.*/).rgb;

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
    
    return (diffuseIBL + specularIBL) * ao;
}

float3 GetThinFilmVector(float n1, float n2, float3 viewDir, float3 normalWorld, float distance, float time, float3 albedo, float NdotV)
{
    //입사각 계산 //refract함수 내부에서 이런식으로 작동한다.
    //float costheta1 = NdotV;
    //float sintheta1 = sqrt(1 - pow(costheta1, 2));
    //float sintheta2 = (n1 / n2) * sintheta1;
    //float costheta2 = sqrt(1 - pow(sintheta2, 2));
    //float3 finalvec = (n1 / n2 * costheta1 + (n1 / n2 * costheta1 - costheta2)) * normalWorld * d;
    
    //임의로 정한 시간상수
    float t = time;
    
    float d = distance;
  
    // 두께변수
    d *=lerp(0.1, 1, t);
    d += (albedo * pow(1 - NdotV, 5));
    
    // 반사 및 굴절 벡터 계산
    float3 reflectVec = normalize(reflect(-viewDir, normalWorld));
    float3 refractVec = normalize(refract(-viewDir, normalWorld, n1 / n2));
    float3 reflectVec1 = normalize(reflect(refractVec, normalWorld));
    float3 refractVec1 = normalize(refract(reflectVec1, normalWorld, n2 / n1));
    
    //기본 반사율 공식
    float R0 = pow((n1 - n2) / (n1 + n2), 2.0);
    
    //프레넬 공식
    float fresnelFactor = R0 + (1.0 - R0) * pow(1.0 - abs(dot(normalWorld, viewDir)), 3.0);
    
    // 환경 맵에서 빛 샘플링
    float3 color = g_specularCube.SampleLevel(linearWrapSampler, reflectVec, 0).rgb;
    float3 color1 = g_specularCube.SampleLevel(linearWrapSampler, refractVec1, 0).rgb;
    float3 color2 = g_specularCube.SampleLevel(linearWrapSampler, reflectVec1, 0).rgb;
   
    //-----------------------------------
    // 빛이 법선과 이루는 각도 계산
    float cosTheta = abs(dot(refractVec, normalWorld));

    // OPD(광학 경로 차이) 계산
    float OPD = 2.0 * n2 * d * cosTheta *1000;

    // OPD를 특정한 파장과 비교하여 RGB 값 생성
    float R = sin(2.0 * 3.1415 * OPD / 650.0); // 빨강 (λ = 650nm)
    float G = sin(2.0 * 3.1415 * OPD / 510.0); // 초록 (λ = 510nm)
    float B = sin(2.0 * 3.1415 * OPD / 475.0); // 파랑 (λ = 475nm)

    // 절대값 적용하여 음수 값 방지
    float3 interferenceColor = abs(float3(R, G, B));
    
   return (color * fresnelFactor + interferenceColor * (color * (1 - fresnelFactor) * (fresnelFactor) 
                                                     + (color1 * (1 - fresnelFactor) * (1-fresnelFactor))));
    
    
}

float4 main(PBRPixelShaderInput input) : SV_TARGET
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
    
    float3 lightVec = normalize(lights[0].position - input.posWorld);
    
    float3 halfway = normalize(pixelToEye + lightVec);
    
    //float3 reflectVec = reflect(-lightVec, normalWorld);
    
    float NdotL = max(0.0, dot(normalWorld, lightVec));
    float NdotH = max(0.0, dot(normalWorld, halfway));
    float NdotV = max(0.0, dot(normalWorld, pixelToEye));
    
    float3 GIFinalcolor = GetThinFilmVector(n1, n2, pixelToEye, normalWorld, d, time, albedo, NdotV);
    
    return float4(GIFinalcolor, 0.5);
}