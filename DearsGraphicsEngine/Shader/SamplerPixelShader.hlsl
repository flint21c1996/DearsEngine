#include "Common.hlsli"

Texture2D g_Texture0 : register(t0);

struct PostProcessingPixelShaderInput
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

float4 main(PostProcessingPixelShaderInput input) : SV_TARGET
{
    float3 color = g_Texture0.Sample(linearClampSampler, input.texcoord).rgb;
    float l = (color.r + color.g + color.b) / 3;

    return l > 0.5f ? float4(color, 1.0f)
                         : float4(0.0f, 0.0f, 0.0f, 1.0f);

}