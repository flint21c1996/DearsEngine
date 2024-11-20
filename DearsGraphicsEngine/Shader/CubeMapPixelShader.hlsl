#include "Common.hlsli" // ���̴������� include ��� ����

// TextureCube g_diffuseCube : register(t0);
// TextureCube g_specularCube : register(t1);
//SamplerState g_sampler : register(s0);

float4 main(PixelShaderInput input) : SV_TARGET0
{
    // ����: �ؽ��� ��ǥ�� float3 �Դϴ�.
   // return g_diffuseCube.Sample(g_sampler, input.posWorld.xyz);
    float4 color = g_specularCube.Sample(linearWrapSampler, input.posWorld.xyz);
    return color;
}


///����� ��ǻƮ ���̴� �����Ҷ��� �°� ����.