#pragma once
#include <memory>
#include "DearsGraphicsEngine.h"
struct ModelBuffer;
struct VSConstantBufferData;
struct VSBoneConstantBufferData;
struct PSConstantBufferData;

class RenderObject
{
public:
	RenderObject() = delete;
	RenderObject(DearsGraphicsEngine* _pGrapicsEngine);
	~RenderObject();

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

	//占싣뤄옙 占싸곤옙占쏙옙 占쏙옙占쏙옙 占싫억옙占쏙옙占쏙옙 占식울옙 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙占?占쏙옙占쏙옙占쏙옙 占쏙옙占쌩댐옙. 
	VSShadowConstantBufferData mVSShadowConstantBufferData;
	PsShadowConstantBufferData mPSShadowConstantBufferData;

	VSEdgeConstantBufferData mVSEdgeConstantBufferData;
	PSEdgeConstantBufferData mPSEdgeConstantBufferData;

	PBRVertexShaderConstantData mVSPBRConstantBufferData;
	PBRPixelShaderConstantData mPSPBRConstantBufferData;

	ThinFilmPixelShaderConstantData mPSThinFilmConstantBufferData;
	
	//占쏙옙占쏙옙占싱듸옙占쏙옙 占십울옙占쏙옙 占쏙옙占쏙옙 UV占쏙옙표占썼를 占쏙옙占쏙옙占싱곤옙 占싹깍옙占쏙옙占쏙옙.
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

	bool mIs_VSPBRConstant = false;
	bool mIs_PSPBRConstant = false;
	bool mIs_PSThinFilmConstant = false;


	int mTargetBoneIndex;			//占쏙옙占쏙옙占?占쏙옙占?타占쌕븝옙占쏙옙 占싸듸옙占쏙옙占쏙옙 占싯곤옙 占쌍억옙占쏙옙磯占? -> 占싫곤옙占쏙옙 占쏙옙占쏙옙占?占싸뱄옙 占쏙옙占쌔듸옙 占싫댐옙.


public:
	std::unique_ptr<ModelBuffer> mpModelBuffer;

	Matrix ObjectPos;
	Matrix ObjectRot;
	Matrix ObjectScl;

public:
	
	void CreateVSConstantBuffer() 
	{
		mpModelBuffer->m_pVSConstantBuffer = mpGraphicsEngine->CreateConstantBuffer(mpVSConstantBufferData);
		mIs_VSconstant = true;
	};

	void CreateVSPBRConstantBuffer()
	{
		mpModelBuffer->m_VSPBRConstantBuffer = mpGraphicsEngine->CreateConstantBuffer<PBRVertexShaderConstantData>(mVSPBRConstantBufferData);
		mIs_VSPBRConstant = true;
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

	void CreatePSPBRConstantBuffer()
	{
		mpModelBuffer->m_PSPBRConstantBuffer = mpGraphicsEngine->CreateConstantBuffer(mPSPBRConstantBufferData);  mIs_PSPBRConstant = true;
	};

	void CreatePSThinFilmConstantBuffer()
	{
		mpModelBuffer->m_PSThinFilmConstantBuffer = mpGraphicsEngine->CreateConstantBuffer(mPSThinFilmConstantBufferData);  mIs_PSThinFilmConstant= true;
	};

	//占쏙옙 占싱녀석占쏙옙 占쏙옙占쏙옙占쏙옙트 占쏙옙킬 占쏙옙占쏙옙占쏙옙 占싹댐옙 占쏙옙占쌥억옙占쏙옙. 
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
	void SetCubeMapTexture(std::string _DiffuseTextureName = "", std::string _SpecularTextureName = "");

	void SetPBRTextures(
		std::string albedoTex = "",
		std::string normalTex = "",
		std::string aoTex = "",
		std::string metallicTex = "",
		std::string roughnessTex = "",
		std::string heightTex = ""
	);


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
	ModelBuffer* GetModelBuffer() const;
};

