//���̴����� include�� ������� .hlsli���Ͽ� �ۼ��Ѵ�. -> Module�� ���ϼ��� ������ �ϴ°�..?

/* ����: C++ SimpleMath -> HLSL */
// Matrix -> matrix �Ǵ� float4x4 �� �����ȴ�.
// Vector3 -> float3 �� �����ȴ�.
// float3 a = normalize(b);               //���� b�� ����ȭ�Ѵ�.
// float a = dot(v1, v2);                 //v1�� v2�� ������ ����Ѵ�.
// Satuarate() -> saturate() ���         //�Լ��� ��� ���� [0,1]�� �����Ѵ�. C++�� Satuarate�Լ��� ������ ������ �Ѵ�.
// float l = length(v);                   //���� v�� ���̸� ����Ѵ�.
// struct A{ float a = 1.0f; }; <- ����ü �ȿ��� �ʱ�ȭ �Ұ�, HLSL������ ����ü ���� �� ��������� �ٷ� �ʱ�ȭ �� �� ����.
// Vector3(0.0f) -> float3(0.0, 0.0, 0.0) // �Ǽ� �ڿ� f ���ʿ�
// Vector4::Transform(v, M) -> mul(v, M)    //���� v�� ��� M�� ���Ѵ�. �̴� C++�� 'Vector4::Transform(v, M)�Լ��� ������ ������ �Ѵ�.
#define MAX_LIGHTS 3 // ���̴������� #define ��� ����
#define MAX_BONES 30

// ���÷����� ��� ���̴����� �������� ���
SamplerState linearWrapSampler : register(s0);
SamplerState linearClampSampler : register(s1);
SamplerState shadowPointSampler : register(s2);
SamplerComparisonState shadowCompareSampler : register(s3);

//�����ؽ���
TextureCube g_diffuseCube : register(t10);
TextureCube g_specularCube : register(t11);
Texture2D shadowMaps[1] : register(t15);

//���� https://www.notion.so/Phong-vs-Blinn-Phong-dcf2952858e0449f834fbe3981a729d5
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

//���� consTant
cbuffer CommonConstantBufferData : register(b1)
{
    matrix view;
    matrix proj;
    matrix viewproj;
    
    matrix invView; //���� �����
    matrix invProj; //���������� �����
    matrix invViewProj; //Proj -> World
    
    float3 eyeWorld;
    float strengthIBL;

    Light lights[MAX_LIGHTS];
    
    unsigned int lightNum;
    float3 dummy10;
}

//�⺻��
struct VertexShaderInput
{
    float3 pos : POSITION; //�� ��ǥ���� ��ġ, position
    float3 normal : NORMAL; //�� ��ǥ���� normal
    float2 texcoord : TEXCOORD;
};

//�ִϸ��̼��� �ִ� ��
struct AniVertexShaderInput
{
    float3 pos : POSITION; //�� ��ǥ���� ��ġ, position
    float3 normal : NORMAL; //�� ��ǥ���� normal
    float2 texcoord : TEXCOORD;
    uint4 boneIndices1 : BONEINDICES; //�� �ε���
    uint4 boneIndices2 : BONEINDICES; //�� �ε���
    float4 boneWeights1 : BONEWEIGHTS; //�� ����ġ
    float4 boneWeights2 : BONEWEIGHTS; //�� ����ġ
};

struct PixelShaderInput
{
    float4 pos : SV_POSITION; // ��ũ�� ��ġ
    float3 posWorld : POSITION; // World ��ġ (���� ��꿡 ���)
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD0; //���̵�
    float3 ndcPos : TEXCOORD1; // 
};


//����
struct Material
{
    float3 ambient;
    float shininess;
    float3 diffuse;
    float dummy1; //16byte�� �����ֱ� ����..
    float3 specular;
    float dummy2; //16����Ʈ �����ֱ� ����..
    float3 fresnel;
    float dummy3; //16����Ʈ
};


// �ø�ƽ�� ����
// �ø�ƽ�� �������� ������ ��Ÿ����. 
// ���� ��� POSITION�� ������ ��ġ�� ��Ÿ���� NORMAL�� ������ ���� ���͸� ��Ÿ����.
// TEXCOORD�� �ؽ�ó ��ǥ�� ��Ÿ����, �ؽ�ó ���ο� ���ȴ�.
// SV_POSITION�� ���� ���̴��� ����� ������ ��ũ�� ���������� ��ġ�� ��Ÿ����.
// �̶� �ø�ƽ�� �ý��ۿ� ���� ���ǵǾ� �ְ� �� �ø�ƽ�� ��������ν� ������������ �����ܰ�
// (��_�ȼ����̴�)�� �� �����Ͱ� ��ũ�� ��ġ �������� �� �� �ִ�.
// �� �ø�ƽ�� HLSL(High Level Shader Language)���� �����ϴ� ������ "��ӵ� ����"���� �����ִ�.

float CalcAttenuation(float d, float falloffStart, float falloffEnd) // ���� ����� ����ϴ� �����Դϴ�.
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
    float ndotl = max(dot(normal, lightVec), 0.0f); // ������ ���� 0.0���� ó���Ѵ�.
    float LightStrength = L.strength * ndotl;
    return L.lightColor * BlinnPhong(LightStrength, lightVec, normal, toEye, mat);
}

