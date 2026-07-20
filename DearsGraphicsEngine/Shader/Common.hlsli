//쉐이더에서 include할 내용들은 .hlsli파일에 작성한다. -> Module과 통일성을 가져야 하는가..?

/* 참고: C++ SimpleMath -> HLSL */
// Matrix -> matrix 또는 float4x4 로 대응된다.
// Vector3 -> float3 로 대응된다.
// float3 a = normalize(b);               //벡터 b를 정규화한다.
// float a = dot(v1, v2);                 //v1과 v2의 내적을 계산한다.
// Satuarate() -> saturate() 사용         //함수의 결과 값을 [0,1]로 제한한다. C++의 Satuarate함수와 동일한 역할을 한다.
// float l = length(v);                   //벡터 v의 길이를 계산한다.
// struct A{ float a = 1.0f; }; <- 구조체 안에서 초기화 불가, HLSL에서는 구조체 선언 시 멤버변수를 바로 초기화 할 수 없다.
// Vector3(0.0f) -> float3(0.0, 0.0, 0.0) // 실수 뒤에 f 불필요
// Vector4::Transform(v, M) -> mul(v, M)    //벡터 v와 행렬 M을 곱한다. 이는 C++의 'Vector4::Transform(v, M)함수와 유사한 역할을 한다.
#define MAX_LIGHTS 3 // 쉐이더에서도 #define 사용 가능
#define MAX_BONES 70

// 샘플러들을 모든 쉐이더에서 공통으로 사용
SamplerState linearWrapSampler : register(s0);
SamplerState linearClampSampler : register(s1);
SamplerState shadowPointSampler : register(s2);
SamplerComparisonState shadowCompareSampler : register(s3);

//공통텍스쳐
TextureCube g_environmentCube : register(t10);
TextureCube g_diffuseCube : register(t11);
TextureCube g_specularCube : register(t12);
Texture2D g_BRDFTex : register(t13);

// Directional/Spot Light의 그림자는 라이트 인덱스와 같은 배열 슬라이스에 저장한다.
// 예를 들어 lights[2]가 Spot Light라면 shadowMaps의 2번 슬라이스를 읽는다.
Texture2DArray shadowMaps : register(t15);
// Point Light는 한 위치에서 여섯 방향의 Depth가 필요하므로 TextureCube로 읽는다.
// t15와 t16을 분리하면 2D/Cube 리소스 형식이 달라도 같은 프레임 상수 버퍼를 공유할 수 있다.
// Point Light도 같은 규칙을 사용한다. TextureCubeArray의 Cube 인덱스가 lightIndex다.
TextureCubeArray pointShadowMap : register(t16);

//조명 https://www.notion.so/Phong-vs-Blinn-Phong-dcf2952858e0449f834fbe3981a729d5
struct Light
{
    float strength;
    float dummy;
    float fallOffStart;
    float fallOffEnd;
    float3 direction;
    uint lightType;
    float3 position;
    float spotpower;
    float3 lightColor;
    unsigned int type;
    // C++ Light와 같은 순서를 유지해야 constant buffer의 뒤쪽 행렬이 밀리지 않는다.
    float shadowNear;
    float shadowFar;
    float shadowFovY;
    float shadowWidth;
    float3 shadowUp;
    float shadowPadding;
    matrix viewProj;            //
    matrix invProj;             //
};

//공통 consTant
cbuffer CommonConstantBufferData : register(b1)
{
    matrix view;
    matrix proj;
    matrix viewproj;
    
    matrix invView; //뷰의 역행렬
    matrix invProj; //프로젝션의 역행렬
    matrix invViewProj; //Proj -> World
    
    float3 eyeWorld;
    float strengthIBL;

    Light lights[MAX_LIGHTS];
    
    unsigned int lightNum;
    float3 dummy10;
}

