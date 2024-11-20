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
#define MAX_BONES 30

// 샘플러들을 모든 쉐이더에서 공통으로 사용
SamplerState linearWrapSampler : register(s0);
SamplerState linearClampSampler : register(s1);
SamplerState shadowPointSampler : register(s2);
SamplerComparisonState shadowCompareSampler : register(s3);

//공통텍스쳐
TextureCube g_diffuseCube : register(t10);
TextureCube g_specularCube : register(t11);
Texture2D shadowMaps[1] : register(t15);

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

//기본모델
struct VertexShaderInput
{
    float3 pos : POSITION; //모델 좌표계의 위치, position
    float3 normal : NORMAL; //모델 좌표계의 normal
    float2 texcoord : TEXCOORD;
};

//애니메이션이 있는 모델
struct AniVertexShaderInput
{
    float3 pos : POSITION; //모델 좌표계의 위치, position
    float3 normal : NORMAL; //모델 좌표계의 normal
    float2 texcoord : TEXCOORD;
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
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

float3 BlinnPhong(float _lightStrength, float3 _lightVec, float3 _normal,
                   float3 _toEye, Material _mat)
{
    float3 halfway = normalize(_lightVec + _toEye);
    float hdotn = dot(halfway, _normal);
    float3 specular = _mat.specular * pow(max(hdotn, 0.0001f), _mat.shininess);
    return _mat.ambient + (specular + _mat.diffuse) * float3(_lightStrength, _lightStrength, _lightStrength);
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

    // 쉐이딩할 지점부터 조명까지의 거리 계산
    float d = length(lightVec);

    lightVec /= d; // Normalize light vector

    float ndotl = max(dot(normal, lightVec), 0.0f);
    float LightStrength = L.strength * ndotl;

    float attenuation = CalcAttenuation(d, L.fallOffStart, L.fallOffEnd);
    float d2 = d * d;
    float attEquation = (1 - d2) * (1 - d2) / (1 + attenuation * d); // 감쇄 방정식을 계산한다.
    LightStrength *= attenuation; // 빛에 감쇄율을 곱해준다.
        
    return L.lightColor * BlinnPhong(LightStrength, lightVec, normal, toEye, mat);
}

float3 ComputeSpotLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
    float3 lightVec = L.position - pos;

    // 쉐이딩할 지점부터 조명까지의 거리 계산
    float d = length(lightVec);

    // 너무 멀면 조명이 적용되지 않음
    if (d > L.fallOffEnd)
    {
        return float3(0.0f, 0.0f, 0.0f);
    }
    else
    {
        lightVec /= d;
        float ndotl = max(dot(normal, lightVec), 0.0f);
        float LightStrength = L.strength * ndotl;

        float attenuation = CalcAttenuation(d, L.fallOffStart, L.fallOffEnd);
        float d2 = d * d;
        float attEquation = (1-d2)*(1-d2) / ( 1 + attenuation * d); // 감쇄 방정식을 계산한다.
        LightStrength *= attenuation; // 빛에 감쇄율을 곱해준다.

        float3 spotDir = normalize(L.direction); // spotLight 단위벡터 = 방향 단위벡터
        float spotFactor = max(-dot(spotDir, lightVec), 0.0f); // lightVec과 spotDir의 방향이 다르므로 -를 곱해줘서 양수 취급
        spotFactor = pow(spotFactor, L.spotpower); // 너무 수치가 작아서 오류가 날 수 있어 보간한다.
        LightStrength *= spotFactor;
        return BlinnPhong(LightStrength, lightVec, normal, toEye, mat);
    }
}

//Schlick 근사값(approximation) : 9.17 "리얼타임렌더링"
//_frenel은 물질의 고유성질
//Water   :(0.02, 0.02, 0.02) 
//Glass   :(0.08, 0.08, 0.08) 
//Plastic :(0.05, 0.05, 0.05) 
//Gold    :( 1.0, 0.71, 0.29) 
//Silver  :(0.95, 0.93, 0.88) 
//Copper  :(0.95, 0.64, 0.54)
float3 SchlickFresnel(float3 frenel, float3 normal, float3 toEye)
{
    float normalDotView = saturate(dot(normal, toEye));
    float f = 1.0f - normalDotView;     //90도에 가까울수록 1, 0도에 가까울수록 0
    
    return frenel + (1.0f - frenel) * pow(f, 5.0);

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