struct DebugVertexOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

// Vertex Buffer 없이 SV_VertexID 0~2만으로 화면 전체를 덮는 삼각형을 만든다.
DebugVertexOutput main(uint vertexId : SV_VertexID)
{
    DebugVertexOutput output;
    output.uv = float2((vertexId << 1) & 2, vertexId & 2);
    output.position = float4(output.uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    return output;
}
