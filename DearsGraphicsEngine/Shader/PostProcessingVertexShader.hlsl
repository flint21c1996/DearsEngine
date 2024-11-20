#include "Common.hlsli"

struct PostProcessingPixelShaderInput
{
float4 position : SV_POSITION;
float2 texcoord : TEXCOORD;
};

PostProcessingPixelShaderInput main(VertexShaderInput input)
{
PostProcessingPixelShaderInput output;

output.position = float4(input.pos, 1.0);
output.texcoord = input.texcoord;
return output;
}