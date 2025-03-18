#include "Common.hlsli"

Texture2D albedoTex : register(t1);
Texture2D normalTex : register(t2);
Texture2D aoTex : register(t3);
Texture2D metallicTex : register(t4);
Texture2D roughnessTex : register(t5);

cbuffer ThinFilmConstantBuffer : register(b3)
{
	//n1 - ����1�� ������ 
    float n1;
    //n2 - ����2�� ������
    float n2;
    //�ð�
    float time;
    //���� �β�
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

static const float3 Fdielectric = (0.04, 0.04, 0.04); // ��ݼ�(Dielectric) ������ F0

//�븻�ʿ��� �븻���� �����´�.
float3 GetNormal(PBRPixelShaderInput input)
{
    float3 normalWorld = input.normal;
    float3 tangent = input.tangentWorld;
    if (useNormalMap)    //�븻���� �����ΰ�?
    {
        float3 normal = normalTex.Sample(linearWrapSampler, input.texcoord, 0.0).rgb; //mipmap - 0.0 (�⺻ Mip���)
        normal = 2.0 * normal - 1.0; //[-1.0 ~ 1.0]���� ������ ����
        
        float3 N = normalWorld; //�븻
        float3 T = tangent;
        float3 B = cross(N, T); //���� ź��Ʈ
        
        float3x3 TBN = float3x3(T, B, N);
        
        normalWorld = normalize(mul(normal, TBN));
    }
    return normalWorld;
}

float3 PBRSchlickFresnel(float3 F0, float NdotV)
{
    float f = 1.0f - NdotV;
    // 90���� ��� 1
    // 0���� ��� 0
    //��, normal�� view���Ͱ� 90�� ������ -> �����ڸ���. 
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
    
    //�Ӹ� -> ��ĥ�Ⱑ ��ĥ���� low�Ӹ��� ����
    float3 specularIrradiance = g_specularCube.SampleLevel(linearWrapSampler,
                                reflect(-pixelToEye, normalWorld), roughness * 11 /*�Ӹ��� ����- ���� ť����� �Ӹʰ����� �׳� �ϵ� �ڵ����� �ھҴ� �Ŀ��� contant���� �޾� ����ϵ� ���Ѵ�.*/).rgb;

    float3 F0 = lerp(Fdielectric, albedo, metallic);

    return (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;

    //���⼭ specularBRDF.x�� D*G�� ���� ��Ÿ����. 
    // D - �̼�ǥ����� ���������� ��Ÿ����. roughness�� ���� ǥ���� ��ĥ�⸦ ǥ��
    // G - ǥ���� ����ũ���н�(�̼���)�� �ε��װ� ����ŷ�� ���� ������ ȿ���� ��Ÿ����.
    // �̰��� �̸� ����ؼ� LUT�� ����� ���� ����ȭ�� �����ϰ� �Ѵ�. ->���ø������� �ٷ� ��� �����ϰ�
    //https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf ->p6�� ���� ������ �����ִ�.
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
    //�Ի簢 ��� //refract�Լ� ���ο��� �̷������� �۵��Ѵ�.
    //float costheta1 = NdotV;
    //float sintheta1 = sqrt(1 - pow(costheta1, 2));
    //float sintheta2 = (n1 / n2) * sintheta1;
    //float costheta2 = sqrt(1 - pow(sintheta2, 2));
    //float3 finalvec = (n1 / n2 * costheta1 + (n1 / n2 * costheta1 - costheta2)) * normalWorld * d;
    
    //���Ƿ� ���� �ð����
    float t = time;
    
    float d = distance;
  
    // �β�����
    d *=lerp(0.1, 1, t);
    d += (albedo * pow(1 - NdotV, 5));
    
    // �ݻ� �� ���� ���� ���
    float3 reflectVec = normalize(reflect(-viewDir, normalWorld));
    float3 refractVec = normalize(refract(-viewDir, normalWorld, n1 / n2));
    float3 reflectVec1 = normalize(reflect(refractVec, normalWorld));
    float3 refractVec1 = normalize(refract(reflectVec1, normalWorld, n2 / n1));
    
    //�⺻ �ݻ��� ����
    float R0 = pow((n1 - n2) / (n1 + n2), 2.0);
    
    //������ ����
    float fresnelFactor = R0 + (1.0 - R0) * pow(1.0 - abs(dot(normalWorld, viewDir)), 3.0);
    
    // ȯ�� �ʿ��� �� ���ø�
    float3 color = g_specularCube.SampleLevel(linearWrapSampler, reflectVec, 0).rgb;
    float3 color1 = g_specularCube.SampleLevel(linearWrapSampler, refractVec1, 0).rgb;
    float3 color2 = g_specularCube.SampleLevel(linearWrapSampler, reflectVec1, 0).rgb;
   
    //-----------------------------------
    // ���� ������ �̷�� ���� ���
    float cosTheta = abs(dot(refractVec, normalWorld));

    // OPD(���� ��� ����) ���
    float OPD = 2.0 * n2 * d * cosTheta *1000;

    // OPD�� Ư���� ����� ���Ͽ� RGB �� ����
    float R = sin(2.0 * 3.1415 * OPD / 650.0); // ���� (�� = 650nm)
    float G = sin(2.0 * 3.1415 * OPD / 510.0); // �ʷ� (�� = 510nm)
    float B = sin(2.0 * 3.1415 * OPD / 475.0); // �Ķ� (�� = 475nm)

    // ���밪 �����Ͽ� ���� �� ����
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