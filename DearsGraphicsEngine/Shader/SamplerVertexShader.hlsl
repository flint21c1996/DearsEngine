#include "Common.hlsli"

struct SamplerPixelShaderInput
{
float4 position : SV_POSITION;
float2 texcoord : TEXCOORD;
};

SamplerPixelShaderInput main(VertexShaderInput input)
{
SamplerPixelShaderInput output;

output.position = float4(input.pos, 1.0);
output.texcoord = input.texcoord;
return output;
}