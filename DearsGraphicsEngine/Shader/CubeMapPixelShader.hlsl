#include "Common.hlsli" // 쉐이더에서도 include 사용 가능

// TextureCube g_diffuseCube : register(t0);
// TextureCube g_specularCube : register(t1);
//SamplerState g_sampler : register(s0);

float4 main(PixelShaderInput input) : SV_TARGET0
{
    // 주의: 텍스춰 좌표가 float3 입니다.
   // return g_diffuseCube.Sample(g_sampler, input.posWorld.xyz);
    float4 color = g_specularCube.Sample(linearWrapSampler, input.posWorld.xyz);
    return color;
}


///동재야 컴퓨트 쉐이더 시작할때가 온것 같다.