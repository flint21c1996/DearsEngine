#include "Common.hlsli"

Texture2D materialBuffer : register(t22);
Texture2D depthBuffer : register(t23);

struct DebugPixelInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

struct DebugOutput
{
    float4 metallic : SV_Target0;
    float4 roughness : SV_Target1;
    float4 ao : SV_Target2;
    float4 linearDepth : SV_Target3;
    float4 worldPosition : SV_Target4;
};

float3 ReconstructWorldPosition(float2 uv, float depth)
{
    // DirectX의 NDC Z는 0~1이므로 OpenGL처럼 depth * 2 - 1 변환을 하지 않는다.
    float2 ndc = float2(uv.x * 2.0f - 1.0f, 1.0f - uv.y * 2.0f);
    float4 clipPosition = float4(ndc, depth, 1.0f);
    float4 worldPosition = mul(clipPosition, invViewProj);
    return worldPosition.xyz / max(abs(worldPosition.w), 0.00001f);
}

DebugOutput main(DebugPixelInput input)
{
    DebugOutput output;
    int2 pixel = int2(input.position.xy);
    float4 material = materialBuffer.Load(int3(pixel, 0));
    float depth = depthBuffer.Load(int3(pixel, 0)).r;
    float3 worldPosition = ReconstructWorldPosition(input.uv, depth);

    output.metallic = float4(material.rrr, 1.0f);
    output.roughness = float4(material.ggg, 1.0f);
    output.ao = float4(material.bbb, 1.0f);

    // 원근 Depth는 대부분 1 근처에 몰려 있어 원본 값을 그대로 표시하면 배경과 메시가 거의 같다.
    // 1-depth로 가까운 표면일수록 큰 값이 되게 뒤집고, 가시화 상수로 작은 차이를 확대한다.
    // 이 값은 Lighting 계산에 쓰는 Depth를 변경하지 않고 디버그 출력에만 적용된다.
    // 투영 깊이는 대부분 1.0 근처에 모이므로 반전한 뒤 확대해서 표시한다.
    // 값이 너무 크면 서로 다른 깊이가 모두 1.0으로 포화되어 같은 빨간색으로 보이므로,
    // 가까운 물체와 먼 물체 사이의 명암이 남을 정도로만 확대한다.
    const float depthVisualizationScale = 12.0f;
    float visibleDepth = depth >= 0.99999f
        ? 0.0f
        : saturate((1.0f - depth) * depthVisualizationScale);
    output.linearDepth = float4(visibleDepth, 0.0f, 0.0f, 1.0f);

    // 절대 월드 좌표는 화면 색 범위를 벗어나므로 -10~10 정도를 0~1 색으로 매핑한다.
    output.worldPosition = depth >= 0.99999f
        ? float4(0.0f, 0.0f, 0.0f, 1.0f)
        : float4(saturate(worldPosition * 0.05f + 0.5f), 1.0f);
    return output;
}
