#include "Common.hlsli"

Texture2D g_texture0 : register(t0);

cbuffer PixelConstantBuffer : register(b0)
{
    float maxLights;
    float mipmapLevel;
    float2 dummy;
    Material material;
};

float4 main(PixelShaderInput input) : SV_TARGET0
{
   float4 color =  g_texture0.SampleLevel(linearWrapSampler, input.texcoord,mipmapLevel);
   //�Ʒ��� ���� � �Ӹ��� ����Ǵ��� �ð������� Ȯ���ϱ� ����. 
   //float4 color =  g_texture0.SampleLevel(shadowPointSampler, input.texcoord,mipmapLevel);

/// ������ [255, 0, 255](����ũ)���� ���� �ش�Ǵ� ���� ����������� �ڲ� �ܰ����ʿ���(����) �ڲ� ��ũ���� �����Ǿ� �׳� �����������
/// ���� ���� ������ �⺻ �� (1.0)
//   float alpha = 1.0;
///���� Ư������
//    float3 targetColor = float3(1.0, 0.0, 1.0);  //(255, 0, 255)
// 
/// ����: ������ Ư�� ����� ������ ���� ���� 0���� ����
//    if (all(abs(color - targetColor) < 0.1))  // �ٻ�ġ ��
//    {
//        alpha = 0.0;
//    }

//   return float4(color, alpha);
//color[3] *= 0.5f;
   return color;
}