// �� �� �����ϰ� ������ ���������Ƿ�... ������ ���ٸ� �� Ȯ���غ���.
float3 ComputePointLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
    float3 lightVec = L.position - pos;

    // ���̵��� �������� ��������� �Ÿ� ���
    float d = length(lightVec);

    lightVec /= d; // Normalize light vector

    float ndotl = max(dot(normal, lightVec), 0.0f);
    float LightStrength = L.strength * ndotl;

    float attenuation = CalcAttenuation(d, L.fallOffStart, L.fallOffEnd);
    float d2 = d * d;
    float attEquation = (1 - d2) * (1 - d2) / (1 + attenuation * d); // ���� �������� ����Ѵ�.
    LightStrength *= attenuation; // ���� �������� �����ش�.
        
    return L.lightColor * BlinnPhong(LightStrength, lightVec, normal, toEye, mat);
}

float3 ComputeSpotLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
    float3 lightVec = L.position - pos;

    // ���̵��� �������� ��������� �Ÿ� ���
    float d = length(lightVec);

    // �ʹ� �ָ� ������ ������� ����
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
        float attEquation = (1-d2)*(1-d2) / ( 1 + attenuation * d); // ���� �������� ����Ѵ�.
        LightStrength *= attenuation; // ���� �������� �����ش�.

        float3 spotDir = normalize(L.direction); // spotLight �������� = ���� ��������
        float spotFactor = max(-dot(spotDir, lightVec), 0.0f); // lightVec�� spotDir�� ������ �ٸ��Ƿ� -�� �����༭ ��� ���
        spotFactor = pow(spotFactor, L.spotpower); // �ʹ� ��ġ�� �۾Ƽ� ������ �� �� �־� �����Ѵ�.
        LightStrength *= spotFactor;
        return BlinnPhong(LightStrength, lightVec, normal, toEye, mat);
    }
}

//Schlick �ٻ簪(approximation) : 9.17 "����Ÿ�ӷ�����"
//_frenel�� ������ ��������
//Water   :(0.02, 0.02, 0.02) 
//Glass   :(0.08, 0.08, 0.08) 
//Plastic :(0.05, 0.05, 0.05) 
//Gold    :( 1.0, 0.71, 0.29) 
//Silver  :(0.95, 0.93, 0.88) 
//Copper  :(0.95, 0.64, 0.54)
float3 SchlickFresnel(float3 frenel, float3 normal, float3 toEye)
{
    float normalDotView = saturate(dot(normal, toEye));
    float f = 1.0f - normalDotView;     //90���� �������� 1, 0���� �������� 0
    
    return frenel + (1.0f - frenel) * pow(f, 5.0);

}


///---------------------------------------------�Լ���----------------------------------------------------
// ��ġ ��ȯ ��� ���� �Լ�
float4x4 TranslationMatrix(float3 pos)
{
    return float4x4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        pos.x, pos.y, pos.z, 1
    );
}

// �����ϸ� ��� ���� �Լ�
float4x4 ScaleMatrix(float3 scale)
{
    return float4x4(
        scale.x, 0, 0, 0,
        0, scale.y, 0, 0,
        0, 0, scale.z, 0,
        0, 0, 0, 1
    );
}

// ȸ�� ��� ���� �Լ� (Yaw-Pitch-Roll ���, XYZ ������ ȸ��)
float4x4 RotationMatrix(float3 rotation)
{
    float cosX = cos(rotation.x);
    float sinX = sin(rotation.x);
    float cosY = cos(rotation.y);
    float sinY = sin(rotation.y);
    float cosZ = cos(rotation.z);
    float sinZ = sin(rotation.z);

    // Yaw-Pitch-Roll ȸ�� ���� (Z, Y, X �� ����)
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

    return mul(rotZ, mul(rotY, rotX)); // Z -> Y -> X ������ ȸ�� ����
}

// �̵� ����� ����� ����
float4x4 InverseTranslationMatrix(float3 pos)
{
    return float4x4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        -pos.x, -pos.y, -pos.z, 1
    );
}

// ������ ����� ����� ����
float4x4 InverseScaleMatrix(float3 scale)
{
    return float4x4(
        1.0 / scale.x, 0, 0, 0,
        0, 1.0 / scale.y, 0, 0,
        0, 0, 1.0 / scale.z, 0,
        0, 0, 0, 1
    );
}

// ȸ�� ����� ����� ���� (Yaw-Pitch-Roll ���, ȸ�� ������ �ݴ��)
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

    return mul(rotX, mul(rotY, rotZ)); // X -> Y -> Z ������ ��ȸ�� ����
}

///���� �����Լ��� ���� ������ ���� ����ȭ�� �������� ������ �ϴ� ������ ���.
//�����Լ�, -1 ~ 1 ������ ������ ���� ��ȯ�Ѵ�.
float Random(float _ID)
{
    return (frac(sin(_ID * 12.9898) * 43758.5453) * 2.0) - 1.0;
}

///-------------------------------------------------------------------------------------------------