// 원근 Shadow Map의 NDC Depth 변화량은 라이트 카메라에서 멀어질수록 작아진다.
// 고정된 NDC bias는 먼 거리에서 물체와 바닥의 깊이 차이까지 덮어 그림자를 지울 수 있다.
// 따라서 월드 단위 bias를 먼저 정하고 현재 카메라 거리에서 필요한 Depth 단위로 환산한다.
float ComputeShadowBias(
    Light light,
    float receiverDistance,
    float3 positionWorld,
    float3 normalWorld)
{
    float3 surfaceToLight = light.lightType == 1
        ? -light.direction
        : light.position - positionWorld;
    surfaceToLight = normalize(surfaceToLight);

    // 빛과 거의 평행한 면은 shadow acne가 잘 생기므로 약간 더 큰 월드 bias를 사용한다.
    float normalToLight = saturate(dot(normalize(normalWorld), surfaceToLight));
    float worldSpaceBias = lerp(0.01f, 0.05f, 1.0f - normalToLight);

    float nearPlane = max(light.shadowNear, 0.001f);
    float farPlane = max(light.shadowFar, nearPlane + 0.001f);
    float safeDistance = max(abs(receiverDistance), nearPlane);

    // D3D 원근 투영 Depth 식의 거리 미분값이다.
    // 같은 월드 오프셋도 먼 거리에서는 훨씬 작은 NDC Depth 차이로 나타난다.
    // Directional은 직교 투영이므로 거리와 무관한 선형 Depth 변화량을 사용한다.
    // Spot은 원근 투영이므로 기존처럼 현재 거리에서의 비선형 Depth 변화량을 사용한다.
    float depthPerWorldUnit = light.lightType == 1
        ? 1.0f / (farPlane - nearPlane)
        : (farPlane * nearPlane) /
          ((farPlane - nearPlane) * safeDistance * safeDistance);
    return clamp(worldSpaceBias * depthPerWorldUnit, 0.000005f, 0.003f);
}

// Directional/Spot Light가 사용하는 2D Shadow Map 판정이다.
// Directional은 직교 투영이라 w가 1이고, Spot은 원근 투영이라 w가 라이트 카메라 거리이다.
float Calculate2DShadowFactor(
    Light light,
    uint lightIndex,
    float3 positionWorld,
    float3 normalWorld)
{
    float result = 1.0f;
    // Shadow Map을 만들 때 기록한 표면과, 메인 패스에서 다시 계산한 표면의 깊이는
    // 래스터라이즈 해상도와 부동소수점 보간 차이 때문에 완전히 같지 않을 수 있다.
    // 특히 Spot Light를 눕히면 바닥을 비스듬히 보게 되어 이 오차가 커지고,
    // 바닥이 자기 자신을 가렸다고 판단하는 넓은 self-shadow 영역이 생긴다.
    //
    // 비교할 receiver 위치를 표면 노말 방향으로 조금 들어 올리면 실제 caster의
    // 저장 깊이보다 라이트에 가까워져 이러한 오판을 줄일 수 있다. 빛과 표면이
    // 나란할수록 더 큰 offset을 사용하되, 그림자가 물체에서 떨어져 보이지 않도록
    // 월드 단위 최대값을 작게 제한한다.
    float3 surfaceToLight = light.lightType == 1
        ? -light.direction
        : light.position - positionWorld;
    surfaceToLight = normalize(surfaceToLight);
    float normalToLight = saturate(dot(normalize(normalWorld), surfaceToLight));
    float receiverNormalOffset = lerp(0.01f, 0.08f, 1.0f - normalToLight);
    float3 biasedReceiverPosition =
        positionWorld + normalize(normalWorld) * receiverNormalOffset;

    float4 lightSpacePosition = mul(
        float4(biasedReceiverPosition, 1.0f), light.viewProj);
    if (lightSpacePosition.w > 0.0f)
    {
        float receiverDistance = lightSpacePosition.w;
        lightSpacePosition.xyz /= lightSpacePosition.w;

        // fwidth는 현재 픽셀과 이웃 픽셀 사이의 Depth 변화량을 반환한다.
        // Spot을 눕혀 표면을 비스듬히 볼수록 값이 커지므로, 고정 Bias가 놓치는
        // 기울어진 면의 self-shadowing만 선택적으로 더 보정할 수 있다.
        // 최대값은 그림자가 물체에서 떨어져 보이는 Peter Panning을 막기 위한 제한이다.
        float receiverSlopeBias = min(fwidth(lightSpacePosition.z) * 2.0f, 0.002f);
        bool isInsideFrustum =
            abs(lightSpacePosition.x) <= 1.0f &&
            abs(lightSpacePosition.y) <= 1.0f &&
            lightSpacePosition.z >= 0.0f && lightSpacePosition.z <= 1.0f;
        if (isInsideFrustum)
        {
            // Direct3D의 NDC Y와 텍스처 V축 방향이 반대이므로 샘플 좌표에서 한 번 뒤집는다.
            lightSpacePosition.y *= -1.0f;
            float2 shadowTexCoord = lightSpacePosition.xy * 0.5f + 0.5f;
            // Texture2DArray는 (u, v, arraySlice)로 샘플링한다.
            // C++에서 GPU Light 배열과 Shadow Map 배열의 인덱스를 동일하게 유지하므로
            // 별도의 리소스 번호 없이 현재 라이트의 Shadow Map을 선택할 수 있다.
            float shadowMapDepth = shadowMaps.Sample(
                shadowPointSampler, float3(shadowTexCoord, lightIndex)).r;
            float shadowBias = ComputeShadowBias(
                light, receiverDistance, positionWorld, normalWorld);
            shadowBias = min(shadowBias + receiverSlopeBias, 0.003f);
            // 현재 단계는 단일 Depth 샘플을 사용하는 하드 섀도우다.
            // 가려진 직접광을 0.7로 남기면 여러 라이트와 IBL이 더해졌을 때
            // 밝기 차이가 거의 사라지므로, 우선 가시성 판정을 명확한 0/1로 유지한다.
            // 이후 부드러운 경계는 이 값을 임의로 밝히는 대신 PCF 샘플 평균으로 만든다.
            result = lightSpacePosition.z - shadowBias > shadowMapDepth ? 0.0f : 1.0f;
        }
    }
    return result;
}

