#include "Common.hlsli"

cbuffer EdgePixelConstantBuffer : register(b2)
{
    float3 color;
    float dummy;
};

float4 main(PixelShaderInput input) : SV_TARGET0
{
   float3 tempColor = color;
	//return float4(color, 1.0f);
	return float4(tempColor, 1.0f);
}