#include "RenderObject.h"

RenderObject::RenderObject(DearsGraphicsEngine* _pGrapicsEngine)
{
	mpGraphicsEngine = _pGrapicsEngine;
	mTargetBoneIndex = -1;
	ObjectPos = Matrix();
	ObjectRot = Matrix();
	ObjectScl = Matrix();

	mIs_VSconstant = false;
	mIs_VSBoneConstant = false;
	mIs_VSTargetBoneConstant = false;
	mIs_VSInstanceConstant = false;
	mIs_PSconstant = false;
	mIs_VSShadowConstant = false;

	mIs_VSEdgeConstant = false;
	mIs_PSEdgeConstant = false;
}

RenderObject::~RenderObject()
{ 

}

void RenderObject::Initialize()
{
	mpModelBuffer = std::make_unique<ModelBuffer>();
}

void RenderObject::Update()
{
	if (mIs_VSconstant)
	{
		mpVSConstantBufferData.world =
			(
			ObjectScl * ObjectRot * ObjectPos
			).Transpose();
	}
	if (mIs_VSPBRConstant)
	{
		mVSPBRConstantBufferData.world =
			(
				ObjectScl * ObjectRot * ObjectPos
				).Transpose();
	}
	mpVSConstantBufferData.invWorld = mpVSConstantBufferData.world.Transpose().Invert();
	mVSPBRConstantBufferData.invWorld = mVSPBRConstantBufferData.world.Transpose().Invert();

	if (mIs_VSconstant)
	{
		mpGraphicsEngine->UpdateConstantBuffer(mpModelBuffer.get(), mpVSConstantBufferData);
	}
	if (mIs_VSBoneConstant)
	{
		if (mpModelBuffer->mpTargetAnimation)
		{
			if (mpModelBuffer->mpNextTargetAnimation)
			{
				if (!mpGraphicsEngine->UpdateTransitionBoneConstantBuffer(mpModelBuffer.get(), mpVSBoneConstantBufferData))			  ///false?좎룞???좎룞?숉솚?좎떎?듭삕?좎듅怨ㅼ삕? -> ?좎룞?쇿뜝?숈삕 ?좎뙇?덈챿?쇿뜝?깆눦?쇿뜝?숈삕 ?좎룞?쇿뜝?숈삕?좎룞???좎뙇?덈챿?쇿뜝?깆눦?쇿뜝?숈삕?좎룞???좎룞?숈껜?좎떬?먯삕.
				{
					mpModelBuffer->mAnimationPlaytime = mpModelBuffer->mNextAnimationPlaytime;
					mpModelBuffer->mpTargetAnimation = mpModelBuffer->mpNextTargetAnimation;
					mpModelBuffer->mNextAnimationPlaytime = 0;
					mpModelBuffer->mpNextTargetAnimation = nullptr;
				}
			}
			else
			{
				mpGraphicsEngine->UpdateBoneConstantBuffer(mpModelBuffer.get(), mpVSBoneConstantBufferData);
			}
		}
	}

	if (mIs_VSTargetBoneConstant)
	{
		mpGraphicsEngine->UpdateTargetBoneConstantBuffer(mpModelBuffer.get(), mpTargetBoneConstantBufferData);
	}
	if (mIs_VSShadowConstant)
	{
		mpGraphicsEngine->UpdateShadowConstantBuffer(mpModelBuffer.get(), mVSShadowConstantBufferData);
	}
	if (mIs_PSconstant)
	{
		mpGraphicsEngine->UpdateConstantBuffer(mpModelBuffer.get(), mPSConstantBuffer);
	}
	if (mIs_PSShadowConstant)
	{
		mpGraphicsEngine->UpdateShadowConstantBuffer(mpModelBuffer.get(), mPSShadowConstantBufferData);
	}
	if (mIs_VSEdgeConstant)
	{
		mpGraphicsEngine->UpdateVSEdgeConstantBuffer(mpModelBuffer.get(), mVSEdgeConstantBufferData);

	}
	if (mIs_PSEdgeConstant)
	{
		mpGraphicsEngine->UpdatePSEdgeConstantBuffer(mpModelBuffer.get(), mPSEdgeConstantBufferData);

	}
	if (mIs_VSWaterConstant)
	{
		mpGraphicsEngine->UpdateVSWaterConstantBuffer(mpModelBuffer.get(), mVSWaterConstantBufferData);

	}
	if (mIs_VSPBRConstant)
	{
		mpGraphicsEngine->UpdateVSPBRConstantBuffer(mpModelBuffer.get(), mVSPBRConstantBufferData);

	}
	if (mIs_PSPBRConstant)
	{
		mpGraphicsEngine->UpdatePSPBRConstantBuffer(mpModelBuffer.get(), mPSPBRConstantBufferData);

	}

	if (mIs_PSThinFilmConstant)
	{
		mpGraphicsEngine->UpdatePSThinFilmonstantBuffer(mpModelBuffer.get(), mPSThinFilmConstantBufferData);

	}

}