// Point Shadow Map은 UV를 직접 계산하지 않는다.
// 라이트에서 현재 픽셀로 향하는 3차원 방향을 TextureCube에 넘기면 GPU가 여섯 면 중 하나를 고른다.
float CalculatePointShadowFactor(
    Light light,
    uint lightIndex,
    float3 positionWorld,
    float3 normalWorld)
{
    float result = 1.0f;
    float3 lightToPixel = positionWorld - light.position;
    float distanceToLight = length(lightToPixel);
    float nearPlane = max(light.shadowNear, 0.001f);
    float farPlane = max(light.shadowFar, nearPlane + 0.001f);
    if (distanceToLight > nearPlane && distanceToLight < farPlane)
    {
        // 각 Cube 면 카메라의 view-space Z는 방향 벡터의 가장 큰 절댓값과 같다.
        // Shadow Pass가 일반 D32 투영 깊이를 기록하므로, 거리 자체가 아니라 동일한 NDC 깊이로 변환해 비교한다.
        float receiverFaceDepth = max(
            abs(lightToPixel.x),
            max(abs(lightToPixel.y), abs(lightToPixel.z)));
        float projectedDepth = farPlane / (farPlane - nearPlane) -
            (farPlane * nearPlane) /
            ((farPlane - nearPlane) * max(receiverFaceDepth, nearPlane));
        // TextureCubeArray는 (방향 xyz, Cube 인덱스)로 샘플링한다.
        float shadowMapDepth = pointShadowMap.Sample(
            shadowPointSampler, float4(lightToPixel, lightIndex)).r;
        float shadowBias = ComputeShadowBias(
            light, receiverFaceDepth, positionWorld, normalWorld);
        // Point도 2D Shadow와 동일하게 현재는 하드 섀도우 0/1을 반환한다.
        result = projectedDepth - shadowBias > shadowMapDepth ? 0.0f : 1.0f;
    }
    return result;
}

float CalculateShadowFactor(
    Light light,
    uint lightIndex,
    float3 positionWorld,
    float3 normalWorld)
{
    return light.lightType == 2
        ? CalculatePointShadowFactor(light, lightIndex, positionWorld, normalWorld)
        : Calculate2DShadowFactor(light, lightIndex, positionWorld, normalWorld);
}

//기본모델
struct VertexShaderInput
{
    float3 pos : POSITION; //모델 좌표계의 위치, position
    float3 normal : NORMAL; //모델 좌표계의 normal
    float2 texcoord : TEXCOORD;
    float3 tangentModel : TANGENT;
};

//애니메이션이 있는 모델
struct AniVertexShaderInput
{
    float3 pos : POSITION; //모델 좌표계의 위치, position
    float3 normal : NORMAL; //모델 좌표계의 normal
    float2 texcoord : TEXCOORD;
    float3 tangentModel : TANGENT;
    uint4 boneIndices1 : BONEINDICES; //본 인덱스
    uint4 boneIndices2 : BONEINDICES; //본 인덱스
    float4 boneWeights1 : BONEWEIGHTS; //본 가중치
    float4 boneWeights2 : BONEWEIGHTS; //본 가중치
};

struct PixelShaderInput
{
    float4 pos : SV_POSITION; // 스크린 위치
    float3 posWorld : POSITION; // World 위치 (조명 계산에 사용)
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD0; //셰이딩
    float3 ndcPos : TEXCOORD1; // 
};

struct PBRPixelShaderInput
{
    float4 pos : SV_POSITION; // 스크린 위치
    float3 posWorld : POSITION; // World 위치 (조명 계산에 사용)
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD0; //셰이딩
    float3 tangentWorld : TANGENT0;
};


