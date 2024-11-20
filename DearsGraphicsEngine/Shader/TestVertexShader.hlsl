#include "Common.hlsli"

cbuffer VertexConstantBuffer : register(b0)
{
    matrix world;
    matrix invWorld;
}

PixelShaderInput main(VertexShaderInput input)
{
    float4 pos = float4(input.pos, 1.0f);

    Matrix viewNoTrans = invView;
    viewNoTrans[3] = float4(0, 0, 0, 1);
    pos = mul(pos, viewNoTrans);

    PixelShaderInput output;
    pos = mul(pos, world);
    output.posWorld = pos;
    pos = mul(pos, view);
    pos = mul(pos, proj);
    //pos = mul(pos, viewproj);   //x,y,z,w(클립 공간)이고 w로 나눌시 ndc공간으로 바뀐다.
    output.pos = pos;

  output.normal = mul(input.normal, invWorld).xyz;
  output.normal = normalize(output.normal);
    
    //output.normal = normalize(mul((float3x3) invWorld, input.normal));
    output.texcoord = input.texcoord;
    
    return output;
}