// / <summary>
// / ?좎룞?쇿뜝??숈삕 ?좎떊?먯삕?좎룞?쇿뜝?숈삕 ?좎룞?쇿뜝?숈삕 ?좎룞???좎룞?쇿뜝?쒕챿???좎떬諭꾩삕?좎룞???좎룞?쇿뜝?숈삕?좎떦?먯삕 ?좎뙃?쎌삕
// / </summary>
// / <param name="_VBname">VertexBuffer?좎룞??/param>
// / <param name="_IBname">Index Buffer?좎룞??/param>
// / <param name="_NIname">NumIndex?좎룞??/param>
// / <param name="_Tname">Texture?좎룞??/param>
// / <param name="_MIname">?좎룞?쇿뜝?숈삕?좎룞???좎뙣?꾩슱??TargetModel?좎룞??/param>
// / <param name="_Aname">Animation?좎룞??/param>
// void RenderObject::SetModelBuffer(std::string _VIname, std::string _dTname, std::string _MIname, std::string _Aname)
// {
// 	mpModelBuffer->m_pVertexBuffer = mpGraphicsEngine->Get_VertexBuffer(_VIname);
// 	mpModelBuffer->m_pIndexBuffer = mpGraphicsEngine->Get_IndexBuffer(_VIname);
// 	mpModelBuffer->mNumIndices = mpGraphicsEngine->Get_NumIndex(_VIname);
// 
// 	mpModelBuffer->m_diffusetexture = mpGraphicsEngine->Get_Textures(_dTname);
// 	if (!_MIname.empty()) mpModelBuffer->mpTargetModel = mpGraphicsEngine->Get_ModelInfo(_MIname);
// 	if (!_Aname.empty()) mpModelBuffer->mpTargetAnimation = mpGraphicsEngine->Get_Animation(_Aname);
// }

void RenderObject::SetVIBuffer(std::string _bufferName)
{
	mpModelBuffer->m_pVertexBuffer = mpGraphicsEngine->Get_VertexBuffer(_bufferName);
	mpModelBuffer->m_pIndexBuffer = mpGraphicsEngine->Get_IndexBuffer(_bufferName);
	mpModelBuffer->mNumIndices = mpGraphicsEngine->Get_NumIndex(_bufferName);
}

void RenderObject::SetDiffuseTexture(std::string _TextureName)
{
	mpModelBuffer->m_diffusetexture = mpGraphicsEngine->Get_Textures(_TextureName);

}


void RenderObject::SetCubeMapTexture(std::string _DiffuseTextureName, std::string _SpecularTextureName)
{
	mpModelBuffer->m_cubeMapDiffuseResourceView = mpGraphicsEngine->Get_Textures(_DiffuseTextureName);

	if (!_SpecularTextureName.empty())
	{
	mpModelBuffer->m_cubeMapSpecularResourceView = mpGraphicsEngine->Get_Textures(_SpecularTextureName);
	}
}

