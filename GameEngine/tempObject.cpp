#include "tempObject.h"

tempObject::tempObject(DearsGraphicsEngine* _pGrapicsEngine)
{
	mpGraphicsEngine = _pGrapicsEngine;
	mpModelBuffer = nullptr;
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

tempObject::~tempObject()
{ 

}

void tempObject::Initialize()
{
	if (mpModelBuffer)
	{
		delete mpModelBuffer;
	}
	mpModelBuffer = new ModelBuffer();
}

void tempObject::Update()
{
	mpVSConstantBufferData.world =
		(
			ObjectScl * ObjectRot * ObjectPos
			).Transpose();
	mpVSConstantBufferData.invWorld = mpVSConstantBufferData.world.Transpose().Invert();

	if (mIs_VSconstant)
	{
		mpGraphicsEngine->UpdateConstantBuffer(mpModelBuffer, mpVSConstantBufferData);
	}
	if (mIs_VSBoneConstant)
	{
		if (mpModelBuffer->mpTargetAnimation)
		{
			if (mpModelBuffer->mpNextTargetAnimation)
			{
				if (!mpGraphicsEngine->UpdateTransitionBoneConstantBuffer(mpModelBuffer, mpVSBoneConstantBufferData))			  ///false�� ��ȯ�Ǿ��°�? -> ���� �ִϸ��̼��� ������ �ִϸ��̼����� ��ü�Ѵ�.
				{
					mpModelBuffer->mAnimationPlaytime = mpModelBuffer->mNextAnimationPlaytime;
					mpModelBuffer->mpTargetAnimation = mpModelBuffer->mpNextTargetAnimation;
					mpModelBuffer->mNextAnimationPlaytime = 0;
					mpModelBuffer->mpNextTargetAnimation = nullptr;
				}
			}
			else
			{
				mpGraphicsEngine->UpdateBoneConstantBuffer(mpModelBuffer, mpVSBoneConstantBufferData);
			}
		}
	}

	if (mIs_VSTargetBoneConstant)
	{
		mpGraphicsEngine->UpdateTargetBoneConstantBuffer(mpModelBuffer, mpTargetBoneConstantBufferData);
	}
	if (mIs_VSShadowConstant)
	{
		mpGraphicsEngine->UpdateShadowConstantBuffer(mpModelBuffer, mVSShadowConstantBufferData);
	}
	if (mIs_PSconstant)
	{
		mpGraphicsEngine->UpdateConstantBuffer(mpModelBuffer, mPSConstantBuffer);
	}
	if (mIs_PSShadowConstant)
	{
		mpGraphicsEngine->UpdateShadowConstantBuffer(mpModelBuffer, mPSShadowConstantBufferData);
	}
	if (mIs_VSEdgeConstant)
	{
		mpGraphicsEngine->UpdateVSEdgeConstantBuffer(mpModelBuffer, mVSEdgeConstantBufferData);

	}
	if (mIs_PSEdgeConstant)
	{
		mpGraphicsEngine->UpdatePSEdgeConstantBuffer(mpModelBuffer, mPSEdgeConstantBufferData);

	}
	if (mIs_VSWaterConstant)
	{
		mpGraphicsEngine->UpdateVSWaterConstantBuffer(mpModelBuffer, mVSWaterConstantBufferData);

	}
}


// / <summary>
// / ���ҽ� �Ŵ����� ���� �� ���۸� �ѹ��� �����ϴ� �Լ�
// / </summary>
// / <param name="_VBname">VertexBuffer��</param>
// / <param name="_IBname">Index Buffer��</param>
// / <param name="_NIname">NumIndex��</param>
// / <param name="_Tname">Texture��</param>
// / <param name="_MIname">������ �޾ƿ� TargetModel��</param>
// / <param name="_Aname">Animation��</param>
// void tempObject::SetModelBuffer(std::string _VIname, std::string _dTname, std::string _MIname, std::string _Aname)
// {
// 	mpModelBuffer->m_pVertexBuffer = mpGraphicsEngine->Get_VertexBuffer(_VIname);
// 	mpModelBuffer->m_pIndexBuffer = mpGraphicsEngine->Get_IndexBuffer(_VIname);
// 	mpModelBuffer->mNumIndices = mpGraphicsEngine->Get_NumIndex(_VIname);
// 
// 	mpModelBuffer->m_diffusetexture = mpGraphicsEngine->Get_Textures(_dTname);
// 	if (!_MIname.empty()) mpModelBuffer->mpTargetModel = mpGraphicsEngine->Get_ModelInfo(_MIname);
// 	if (!_Aname.empty()) mpModelBuffer->mpTargetAnimation = mpGraphicsEngine->Get_Animation(_Aname);
// }

void tempObject::SetVIBuffer(std::string _bufferName)
{
	mpModelBuffer->m_pVertexBuffer = mpGraphicsEngine->Get_VertexBuffer(_bufferName);
	mpModelBuffer->m_pIndexBuffer = mpGraphicsEngine->Get_IndexBuffer(_bufferName);
	mpModelBuffer->mNumIndices = mpGraphicsEngine->Get_NumIndex(_bufferName);
}

void tempObject::SetDiffuseTexture(std::string _TextureName)
{
	mpModelBuffer->m_diffusetexture = mpGraphicsEngine->Get_Textures(_TextureName);

}

void tempObject::SetNormalMapTexture(std::string _TextureName)
{
	mpModelBuffer->m_normaltexture = mpGraphicsEngine->Get_Textures(_TextureName);

}

void tempObject::SetCubeMapTexture(std::string _DiffuseTextureName, std::string _SpecularTextureName)
{
	mpModelBuffer->m_cubeMapDiffuseResourceView = mpGraphicsEngine->Get_Textures(_DiffuseTextureName);

	if (!_SpecularTextureName.empty())
	{
	mpModelBuffer->m_cubeMapSpecularResourceView = mpGraphicsEngine->Get_Textures(_SpecularTextureName);
	}
}

void tempObject::SetAnimation(std::string _Aname)
{
	mpModelBuffer->mpTargetAnimation = mpGraphicsEngine->Get_Animation(_Aname);
}

void tempObject::SetNextAnimation(std::string _Aname)
{
	Animation* temp = mpGraphicsEngine->Get_Animation(_Aname);
	//if (!mpModelBuffer->mpNextTargetAnimation && mpModelBuffer->mpNextTargetAnimation != temp)
	//{
	mpModelBuffer->mpNextTargetAnimation = mpGraphicsEngine->Get_Animation(_Aname);
	mpModelBuffer->mNextAnimationPlaytime = 0;
	//}
}

void tempObject::SetModelInfo(std::string _ModelName)
{
	mpModelBuffer->mpTargetModel = mpGraphicsEngine->Get_ModelInfo(_ModelName);
}

void tempObject::SetTargetBoneIndex(int _targetBoneIndex)
{
	mTargetBoneIndex = _targetBoneIndex;
}

void tempObject::UpdateAnimationTime(float _deltaTime)
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

void tempObject::UpdatePSConstantBufferData(PSConstantBufferData& _PSConstantdata)
{
	mPSConstantBuffer = _PSConstantdata;
}

void tempObject::UpdateVSShadowConstantBufferData(Matrix _shadowView, Matrix _shadowProjection)
{
	mVSShadowConstantBufferData.ShadowView = _shadowView;
	mVSShadowConstantBufferData.ShadowProjection = _shadowProjection;
}

void tempObject::SetObjectPos(Matrix _pos)
{
	ObjectPos = _pos;
}

void tempObject::SetObjectRot(Matrix _Rot)
{
	ObjectRot = _Rot;
}

void tempObject::SetObjectScl(Matrix _Scl)
{
	ObjectScl = _Scl;
}

void tempObject::GetObjectTargetBoneMatrix(std::string _targetModel, std::string _targetBoneName)
{
	mpTargetBoneConstantBufferData.targrtBoneMatrix = mpGraphicsEngine->m_pResourceManager->Get_TargetBoneMatrix(_targetModel, _targetBoneName).Transpose();
}

void tempObject::GetObjectTargetBoneMatrix(VSBoneConstantBufferData _targetModelBoneConstantBuffer)
{
	mpTargetBoneConstantBufferData.targrtBoneMatrix = _targetModelBoneConstantBuffer.bone[mTargetBoneIndex];
}

ModelBuffer* tempObject::GetModelBuffer()
{
	return mpModelBuffer;
}
