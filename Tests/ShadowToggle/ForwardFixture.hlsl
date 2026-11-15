#include "../../DearsGraphicsEngine/Shader/Common.hlsli"
PBRPixelShaderInput main(uint id : SV_VertexID)
{
    PBRPixelShaderInput o;
    float2 uv = float2((id << 1) & 2, id & 2);
    o.pos = float4(uv * float2(2, -2) + float2(-1, 1), .5, 1);
    o.posWorld = float3(0, 0, .5);
    o.normal = float3(0, 0, -1);
    o.texcoord = float2(.5, .5);
    o.tangentWorld = float3(1, 0, 0);
    return o;
}