//재질
struct Material
{
    float3 ambient;
    float shininess;
    float3 diffuse;
    float dummy1; //16byte를 맞춰주기 위해..
    float3 specular;
    float dummy2; //16바이트 맟줘주기 위해..
    float3 fresnel;
    float dummy3; //16바이트
};

struct PBRMaterial
{
    float3 albedo; // baseColor
    float roughness;
    float metallic;
    float3 dummy;
};

// 시맨틱의 역할
// 시맨틱은 데이터의 종류를 나타낸다.
// 예를 들어 POSITION은 정점의 위치를 나타내고 NORMAL은 정점의 법선 벡터를 나타낸다.
// TEXCOORD는 텍스처 좌표를 나타내며, 텍스처 매핑에 사용된다.
// SV_POSITION은 정점 셰이더가 계산한 정점의 스크린 공간에서의 위치를 나타낸다.
// 이때 시맨틱은 시스템에 의해 정의되어 있고 이 시맨틱을 사용함으로써 파이프라인의 다음단계
// (예_픽셀셰이더)는 이 데이터가 스크린 위치 정보임을 알 수 있다.
// 즉 시맨틱은 HLSL(High Level Shader Language)에서 제공하는 일종의 "약속된 문법"으로 볼수있다.

float CalcAttenuation(float d, float falloffStart, float falloffEnd) // 감쇄 상수를 계산하는 공식입니다.
{
    //linear falloff
    return saturate((falloffEnd - d) / max(falloffEnd - falloffStart, 0.0001f));
}

float3 BlinnPhong(float _lightStrength, float3 _lightVec, float3 _normal,
                   float3 _toEye, Material _mat)
{
    float3 halfway = normalize(_lightVec + _toEye);
    float hdotn = dot(halfway, _normal);
    float3 specular = _mat.specular * pow(max(hdotn, 0.0001f), _mat.shininess);

    // 이 함수는 라이트 한 개가 만드는 '직접광'만 반환해야 한다.
    // Ambient를 여기서 더하면 라이트 개수만큼 반복 누적되고, PixelShader의 최종 Ambient까지
    // 한 번 더 더해져 그림자 안쪽이 계속 밝게 남는다. 주변광은 모든 직접광 계산이 끝난 뒤
    // PixelShader에서 딱 한 번만 합성한다.
    return (specular + _mat.diffuse) *
        float3(_lightStrength, _lightStrength, _lightStrength);
}

float3 ComputeDirectionalLight(Light L, Material mat, float3 normal, float3 toEye)
{
    float3 lightVec = -L.direction;
    float ndotl = max(dot(normal, lightVec), 0.0f); // 음수일 때는 0.0으로 처리한다.
    float LightStrength = L.strength * ndotl;
    return L.lightColor * BlinnPhong(LightStrength, lightVec, normal, toEye, mat);
}

// 좀 더 온전하게 공식을 수정했으므로... 오류가 난다면 꼭 확인해보자.
float3 ComputePointLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
    float3 lightVec = L.position - pos;
    float d = length(lightVec);
    float3 result = float3(0.0f, 0.0f, 0.0f);
    if (d > 0.0001f && d < L.fallOffEnd)
    {
        lightVec /= d;
        float ndotl = max(dot(normal, lightVec), 0.0f);
        float lightStrength = L.strength * ndotl;
        float attenuation = CalcAttenuation(d, L.fallOffStart, L.fallOffEnd);
        lightStrength *= attenuation;
        result = L.lightColor * BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
    }
    return result;
}

float3 ComputeSpotLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
    float3 lightVec = L.position - pos;
    float d = length(lightVec);
    float3 result = float3(0.0f, 0.0f, 0.0f);
    if (d > 0.0001f && d < L.fallOffEnd)
    {
        lightVec /= d;
        float ndotl = max(dot(normal, lightVec), 0.0f);
        float lightStrength = L.strength * ndotl;
        float attenuation = CalcAttenuation(d, L.fallOffStart, L.fallOffEnd);
        float3 spotDir = normalize(L.direction);
        float spotCos = max(-dot(spotDir, lightVec), 0.0f);
        float outerCos = cos(radians(clamp(L.shadowFovY, 1.0f, 179.0f)) * 0.5f);
        if (spotCos > outerCos)
        {
            // FOV는 빛이 완전히 끊기는 바깥 경계만 결정한다.
            // Spot Power는 기존 엔진과 동일하게 방향 내적값 자체에 적용해야 한다.
            // FOV 내부 값을 다시 0~1로 정규화한 뒤 거듭제곱하면 기본값 32에서도
            // 중심에서 조금만 벗어난 직접광이 거의 0이 되어 그림자가 보이지 않는다.
            float spotFactor = pow(saturate(spotCos), max(L.spotpower, 1.0f));
            lightStrength *= attenuation * spotFactor;
            result = L.lightColor * BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
        }
    }
    return result;
}

