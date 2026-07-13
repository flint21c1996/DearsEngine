#include "Common.hlsli"

struct DebugLineInput
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct DebugLineOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

DebugLineOutput main(DebugLineInput input)
{
    DebugLineOutput output;
    // 선 정점은 CPU에서 이미 월드 좌표로 만들어졌으므로 World 행렬은 필요 없다.
    // 다만 이 엔진은 row-vector 기준으로 view와 projection을 각각 곱하고 있으므로,
    // 기존 메시 Vertex Shader와 정확히 같은 순서를 사용해야 화면 위치가 일치한다.
    float4 clipPosition = mul(float4(input.position, 1.0f), view);
    clipPosition = mul(clipPosition, proj);
    output.position = clipPosition;
    output.color = input.color;
    return output;
}
