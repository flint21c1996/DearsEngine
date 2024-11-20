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
     // ���� ��ǥ���� ����Ʈ ��-���� ��ǥ�� ��ȯ
    float4 lightSpacePos = mul(float4(posWorld, 1.0f), light.viewProj);

    // Ŭ�� ���� ��ǥ���� NDC ��ǥ�� ��ȯ
    lightSpacePos.xy /= float2(1920.0f, 1080.0f);

    //float2 screenPos = input.pos.xy / float2(1920.0f, 1080.0f);
    
    // �ؽ�ó ��ǥ�� ��ȯ (0, 1 ����)
    float2 shadowTexCoord = lightSpacePos.xy * 0.5f + 0.5f;

    // �׸��� �ʿ��� ���� ���� ���ø� (���� ��ǥ�� ����)
    float shadowMapDepth = shadowMap.Sample(shadowPointSampler, shadowTexCoord).r;

    // �׸��� �ʿ����� z ���� ��
    float bias = 0.005f; // �׸��� ��踦 �ε巴�� �ϱ� ���� ���̾
    float shadowFactor = (lightSpacePos.z - bias > shadowMapDepth) ? 0.0f : 1.0f;

    // ���� ���� �׸��� ȿ�� ����
    return light.lightColor * shadowFactor;
}




//
//
//
float4 main(PixelShaderInput input) : SV_TARGET0
{
   float3 normalWorld = input.normal;
    
   float3 toEye = normalize(eyeWorld - input.posWorld);

   float3 color = float3(0, 0, 0);

   int i = 0;

   float3 lightColor = 1.0f;
   [unroll]
//  for (int i = 0; i < maxLights; i++)
//   {
//       if (lights[i].lightType == 1)
       {
           color += ComputeDirectionalLight(lights[0], material, input.normal, toEye);
       }
//        else if (lights[i].lightType == 2)
//        {
//            color += ComputePointLight(lights[i], material, input.posWorld, input.normal, toEye);
//        }
//        else if (lights[i].lightType == 3)
//        {
//            color += ComputeSpotLight(lights[i], material, input.posWorld, input.normal, toEye);
//        }
//   }


   //lightColor = LightRadiance(lights[0], input.posWorld, shadowMaps[0]);
    
    
    // ���� ��ǥ���� ����Ʈ ��-���� ��ǥ�� ��ȯ
   float4 lightSpacePos = mul(float4(input.posWorld, 1.0f), lights[0].viewProj);

   // Ŭ�� ���� ��ǥ���� NDC ��ǥ�� ��ȯ
   lightSpacePos.xyz /= lightSpacePos.w;
    lightSpacePos.y *= -1;
       
   // �ؽ�ó ��ǥ�� ��ȯ (0, 1 ����)
   float2 shadowTexCoord = lightSpacePos.xy * 0.5f + 0.5f;
  
   // �׸��� �ʿ��� ���� ���� ���ø� (���� ��ǥ�� ����)
   float shadowMapDepth = shadowMaps[0].Sample(shadowPointSampler, shadowTexCoord).r;

   // �׸��� �ʿ����� z ���� ��
   float bias = 0.003f; // �׸��� ��踦 �ε巴�� �ϱ� ���� ���̾
   float shadowFactor = (lightSpacePos.z - bias > shadowMapDepth) ? 0.7f : 1.0f;

   // ���� ���� �׸��� ȿ�� ����
   //light.lightColor * shadowFactor;
    
    
    
   float4 diffuse = g_diffuseCube.Sample(linearWrapSampler, input.normal);
   float4 specular = g_specularCube.Sample(linearWrapSampler, reflect(-toEye, input.normal));
    
   diffuse *= float4(material.diffuse, 1.0); //RGBä�γ��� �ٸ��� �� ���� �ִ�.
 
   //�پ��� ������ �ݻ� Ư���� �� ���������� ǥ���ϱ�����
   specular *= pow((specular.x + specular.y + specular.z) / 3.0, material.shininess);
   specular *= float4(material.specular, 1.0);
    
   float3 f = SchlickFresnel(material.fresnel, input.normal, toEye);
   specular.xyz *= f;


   //if (useTexture) /�Ŀ� �߰��� ��.
   diffuse *= g_texture0.Sample(linearWrapSampler, input.texcoord);
   //specular *= float4(1.0,1.0,1.0,1.0);  /�Ŀ� specular texture�� �������
    
   //���� IBL - ���� ������ �ȵ� �ִ�.
   //return (diffuse + specular) * shadowFactor;
   //return (input.normal, 1);
    
   ///�̻ڰ� ���̰� �ϱ����� ����� ��.
   float4 finalcolor = (color, 0) + (diffuse + specular*0.1f);
   finalcolor.xyz *= shadowFactor;
   return finalcolor;

   // + ���� ������� �߰��ؼ� ����
   //return float4(color, 1.0) * (diffuse + specular)* shadowFactor;
    
   //specularCube�� �̹��� ���� ���� �ȼ� ���� �����ϰ� �Ѵ�.
   //return g_specularCube.Sample(g_sampler, reflect(-toEye, input.normal));
    
   //�׳� ���� (+����)
   //return float4(color, 1.0) * g_texture0.Sample(linearWrapSampler, input.texcoord) * shadowFactor;
}

// /Ȥ�� ���� RT0�� ���̸��� ����������.
// ���� ���� ����ȭ�ϴ� �Լ�
// float LinearizeDepth(float depth, float nearPlane, float farPlane)
// {
//     return (2.0 * nearPlane) / (farPlane + nearPlane - depth * (farPlane - nearPlane));
// }
// 
// float4 main(PixelShaderInput input) : SV_TARGET
// {
//     float2 screenPos = input.pos.xy / float2(1920.0f, 1080.0f); // �Է� ��ǥ�� �ػ󵵷� ������ -> NDC ��ǥ�� ��ȯ
//      
//     float4 texcoord = mul(float4(input.posWorld, 1.0f), viewproj);
//     float2 screen = texcoord.xy / texcoord.w;                
//     screen.y *= -1;
//     screen = screen *0.5f +0.5f;
// 
//     // ���� ���� �ؽ�ó ��ǥ�� �̿��� ���ø�
//     //float depthValue = shadowMaps[0].Sample(shadowPointSampler, screenPos).r;
//     float depthValue = shadowMaps[0].Sample(shadowPointSampler, screen).r;
//     
//     //������ ��ȭ�� �ѷ��� �ϱ� ���ؼ�.
//     depthValue = LinearizeDepth(depthValue, 0.1, 100);
// 
//     // ���� ���� �׷��̽����Ϸ� ��ȯ (0.0 ~ 1.0)
//     return float4(depthValue, depthValue, depthValue, 1.0f); // RGB�� ������ ���� �־� �׷��̽����Ϸ� ���
// }
