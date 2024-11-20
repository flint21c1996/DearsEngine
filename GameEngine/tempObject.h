#pragma once
#include "DearsGraphicsEngine.h"
struct ModelBuffer;
struct VSConstantBufferData;
struct VSBoneConstantBufferData;
struct PSConstantBufferData;

class tempObject
{
public:
	tempObject() = delete;
	tempObject(DearsGraphicsEngine* _pGrapicsEngine);
	~tempObject();

private:
	DearsGraphicsEngine* mpGraphicsEngine;

public:
	/// <summary>
	/// model, view, projection, invTranspose
	/// </summary>
	VSConstantBufferData mpVSConstantBufferData;

	/// <summary>
	/// Matrix bone[MAX_BONES]
	/// </summary>
	VSBoneConstantBufferData mpVSBoneConstantBufferData;

	/// <summary>
	/// Matrix targrtBoneMatrix;
	/// </summary>
	VSTargetBoneConstantBufferData mpTargetBoneConstantBufferData;
	
	/// <summary>
	/// Matrix world[MAX_INSTANT]
	/// Matrix invWorld[MAX_INSTANT]
	/// </summary>
	VSInstantConstantBufferData mpInstantConstantBufferData;

	/// <summary>
	/// eyeworld, maxLights, material, light[MaxLight]
	/// </summary>
	PSConstantBufferData mPSConstantBuffer;

	//아래 두개는 지금 안쓰지만 후에 쓸일이 있을까봐 지우지 못했다. 
	VSShadowConstantBufferData mVSShadowConstantBufferData;
	PsShadowConstantBufferData mPSShadowConstantBufferData;

	VSEdgeConstantBufferData mVSEdgeConstantBufferData;
	PSEdgeConstantBufferData mPSEdgeConstantBufferData;
	
	//물쉐이딩에 필요한 정보 UV좌표계를 윔직이게 하기위한.
	VSWaterConstantBufferData mVSWaterConstantBufferData;

	bool mIs_VSconstant;
	bool mIs_VSBoneConstant;
	bool mIs_VSTargetBoneConstant;
	bool mIs_VSInstanceConstant;
	bool mIs_PSconstant;
	bool mIs_VSShadowConstant;
	bool mIs_PSShadowConstant = false;

	bool mIs_VSEdgeConstant;
	bool mIs_PSEdgeConstant;

	bool mIs_VSWaterConstant = false;


	int mTargetBoneIndex;			//장비의 경우 타겟본의 인덱스를 알고 있어야한다. -> 똑같은 계산을 두번 안해도 된다.


public:
	ModelBuffer* mpModelBuffer;

	Matrix ObjectPos;
	Matrix ObjectRot;
	Matrix ObjectScl;

public:
	
	void CreateVSConstantBuffer() 
	{
		mpModelBuffer->m_pVSConstantBuffer = mpGraphicsEngine->CreateConstantBuffer(mpVSConstantBufferData);
		mIs_VSconstant = true;
	};

	void CreateVSBoneConstantBuffer() 
	{
		mpModelBuffer->m_BoneConstantBuffer = mpGraphicsEngine->CreateConstantBuffer(mpVSBoneConstantBufferData); mIs_VSBoneConstant = true;
	};

	void CreateVSTargetBoneConstantBuffer() 
	{
		mpModelBuffer->m_TargetBoneConstantBuffer = mpGraphicsEngine->CreateConstantBuffer(mpTargetBoneConstantBufferData); mIs_VSTargetBoneConstant = true;
	};

	void CreatePSConstantBuffer() 
	{
		mpModelBuffer->m_pPSConstantBuffer = mpGraphicsEngine->CreateConstantBuffer(mPSConstantBuffer);  mIs_PSconstant = true;
	};

	
	//난 이녀석을 업데이트 시킬 생각은 일단 지금없다. 
	void CreateVSInstanceConstantBuffer()
	{
		for (int i = 0; i < MAX_INSTANCE; i++)
		{
			mpInstantConstantBufferData.world[i] = Matrix::CreateTranslation({ static_cast<float>(i),0,0 }).Transpose();
			mpInstantConstantBufferData.world[i].Invert();
			mpModelBuffer->mNumInstances++;
		}
		mpModelBuffer->m_InstanceConstantBuffer = mpGraphicsEngine->CreateConstantBuffer(mpInstantConstantBufferData); mIs_VSInstanceConstant = true;
	}

	void CreateVSEdgeConstantBuffer()
	{
		mpModelBuffer->m_pVSEdgeConstantBuffer = mpGraphicsEngine->CreateConstantBuffer(mpVSConstantBufferData);
		mIs_VSEdgeConstant = true;
	}
	
	void CreatePSEdgeConstantBuffer()
	{
		mpModelBuffer->m_pPSEdgeConstantBuffer = mpGraphicsEngine->CreateConstantBuffer(mPSEdgeConstantBufferData);
		mIs_PSEdgeConstant = true;
	}

	void CreateVSWaterConstantBuffer()
	{
		mpModelBuffer->m_pVSWaterConstantBuffer = mpGraphicsEngine->CreateConstantBuffer(mVSWaterConstantBufferData);
		mIs_VSWaterConstant = true;
	
	}


	void Initialize();
	void Update();
	//void SetModelBuffer(std::string _VIBname, std::string _dTname, std::string _MIname = std::string(), std::string _Aname = std::string());
	void SetVIBuffer(std::string _bufferName);
	void SetDiffuseTexture(std::string _TextureName);
	void SetNormalMapTexture(std::string _TextureName);
	void SetCubeMapTexture(std::string _DiffuseTextureName = "", std::string _SpecularTextureName = "");

	void SetAnimation(std::string _AnimationName);
	void SetNextAnimation(std::string _NextAnimationname);
	void SetModelInfo(std::string _ModelName);
	void SetTargetBoneIndex(int _targetBoneIndex);

	void UpdateAnimationTime(float _deltaTime);
	void UpdatePSConstantBufferData(PSConstantBufferData& _PSConstantdata);

	void UpdateVSShadowConstantBufferData(Matrix _shadowView, Matrix _shadowProjection);

	void SetObjectPos(Matrix _pos);
	void SetObjectRot(Matrix _Rot);
	void SetObjectScl(Matrix _Scl);

	void GetObjectTargetBoneMatrix(std::string _targetModel, std::string _targetBoneName);
	void GetObjectTargetBoneMatrix(VSBoneConstantBufferData _targetModelBoneConstantBuffer);
	ModelBuffer* GetModelBuffer();
};

