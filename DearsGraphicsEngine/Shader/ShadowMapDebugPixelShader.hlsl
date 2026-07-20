Texture2DArray shadowDepth : register(t0);

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

float4 main(DebugPixelInput input) : SV_Target
{
    uint sourceWidth;
    uint sourceHeight;
    uint sourceArraySize;
    shadowDepth.GetDimensions(sourceWidth, sourceHeight, sourceArraySize);
    int2 sourcePixel = int2(input.uv * float2(sourceWidth, sourceHeight));
    sourcePixel = clamp(sourcePixel, int2(0, 0), int2(sourceWidth - 1, sourceHeight - 1));
    uint sourceSlice = min(lightIndex, sourceArraySize - 1);
    float depth = shadowDepth.Load(int4(sourcePixel, sourceSlice, 0)).r;

    // Clear 값 1.0은 라이트 카메라에 아무 물체도 잡히지 않은 픽셀이다.
    // 검은 배경으로 분리해 두면 실제로 기록된 영역의 윤곽을 바로 확인할 수 있다.
    if (depth >= 0.999999f)
    {
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    }

    // DirectX 원근 Depth는 화면에 선형으로 저장되지 않고 1.0 근처에 몰린다.
    // View 공간 거리로 되돌린 뒤 near~far를 0~1로 정규화해서 거리 차이를 눈에 보이게 만든다.
    // 원근 투영은 비선형 Depth를 거리로 복원해야 하지만,
    // 직교 투영의 Depth는 Near~Far 사이에서 이미 선형으로 증가한다.
    float perspectiveDepth = nearPlane * farPlane /
        max(farPlane - depth * (farPlane - nearPlane), 0.00001f);
    float orthographicDepth = lerp(nearPlane, farPlane, depth);
    float viewDepth = lerp(orthographicDepth, perspectiveDepth, isPerspective);
    float normalizedDepth = saturate((viewDepth - nearPlane) / (farPlane - nearPlane));

    // 가까운 물체는 밝은 빨강, 먼 물체는 어두운 빨강으로 표시한다.
    // sqrt는 중간 거리의 작은 차이가 검게 뭉개지는 것을 완화하는 디버그용 대비 조정이다.
    // 로그 스케일로 가까운 구체와 바닥 사이의 작은 거리 차이를 더 많은 색 단계로 펼친다.
    float visibleDepth = log2(1.0f + max(viewDepth - nearPlane, 0.0f)) /
        log2(1.0f + farPlane - nearPlane);
    return float4(visibleDepth, 0.0f, 0.0f, 1.0f);
}