void RenderObject::SetPBRTextures(std::string albedoTex, std::string normalTex, std::string aoTex, 
								std::string metallicTex, std::string roughnessTex, std::string heightTex)
{
	if (albedoTex != "")
	{
		mpModelBuffer->albedoTex = mpGraphicsEngine->Get_Textures(albedoTex);
		mPSPBRConstantBufferData.useAlbedoMap = 1;
	}
	else
	{
		mPSPBRConstantBufferData.useAlbedoMap = 0;
	}
	if (normalTex != "")
	{
		mpModelBuffer->normalTex = mpGraphicsEngine->Get_Textures(normalTex);
		mPSPBRConstantBufferData.useNormalMap = 1;
	}
	else
	{
		mPSPBRConstantBufferData.useNormalMap = 0;
	}
	if (aoTex != "")
	{
		mpModelBuffer->aoTex = mpGraphicsEngine->Get_Textures(aoTex);
		mPSPBRConstantBufferData.useAOMap = 1;
	}
	else
	{
		mPSPBRConstantBufferData.useAOMap = 0;
	}
	if (metallicTex != "")
	{
		mpModelBuffer->metallicTex = mpGraphicsEngine->Get_Textures(metallicTex);
		mPSPBRConstantBufferData.useMetallicMap = 1;
	}
	else
	{
		mPSPBRConstantBufferData.useMetallicMap = 0;
	}
	if (roughnessTex != "")
	{
		mpModelBuffer->roughnessTex = mpGraphicsEngine->Get_Textures(roughnessTex);
		mPSPBRConstantBufferData.useRoughnessMap = 1;
	}
	else
	{
		mPSPBRConstantBufferData.useRoughnessMap = 0;
	}
	if (heightTex != "")
	{
		mpModelBuffer->heightTex = mpGraphicsEngine->Get_Textures(heightTex);
		mVSPBRConstantBufferData.useHeightMap = 1;
	}
	else
	{
		mVSPBRConstantBufferData.useHeightMap = 0;
	}

}

void RenderObject::SetAnimation(std::string _Aname)
{
	mpModelBuffer->mpTargetAnimation = mpGraphicsEngine->Get_Animation(_Aname);
}

void RenderObject::SetNextAnimation(std::string _Aname)
{
	Animation* temp = mpGraphicsEngine->Get_Animation(_Aname);
	//if (!mpModelBuffer->mpNextTargetAnimation && mpModelBuffer->mpNextTargetAnimation != temp)
	//{
	mpModelBuffer->mpNextTargetAnimation = mpGraphicsEngine->Get_Animation(_Aname);
	mpModelBuffer->mNextAnimationPlaytime = 0;
	//}
}

void RenderObject::SetModelInfo(std::string _ModelName)
{
	mpModelBuffer->mpTargetModel = mpGraphicsEngine->Get_ModelInfo(_ModelName);
}

void RenderObject::SetTargetBoneIndex(int _targetBoneIndex)
{
	mTargetBoneIndex = _targetBoneIndex;
}

void RenderObject::UpdateAnimationTime(float _deltaTime)
{
	if (mpModelBuffer->mpTargetAnimation)
	{
		mpModelBuffer->mAnimationPlaytime += _deltaTime;
	}
	if (mpModelBuffer->mpNextTargetAnimation)
	{
		mpModelBuffer->mNextAnimationPlaytime += _deltaTime;
	}
}

void RenderObject::UpdatePSConstantBufferData(PSConstantBufferData& _PSConstantdata)
{
	mPSConstantBuffer = _PSConstantdata;
}

void RenderObject::UpdateVSShadowConstantBufferData(Matrix _shadowView, Matrix _shadowProjection)
{
	mVSShadowConstantBufferData.ShadowView = _shadowView;
	mVSShadowConstantBufferData.ShadowProjection = _shadowProjection;
}

void RenderObject::SetObjectPos(Matrix _pos)
{
	ObjectPos = _pos;
}

void RenderObject::SetObjectRot(Matrix _Rot)
{
	ObjectRot = _Rot;
}

void RenderObject::SetObjectScl(Matrix _Scl)
{
	ObjectScl = _Scl;
}

void RenderObject::GetObjectTargetBoneMatrix(std::string _targetModel, std::string _targetBoneName)
{
	mpTargetBoneConstantBufferData.targrtBoneMatrix = mpGraphicsEngine->m_pResourceManager->Get_TargetBoneMatrix(_targetModel, _targetBoneName).Transpose();
}

void RenderObject::GetObjectTargetBoneMatrix(VSBoneConstantBufferData _targetModelBoneConstantBuffer)
{
	mpTargetBoneConstantBufferData.targrtBoneMatrix = _targetModelBoneConstantBuffer.bone[mTargetBoneIndex];
}

ModelBuffer* RenderObject::GetModelBuffer() const
{
	return mpModelBuffer.get();
}
