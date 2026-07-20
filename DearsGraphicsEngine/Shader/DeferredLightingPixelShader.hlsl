#include "Common.hlsli"
#include "PBRLighting.hlsli"

// GBuffer::LightingShaderSlot(20)부터 C++이 같은 순서로 묶어준다.
// 각 Texture2D는 화면의 한 픽셀에 해당하는 표면 정보를 가지고 있다.
Texture2D gBufferAlbedo : register(t20);
Texture2D gBufferNormal : register(t21);
Texture2D gBufferMaterial : register(t22);
Texture2D gBufferDepth : register(t23);

struct DeferredLightingInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

// Geometry Pass에서는 World Position을 별도 G-Buffer에 저장하지 않는다.
// Position은 RGB 3채널의 고정밀 Render Target을 하나 더 요구하므로 메모리와 대역폭이 커진다.
// 대신 현재 픽셀의 Depth와 카메라의 역 View-Projection 행렬로 월드 위치를 복원한다.
float3 ReconstructWorldPosition(float2 uv, float depth)
{
    // 화면 UV(0~1)를 Direct3D NDC의 X/Y(-1~1)로 되돌린다.
    // 텍스처 V축은 아래로 증가하고 NDC Y축은 위로 증가하므로 Y를 뒤집는다.
    float2 ndc = float2(
        uv.x * 2.0f - 1.0f,
        1.0f - uv.y * 2.0f);

    // Direct3D의 NDC Depth는 이미 0~1 범위이므로 OpenGL처럼 -1~1로 바꾸지 않는다.
    float4 clipPosition = float4(ndc, depth, 1.0f);
    float4 worldPosition = mul(clipPosition, invViewProj);
    return worldPosition.xyz / max(abs(worldPosition.w), 0.00001f);
}

float4 main(DeferredLightingInput input) : SV_TARGET0
{
    int2 pixel = int2(input.position.xy);
    float depth = gBufferDepth.Load(int3(pixel, 0)).r;

    // 1.0은 Geometry Pass에서 아무 오브젝트도 기록하지 않은 Clear Depth다.
    // 이 픽셀을 덮지 않아야 뒤의 Forward Pass에서 그릴 Skybox와 배경색이 유지된다.
    if (depth >= 0.99999f)
    {
        discard;
    }

    float3 albedo = gBufferAlbedo.Load(int3(pixel, 0)).rgb;

    // Geometry Pass는 음수도 갖는 월드 노멀을 0~1로 인코딩해서 저장했다.
    // 조명 계산 전에 다시 -1~1 범위로 되돌리고 정규화한다.
    float3 encodedNormal = gBufferNormal.Load(int3(pixel, 0)).rgb;
    float3 normalWorld = normalize(encodedNormal * 2.0f - 1.0f);

    // Material Target의 R/G/B에는 각각 Metallic/Roughness/AO가 들어 있다.
    float3 packedMaterial = gBufferMaterial.Load(int3(pixel, 0)).rgb;
    float metallic = packedMaterial.r;
    float roughness = packedMaterial.g;
    float ao = packedMaterial.b;

    float3 positionWorld = ReconstructWorldPosition(input.uv, depth);
    float3 pixelToEye = normalize(eyeWorld - positionWorld);

    // Forward PBR과 같은 함수를 호출한다. 데이터의 출처만 G-Buffer로 달라졌을 뿐,
    // IBL, Directional/Point/Spot Light, Shadow 계산식은 두 경로가 공유한다.
    float3 finalColor = EvaluateScenePbrLighting(
        positionWorld,
        normalWorld,
        pixelToEye,
        albedo,
        metallic,
        roughness,
        ao);

    return float4(clamp(finalColor, 0.0f, 1000.0f), 1.0f);
}
