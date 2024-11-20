#include "Common.hlsli"

Texture2D g_texture0 : register(t0);

cbuffer PixelConstantBuffer : register(b0)
{
    float maxLights;
    float3 dummy;
    Material material;
};

float4 main(PixelShaderInput input) : SV_TARGET0
{
   float4 color =  g_texture0.Sample(linearWrapSampler, input.texcoord);

/// 원래는 [255, 0, 255](핫핑크)색을 통해 해당되는 색을 지우려했으나 자꾸 외곽선쪽에서(누끼) 자꾸 핑크색이 발현되어 그냥 투명색으로함
/// 알파 값을 설정할 기본 값 (1.0)
//   float alpha = 1.0;
///지울 특정색상
//    float3 targetColor = float3(1.0, 0.0, 1.0);  //(255, 0, 255)
// 
/// 조건: 색상이 특정 색상과 같으면 알파 값을 0으로 설정
//    if (all(abs(color - targetColor) < 0.1))  // 근사치 비교
//    {
//        alpha = 0.0;
//    }

//   return float4(color, alpha);
color[3] *= 0.5f;
   return color;
}
