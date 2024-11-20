#include "GraphicsResourceContainer.h"

GraphicsResourceContainer::GraphicsResourceContainer()
{

}

GraphicsResourceContainer::~GraphicsResourceContainer()
{
	for (auto& modelAnimPair : mModelAnimations) 
	{
		delete[] modelAnimPair.second;
	}
}

void GraphicsResourceContainer::Add_VertexBuffer(std::string _modelName, ComPtr<ID3D11Buffer>_vertexBuffers)
{
	if (!CheckResource(_modelName, mVertexBuffers))
	{
		mVertexBuffers[_modelName] = _vertexBuffers;
	}

}

void GraphicsResourceContainer::Add_IndexBuffer(std::string _modelName, ComPtr<ID3D11Buffer>_indexBuffers)
{
	if (!CheckResource(_modelName, mIndexBuffers))
	{
		mIndexBuffers[_modelName] = _indexBuffers;
	}

}

void GraphicsResourceContainer::Add_NumIndex(std::string _modelName, unsigned int _numIndex)
{
	if (!CheckResource(_modelName, mNumIndex))
	{
		mNumIndex[_modelName] = _numIndex;
	}
}

void GraphicsResourceContainer::Add_ModelInfo(std::string _modelName, Model* _model)
{
	if (!CheckResource(_modelName, mModels))
	{
		mModels[_modelName] = _model;
	}
}

void GraphicsResourceContainer::Add_BoneName(std::string _modelName, std::string _boneName)
{
		mModelBoneName[_modelName].push_back(_boneName);
}

void GraphicsResourceContainer::Add_Animation(std::string _animName, Animation* _animation)
{
	if (!CheckResource(_animName, mModelAnimations))
	{
		mModelAnimations[_animName] = _animation;
	}
}

void GraphicsResourceContainer::Add_Textures(std::string _textureName, ComPtr<ID3D11ShaderResourceView> _shaderResourceView)
{
	if (!CheckResource(_textureName, mTextures))
	{
		mTextures[_textureName] = _shaderResourceView;
	}
}


void GraphicsResourceContainer::Add_Font(std::string _fontName, ImFont* _font)
{
	if (!CheckResource(_fontName, mFont))
	{
		mFont[_fontName] = _font;
	}
}

void GraphicsResourceContainer::Erase_VertexBuffer(std::string _modelName)
{
	if (CheckResource(_modelName, mVertexBuffers))
	{
		mVertexBuffers.erase(_modelName);
	}
}

void GraphicsResourceContainer::Erase_IndexBuffer(std::string _modelName)
{
	if (CheckResource(_modelName, mIndexBuffers))
	{
		mIndexBuffers.erase(_modelName);
	}
}

void GraphicsResourceContainer::Erase_NumIndex(std::string _modelName)
{
	if (CheckResource(_modelName, mNumIndex))
	{
		mNumIndex.erase(_modelName);
	}
}

void GraphicsResourceContainer::Erase_ModelInfo(std::string _modelName)
{
	if (CheckResource(_modelName, mModels))
	{
		delete mModels[_modelName];
		mModels.erase(_modelName);
	}
}

void GraphicsResourceContainer::Erase_BoneNameVec(std::string _modelName)
{

	if (CheckResource(_modelName, mModelBoneName))
	{
		mModelBoneName[_modelName].clear();
	}
}

void GraphicsResourceContainer::Erase_Animation(std::string _animName)
{
	if (CheckResource(_animName, mModelAnimations))
	{
		delete[] mModelAnimations[_animName];  // 객체 삭제
		mModelAnimations[_animName] = nullptr;  // 삭제 후 포인터를 nullptr로 설정하여 이중 삭제 방지
		mModelAnimations.erase(_animName);  // 맵에서 항목 제거
	}
}

void GraphicsResourceContainer::Erase_Textures(std::string _textureName)
{
	if (CheckResource(_textureName, mTextures))
	{
		mTextures[_textureName];
	}
}

void GraphicsResourceContainer::Erase_Font(std::string _fontName)
{
	if (CheckResource(_fontName, mFont))
	{
		delete mFont[_fontName];
		mFont.erase(_fontName);
	}
}

ComPtr<ID3D11Buffer> GraphicsResourceContainer::Get_VertexBuffer(std::string _modelName)
{
	if (CheckResource(_modelName, mVertexBuffers))
	{
		return mVertexBuffers[_modelName];
	}
	else
	{
		DEBUG_LOG(_modelName<<"없음 _ Get_VertexBuffer");
		return nullptr;
	}
}

ComPtr<ID3D11Buffer> GraphicsResourceContainer::Get_IndexBuffer(std::string _modelName)
{
	if (CheckResource(_modelName, mIndexBuffers))
	{
		return mIndexBuffers[_modelName];
	}
	else
	{
		DEBUG_LOG(_modelName<<"없음 _ Get_IndexBuffer");
		return nullptr;
	}
}

unsigned int GraphicsResourceContainer::Get_NumIndex(std::string _modelName)
{
	if (CheckResource(_modelName, mNumIndex))
	{
		return mNumIndex[_modelName];
	}
	else
	{
		DEBUG_LOG(_modelName << "없음 _ Get_NumIndex");
		return 0;
	}
}