//Schlick 근사값(approximation) : 9.17 "리얼타임렌더링"
//_frenel은 물질의 고유성질
//Water   :(0.02, 0.02, 0.02) 
//Glass   :(0.08, 0.08, 0.08) 
//Plastic :(0.05, 0.05, 0.05) 
//Gold    :( 1.0, 0.71, 0.29) 
//Silver  :(0.95, 0.93, 0.88) 
//Copper  :(0.95, 0.64, 0.54)
float3 SchlickFresnel(float3 fresnel, float3 normal, float3 toEye)
{
    float normalDotView = saturate(dot(normal, toEye));
    float f = 1.0f - normalDotView;     //90도에 가까울수록 1, 0도에 가까울수록 0
    
    return fresnel + (1.0f - fresnel) * pow(f, 5.0);

}


///---------------------------------------------함수들----------------------------------------------------
// 위치 변환 행렬 생성 함수
float4x4 TranslationMatrix(float3 pos)
{
    return float4x4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        pos.x, pos.y, pos.z, 1
    );
}

// 스케일링 행렬 생성 함수
float4x4 ScaleMatrix(float3 scale)
{
    return float4x4(
        scale.x, 0, 0, 0,
        0, scale.y, 0, 0,
        0, 0, scale.z, 0,
        0, 0, 0, 1
    );
}

// 회전 행렬 생성 함수 (Yaw-Pitch-Roll 방식, XYZ 순서로 회전)
float4x4 RotationMatrix(float3 rotation)
{
    float cosX = cos(rotation.x);
    float sinX = sin(rotation.x);
    float cosY = cos(rotation.y);
    float sinY = sin(rotation.y);
    float cosZ = cos(rotation.z);
    float sinZ = sin(rotation.z);

    // Yaw-Pitch-Roll 회전 순서 (Z, Y, X 축 순서)
    float4x4 rotZ = float4x4(
        cosZ, -sinZ, 0, 0,
        sinZ, cosZ, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );

    float4x4 rotY = float4x4(
        cosY, 0, sinY, 0,
        0, 1, 0, 0,
        -sinY, 0, cosY, 0,
        0, 0, 0, 1
    );

    float4x4 rotX = float4x4(
        1, 0, 0, 0,
        0, cosX, -sinX, 0,
        0, sinX, cosX, 0,
        0, 0, 0, 1
    );

    return mul(rotZ, mul(rotY, rotX)); // Z -> Y -> X 순서로 회전 적용
}

// 이동 행렬의 역행렬 생성
float4x4 InverseTranslationMatrix(float3 pos)
{
    return float4x4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        -pos.x, -pos.y, -pos.z, 1
    );
}

// 스케일 행렬의 역행렬 생성
float4x4 InverseScaleMatrix(float3 scale)
{
    return float4x4(
        1.0 / scale.x, 0, 0, 0,
        0, 1.0 / scale.y, 0, 0,
        0, 0, 1.0 / scale.z, 0,
        0, 0, 0, 1
    );
}

// 회전 행렬의 역행렬 생성 (Yaw-Pitch-Roll 방식, 회전 각도를 반대로)
float4x4 InverseRotationMatrix(float3 rotation)
{
    float cosX = cos(-rotation.x), sinX = sin(-rotation.x);
    float cosY = cos(-rotation.y), sinY = sin(-rotation.y);
    float cosZ = cos(-rotation.z), sinZ = sin(-rotation.z);

    float4x4 rotZ = float4x4(
        cosZ, -sinZ, 0, 0,
        sinZ, cosZ, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );

    float4x4 rotY = float4x4(
        cosY, 0, sinY, 0,
        0, 1, 0, 0,
        -sinY, 0, cosY, 0,
        0, 0, 0, 1
    );

    float4x4 rotX = float4x4(
        1, 0, 0, 0,
        0, cosX, -sinX, 0,
        0, sinX, cosX, 0,
        0, 0, 0, 1
    );

    return mul(rotX, mul(rotY, rotZ)); // X -> Y -> Z 순서로 역회전 적용
}

///추후 랜덤함수를 좀더 나누면 좀더 최적화에 용이하지 않을까 하는 생각이 든다.
//랜덤함수, -1 ~ 1 사이의 임의의 수를 반환한다.
float Random(float _ID)
{
    return (frac(sin(_ID * 12.9898) * 43758.5453) * 2.0) - 1.0;
}

///-------------------------------------------------------------------------------------------------
