#include "Common.hlsli"
Texture2D HeightTex : register(t0);

cbuffer ThinFilmVertexConstantBuffer : register(b6)
{
    matrix world;
    matrix invWorld;
    int useHeightMap;
    float heightScale;
    float y_Point;            //중점의 y의 위치
    float MeshScale;    //메쉬의 크기
}

PBRPixelShaderInput main(VertexShaderInput input)
{
    float4 pos = float4(input.pos, 1.0f);

    PBRPixelShaderInput output;

    pos = mul(pos, world);

    output.normal = mul(input.normal, invWorld).xyz;
    output.normal = normalize(output.normal);

    output.tangentWorld = input.tangentModel;
    output.tangentWorld = mul(output.tangentWorld, world).xyz;

    if (useHeightMap)
    {
        float height = HeightTex.SampleLevel(linearWrapSampler, input.texcoord, 0).r;
        height = height * 2.0 - 1.0;
        pos += float4(output.normal * height * heightScale, 0.0);
    }

    output.posWorld = pos;
    pos = mul(pos, view);
    pos = mul(pos, proj);
    output.pos = pos;

    //output.normal = normalize(mul((float3x3) invWorld, input.normal));
    output.texcoord.x = input.texcoord.x;
    output.texcoord.y = input.texcoord.y;
    
    return output;
}
