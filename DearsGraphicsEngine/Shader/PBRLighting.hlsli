#ifndef DEARS_PBR_LIGHTING_HLSLI
#define DEARS_PBR_LIGHTING_HLSLI

// 이 파일은 Forward PBR과 Deferred Lighting이 함께 사용하는 조명 계산 함수 모음이다.
//
// 같은 PBR 수식을 두 Pixel Shader에 복사해두면 한쪽만 수정했을 때 화면 결과가 달라진다.
// 따라서 "표면 정보가 어디에서 왔는가"는 각 Pixel Shader가 결정하고,
// "그 표면에 빛을 어떻게 계산하는가"는 이 파일에서 한 번만 정의한다.
//
// 주의: 이 파일보다 먼저 Common.hlsli를 include해야 한다.
// Light, 공통 Cube Map, Shadow Map, 샘플러와 CalculateShadowFactor()를 사용하기 때문이다.

#define LIGHT_DIRECTIONAL 1
#define LIGHT_SPOT 3

static const float PI = 3.141592f;
static const float3 Fdielectric = 0.04f;

// Schlick 근사로 시선 각도에 따른 프레넬 반사율을 계산한다.
// 정면에서는 재질의 F0에 가깝고, 표면을 비스듬하게 볼수록 1에 가까워진다.
float3 PBRSchlickFresnel(float3 F0, float normalDotView)
{
    float oneMinusCosine = 1.0f - saturate(normalDotView);
    return F0 + (1.0f - F0) * pow(oneMinusCosine, 5.0f);
}

// 미리 컨볼루션한 Diffuse Irradiance Cube Map에서 간접 확산광을 읽는다.
float3 DiffuseIBL(
    float3 albedo,
    float3 normalWorld,
    float3 pixelToEye,
    float metallic)
{
    float3 F0 = lerp(Fdielectric, albedo, metallic);
    float3 fresnel = PBRSchlickFresnel(F0, dot(normalWorld, pixelToEye));
    float3 diffuseRatio = (1.0f - fresnel) * (1.0f - metallic);
    float3 irradiance = g_diffuseCube.Sample(linearClampSampler, normalWorld).rgb;
    return diffuseRatio * albedo * irradiance;
}

// Prefiltered Environment Map과 BRDF LUT를 조합해 간접 정반사광을 계산한다.
float3 SpecularIBL(
    float3 albedo,
    float3 normalWorld,
    float3 pixelToEye,
    float metallic,
    float roughness)
{
    float normalDotView = saturate(dot(normalWorld, pixelToEye));
    float2 integratedBrdf = g_BRDFTex.Sample(
        linearClampSampler,
        float2(normalDotView, roughness)).rg;

    // 현재 Specular Cube Map의 mip 개수가 12개라서 최대 LOD를 11로 사용한다.
    // 추후에는 Cube Map 리소스 정보에서 최대 LOD를 받아 상수 버퍼로 전달해야 한다.
    float3 reflection = reflect(-pixelToEye, normalWorld);
    float3 specularIrradiance = g_specularCube.SampleLevel(
        linearWrapSampler,
        reflection,
        roughness * 11.0f).rgb;

    float3 F0 = lerp(Fdielectric, albedo, metallic);
    return (F0 * integratedBrdf.x + integratedBrdf.y) * specularIrradiance;
}

// IBL은 특정 라이트 한 개의 직접광이 아니므로 Shadow Map으로 완전히 지우지 않는다.
// AO는 간접광이 닿기 어려운 틈을 어둡게 만드는 값이므로 IBL 결과에 적용한다.
float3 AmbientLightingByIBL(
    float3 albedo,
    float3 normalWorld,
    float3 pixelToEye,
    float ao,
    float metallic,
    float roughness)
{
    float3 diffuse = DiffuseIBL(albedo, normalWorld, pixelToEye, metallic);
    float3 specular = SpecularIBL(
        albedo, normalWorld, pixelToEye, metallic, roughness);
    return (diffuse + specular) * ao;
}

// GGX/Trowbridge-Reitz Normal Distribution Function이다.
// Roughness가 작을수록 반사 에너지가 좁은 영역에 강하게 모인다.
float NdfGGX(float normalDotHalf, float roughness)
{
    float alpha = roughness * roughness;
    float alphaSquared = alpha * alpha;
    float denominator =
        normalDotHalf * normalDotHalf * (alphaSquared - 1.0f) + 1.0f;
    return alphaSquared / max(PI * denominator * denominator, 0.00001f);
}

float SchlickGeometrySub(float cosine, float k)
{
    return cosine / max(cosine * (1.0f - k) + k, 0.00001f);
}

