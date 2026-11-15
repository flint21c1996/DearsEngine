struct Output { float4 pos : SV_POSITION; float2 uv : TEXCOORD0; };
Output main(uint id : SV_VertexID)
{
    Output o;
    float2 uv = float2((id << 1) & 2, id & 2);
    o.pos = float4(uv * float2(2, -2) + float2(-1, 1), .5, 1);
    o.uv = uv;
    return o;
}