Model* GraphicsResourceContainer::Get_ModelInfo(std::string _modelName)
{
	if (CheckResource(_modelName, mModels))
	{
		return mModels[_modelName];
	}
	else
	{
		DEBUG_LOG(_modelName << "없음 _ Get_ModelInfo");
		return  nullptr;
	}
}

int GraphicsResourceContainer::Get_ModelBoneIndex(std::string _modelName, std::string boneName)
{
	if (CheckResource(_modelName, mModelBoneName))
	{
		int i = 0;
		for (auto e : mModelBoneName[_modelName])
		{
			if (e == boneName)
				return i;
			else
				i++;
		}
		DEBUG_LOG("Get_ModelBoneIndex : "<< _modelName << " 모델의 일치하는 "<<boneName<<"이 없습니다.");

	}
	else
	{
		DEBUG_LOG("Get_ModelBoneIndex : 존재하는 "<<_modelName<<" 모델이 없습니다.");
		return -1;
	}
}

Animation* GraphicsResourceContainer::Get_Animation(std::string _animName)
{
	if (CheckResource(_animName, mModelAnimations))
	{
		return mModelAnimations[_animName];
	}
	else
	{
		DEBUG_LOG(_animName << "없음 _ Get_Animation");
		return  nullptr;

	}
}

ComPtr<ID3D11ShaderResourceView> GraphicsResourceContainer::Get_Textures(std::string _textureName)
{
	if (CheckResource(_textureName, mTextures))
	{
		return mTextures[_textureName];
	}
	else
	{
		DEBUG_LOG(_textureName << "없음 _ Get_Textures");
		return nullptr;
	}
}


ImFont* GraphicsResourceContainer::Get_Font(std::string _fontName)
{
	if (CheckResource(_fontName, mFont))
	{
		return mFont[_fontName];
	}
	else
	{
		DEBUG_LOG(_fontName << "없음 _ Get_Font");
	}
}

Matrix GraphicsResourceContainer::Get_TargetBoneMatrix(std::string _modelName, std::string _boneName)
{
	if (CheckResource(_modelName, mModels))
	{
		return SearchBoneMatrix(_modelName, _boneName);
	}
	else
	{
		DEBUG_LOG(_modelName<<"Get_TargetBoneMatrix - 존재하지 않는 _modelName 입니다.");
		return Matrix();
	}
}

Matrix GraphicsResourceContainer::Get_TargetBoneAboveMatrix(std::string _modelName, std::string _boneName)
{
	if (CheckResource(_modelName, mModels))
	{
		return SearchBoneAboveMatrix(_modelName, _boneName);
	}
	else
	{
		DEBUG_LOG(_modelName << "Get_TargetBoneAboveMatrix - 존재하지 않는 _modelName 입니다.");
		return Matrix();
	}
}

Matrix GraphicsResourceContainer::Get_TargetBoneAboveMatrix(std::string _modelName, int _index)
{
	if (CheckResource(_modelName, mModels))
	{
		return SearchBoneAboveMatrix(_modelName, _index);
	}
	else
	{
		DEBUG_LOG(_modelName << "Get_TargetBoneAboveMatrix - 존재하지 않는 _modelName 입니다.");
		return Matrix();
	}
}

//특정 모델의 본의 메트릭스를 받아와야함
Matrix GraphicsResourceContainer::SearchBoneMatrix(std::string _modelName, std::string _boneName)
{
	auto& tempModel = mModels[_modelName];
	for (int i = 0; i < tempModel->mNumMesh; i++)
	{
		auto& tempMesh = tempModel->mMeshData[i];
		for (int j = 0; j < tempMesh.mNumBones; j++)
		{
			if (tempMesh.mBone[j].mBoneName == _boneName)
			{
				return /*tempMesh.mBone[j].mOffsetMatrix**/ tempMesh.mBone[j].mTargetNode->mTransformation* tempMesh.mBone[j].mTargetNode->mWorldTransformation;
			}
		}
	}
	DEBUG_LOG("SearchBoneMatrix - _boneName이 없습니다.");
	return Matrix();
}

Matrix GraphicsResourceContainer::SearchBoneAboveMatrix(std::string _modelName, std::string _boneName)
{
	auto& tempModel = mModels[_modelName];
	for (int i = 0; i < tempModel->mNumMesh; i++)
	{
		auto& tempMesh = tempModel->mMeshData[i];
		for (int j = 0; j < tempMesh.mNumBones; j++)
		{
			if (tempMesh.mBone[j].mBoneName == _boneName)
			{
				return tempMesh.mBone[j].mTargetNode->mTransformation * tempMesh.mBone[j].mTargetNode->mWorldTransformation;
			}
		}
	}
	DEBUG_LOG("SearchBoneMatrix - _boneName이 없습니다.");
	return Matrix();
}

inline Matrix GraphicsResourceContainer::SearchBoneAboveMatrix(std::string _modelName, int _index)
{
	auto& tempModel = mModels[_modelName];
	auto& tempMesh = tempModel->mMeshData[0];

	return tempMesh.mBone[_index].mTargetNode->mTransformation * tempMesh.mBone[_index].mTargetNode->mWorldTransformation;
}
