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
    
    
    // 월드 좌표에서 라이트 뷰-투영 좌표로 변환
   float4 lightSpacePos = mul(float4(input.posWorld, 1.0f), lights[0].viewProj);

   // 클립 공간 좌표에서 NDC 좌표로 변환
   lightSpacePos.xyz /= lightSpacePos.w;
    lightSpacePos.y *= -1;
       
   // 텍스처 좌표로 변환 (0, 1 범위)
   float2 shadowTexCoord = lightSpacePos.xy * 0.5f + 0.5f;
  
   // 그림자 맵에서 깊이 값을 샘플링 (현재 좌표의 깊이)
   float shadowMapDepth = shadowMaps[0].Sample(shadowPointSampler, shadowTexCoord).r;

   // 그림자 맵에서의 z 값과 비교
   float bias = 0.003f; // 그림자 경계를 부드럽게 하기 위한 바이어스
   float shadowFactor = (lightSpacePos.z - bias > shadowMapDepth) ? 0.7f : 1.0f;

   // 빛의 색과 그림자 효과 적용
   //light.lightColor * shadowFactor;
    
    
    
   float4 diffuse = g_diffuseCube.Sample(linearWrapSampler, input.normal);
   float4 specular = g_specularCube.Sample(linearWrapSampler, reflect(-toEye, input.normal));
    
   diffuse *= float4(material.diffuse, 1.0); //RGB채널끼리 다르게 할 수도 있다.
 
   //다양한 재질의 반사 특성을 더 현실적으로 표현하기위해
   specular *= pow((specular.x + specular.y + specular.z) / 3.0, material.shininess);
   specular *= float4(material.specular, 1.0);
    
   float3 f = SchlickFresnel(material.fresnel, input.normal, toEye);
   specular.xyz *= f;


   //if (useTexture) /후에 추가할 것.
   diffuse *= g_texture0.Sample(linearWrapSampler, input.texcoord);
   //specular *= float4(1.0,1.0,1.0,1.0);  /후에 specular texture가 있을경우
    
   //고전 IBL - 조명 연산은 안들어가 있다.
   //return (diffuse + specular) * shadowFactor;
   //return (input.normal, 1);
    
   ///이쁘게 보이게 하기위해 노력한 값.
   float4 finalcolor = (color, 0) + (diffuse + specular*0.1f);
   finalcolor.xyz *= shadowFactor;
   return finalcolor;

   // + 조명 연산까지 추가해서 만들어봄
   //return float4(color, 1.0) * (diffuse + specular)* shadowFactor;
    
   //specularCube의 이미지 값에 맞춰 픽셀 색상만 결정하게 한다.
   //return g_specularCube.Sample(g_sampler, reflect(-toEye, input.normal));
    
   //그냥 렌더 (+조명)
   //return float4(color, 1.0) * g_texture0.Sample(linearWrapSampler, input.texcoord) * shadowFactor;
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
