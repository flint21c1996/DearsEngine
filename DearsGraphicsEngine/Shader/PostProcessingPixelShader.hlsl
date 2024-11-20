#include "Common.hlsli"

Texture2D g_Texture0 : register(t0);

struct PostProcessingPixelShaderInput
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

// cbuffer PostProcessConstantData : register(b0)
// {
// ///어쩌구 저쩌구 블라블라블라
// }

// float4 main(PostProcessingPixelShaderInput input) : SV_TARGET
// {
// 	//return float4(0.0f, 1.0f, 0.0f, 1.0f);
// 	return g_Texture0.Sample(linearClampSampler, input.texcoord);
// }
static const float weight[5] = { 0.0545, 0.2442, 0.4026, 0.2442, 0.0545 };
//static const float weight[5] = { 0.2, 0.2, 0.2, 0.2, 0.2};

float4 main(PostProcessingPixelShaderInput input) : SV_TARGET
{
    //float dx = float 1 / 1920;
    //float dy = float 1 / 1080;

    float3 color = float3(0, 0, 0);
    int i;
    int j;
    for (i = 0; i < 5; i++)
    {
        for (j=0; j<5; j++)
        {
        color += weight[i] *weight[j]* (g_Texture0.Sample(linearClampSampler, input.texcoord
                        + float2(0.00052 * (i - 2), 0.00052 * (j - 2))).rgb);
        }
    }

    if(color.x + color.y + color.z >=2.0)
    {
        return float4(color, 0.5f);
    }
    else
    {
        return float4(0,0,0,0);
    }
}