// 시선과 빛 방향에서 미세면이 가려지는 Geometry/Visibility 항을 계산한다.
float SchlickGGX(float normalDotLight, float normalDotView, float roughness)
{
    float alpha = roughness + 1.0f;
    float kDirect = alpha * alpha / 8.0f;
    return SchlickGeometrySub(normalDotLight, kDirect) *
        SchlickGeometrySub(normalDotView, kDirect);
}

// 라이트 한 개가 현재 표면에 만드는 직접광을 계산한다.
// Directional은 방향만 사용하고, Point/Spot은 거리 감쇠와 유효 범위를 함께 계산한다.
float3 EvaluatePbrLight(
    Light light,
    float3 positionWorld,
    float3 normalWorld,
    float3 pixelToEye,
    float3 albedo,
    float metallic,
    float roughness)
{
    float3 lightVector = float3(0.0f, 0.0f, 0.0f);
    float attenuation = 1.0f;
    bool isInsideLight = true;

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
            isInsideLight = false;
        }
        else
        {
            lightVector = toLight / distanceToLight;
            attenuation = CalcAttenuation(
                distanceToLight, light.fallOffStart, light.fallOffEnd);

            if (light.lightType == LIGHT_SPOT)
            {
                float spotCosine = max(
                    dot(normalize(light.direction), -lightVector), 0.0f);
                float outerCosine = cos(
                    radians(clamp(light.shadowFovY, 1.0f, 179.0f)) * 0.5f);
                if (spotCosine <= outerCosine)
                {
                    isInsideLight = false;
                }
                else
                {
                    attenuation *= pow(
                        saturate(spotCosine),
                        max(light.spotpower, 1.0f));
                }
            }
        }
    }

    // 유효 범위를 벗어난 경우에는 lightVector를 사용하는 BRDF 계산 자체를 건너뛴다.
    // 이렇게 분기를 감싸면 fxc도 모든 사용 경로에서 값이 초기화되었음을 명확히 알 수 있다.
    float3 result = float3(0.0f, 0.0f, 0.0f);
    if (isInsideLight)
    {
        float normalDotLight = max(dot(normalWorld, lightVector), 0.0f);
        float normalDotView = max(dot(normalWorld, pixelToEye), 0.0f);
        if (normalDotLight > 0.0f && normalDotView > 0.0f)
        {
            float3 halfway = normalize(pixelToEye + lightVector);
            float normalDotHalf = max(dot(normalWorld, halfway), 0.0f);
            float viewDotHalf = max(dot(pixelToEye, halfway), 0.0f);

            float3 F0 = lerp(Fdielectric, albedo, metallic);
            float3 fresnel = PBRSchlickFresnel(F0, viewDotHalf);
            float3 diffuseRatio = (1.0f - fresnel) * (1.0f - metallic);
            float3 diffuseBrdf = diffuseRatio * albedo / PI;

            float distribution = NdfGGX(normalDotHalf, roughness);
            float geometry = SchlickGGX(normalDotLight, normalDotView, roughness);
            float3 specularBrdf = fresnel * distribution * geometry /
                max(4.0f * normalDotLight * normalDotView, 0.00001f);

            result = (diffuseBrdf + specularBrdf) *
                light.lightColor * light.strength * attenuation * normalDotLight;
        }
    }
    return result;
}

// Scene의 모든 라이트와 각 라이트의 Shadow Map을 합산한다.
// Forward는 정점 셰이더에서 받은 표면을, Deferred는 G-Buffer에서 복원한 표면을 넘기지만
// 이 함수 이후의 조명 계산은 두 경로가 완전히 동일하다.
float3 EvaluateScenePbrLighting(
    float3 positionWorld,
    float3 normalWorld,
    float3 pixelToEye,
    float3 albedo,
    float metallic,
    float roughness,
    float ao)
{
    float3 result = AmbientLightingByIBL(
        albedo, normalWorld, pixelToEye, ao, metallic, roughness);

    [loop]
    for (uint lightIndex = 0; lightIndex < min(lightNum, (uint)MAX_LIGHTS); ++lightIndex)
    {
        float3 directLight = EvaluatePbrLight(
            lights[lightIndex],
            positionWorld,
            normalWorld,
            pixelToEye,
            albedo,
            metallic,
            roughness);

        // Shadow는 해당 라이트에서 직접 도달하는 빛에만 적용한다.
        // IBL까지 0으로 만들면 그림자 안에서 주변 환경광도 사라져 지나치게 검게 보인다.
        directLight *= CalculateShadowFactor(
            lights[lightIndex], lightIndex, positionWorld, normalWorld);
        result += directLight;
    }
    return result;
}

#endif
