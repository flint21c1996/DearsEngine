TextureCubeArray pointShadowDepth : register(t0);
SamplerState shadowPointSampler : register(s2);

cbuffer ShadowDebugConstants : register(b0)
{
    float nearPlane;
    float farPlane;
    float isPerspective;
    uint lightIndex;
};

struct DebugPixelInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float3 GetCubeDirection(uint faceIndex, float2 faceUv)
{
    // faceUv는 셀의 왼쪽 위 (0, 0)부터 오른쪽 아래 (1, 1)까지 증가한다.
    // 카메라 투영의 Y는 위가 +이므로 텍스처 V축만 반대로 바꾸어 각 면의 월드 방향을 만든다.
    float2 axis = faceUv * 2.0f - 1.0f;
    float x = axis.x;
    float y = -axis.y;

    float3 direction = float3(-x, y, -1.0f);              // -Z를 기본값으로 둔다.
    if (faceIndex == 0) direction = float3( 1.0f, y, -x); // +X
    else if (faceIndex == 1) direction = float3(-1.0f, y,  x); // -X
    else if (faceIndex == 2) direction = float3( x,  1.0f, -y); // +Y
    else if (faceIndex == 3) direction = float3( x, -1.0f,  y); // -Y
    else if (faceIndex == 4) direction = float3( x,  y,  1.0f); // +Z
    return direction;
}

float4 main(DebugPixelInput input) : SV_Target
{
    // 3열 x 2행 Atlas로 여섯 면을 한 화면에 펼친다.
    float2 atlasPosition = saturate(input.uv) * float2(3.0f, 2.0f);
    uint column = min((uint)atlasPosition.x, 2u);
    uint row = min((uint)atlasPosition.y, 1u);
    uint faceIndex = row * 3u + column;
    float2 faceUv = frac(atlasPosition);
    float3 sampleDirection = GetCubeDirection(faceIndex, faceUv);
    float depth = pointShadowDepth.SampleLevel(
        shadowPointSampler, float4(sampleDirection, lightIndex), 0.0f).r;

    if (depth >= 0.999999f)
    {
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    }

    // Point Shadow의 모든 면은 90도 원근 투영이므로 저장된 비선형 Depth를 거리로 복원한다.
    float safeNear = max(nearPlane, 0.001f);
    float safeFar = max(farPlane, safeNear + 0.001f);
    float viewDepth = safeNear * safeFar /
        max(safeFar - depth * (safeFar - safeNear), 0.00001f);
    float visibleDepth = log2(1.0f + max(viewDepth - safeNear, 0.0f)) /
        log2(1.0f + safeFar - safeNear);
    return float4(visibleDepth, 0.0f, 0.0f, 1.0f);
}
