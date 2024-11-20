#include "FBXLoadHelper.h"

Model* FBXLoadHelper::CopyModeldata(const aiScene* _pScene)
{
	Model* tempModel = new Model();

	if (_pScene->mRootNode)
	{
		tempModel->mRootNode = new Node;
		ProcessNode(_pScene->mRootNode, tempModel->mRootNode);
	}

	if (_pScene->mNumMeshes)
	{
		tempModel->mNumMesh = _pScene->mNumMeshes;
		//tempModel->mBasePath = _basePath;
		Mesh* tempMeshArr = new Mesh[tempModel->mNumMesh];
		tempModel->mMeshData = tempMeshArr;
		for (unsigned int i = 0; i < tempModel->mNumMesh; i++)
		{
			if (_pScene->mMeshes[i]->mNumVertices)
			{
				const aiMesh* currentMesh = _pScene->mMeshes[i];
				tempMeshArr[i].mMeshName = currentMesh->mName.C_Str();
				tempMeshArr[i].mNumVertices = currentMesh->mNumVertices;
				tempMeshArr[i].mVertices = new Vertex[tempMeshArr[i].mNumVertices];     //보유한 버텍스만큼 메모리 할당

				Copy_AABB(currentMesh, &tempMeshArr[i]);

				Copy_Vertex(currentMesh, &tempMeshArr[i]);
				Copy_Index(currentMesh, &tempMeshArr[i]);
				//Copy_Texture(currentMesh, _pScene, tempMeshArr[i].mTextureName);

				tempMeshArr[i].mNumBones = currentMesh->mNumBones;
				Copy_Bone(currentMesh, tempMeshArr[i].mBone, tempModel->mRootNode);
				SetVertexesBoneInfo(tempMeshArr[i]);
			}
		}
	}
	return tempModel;
}

void FBXLoadHelper::Copy_Vertex(const aiMesh* _aiMesh, Mesh* _mesh)
{
	unsigned int numTextureCoords = GetNumTextureCoords(_aiMesh);
	for (unsigned int i = 0; i < _mesh->mNumVertices; i++)
	{
		_mesh->mVertices[i].mPosition.x = _aiMesh->mVertices[i].x;
		_mesh->mVertices[i].mPosition.y = _aiMesh->mVertices[i].y;
		_mesh->mVertices[i].mPosition.z = _aiMesh->mVertices[i].z;

		_mesh->mVertices[i].mNormal.x = _aiMesh->mNormals[i].x;
		_mesh->mVertices[i].mNormal.y = _aiMesh->mNormals[i].y;
		_mesh->mVertices[i].mNormal.z = _aiMesh->mNormals[i].z;

		//AABB세팅
		auto& aabb = _mesh->mAABB;
		const auto& vertex = _mesh->mVertices[i].mPosition;
		if (vertex.x < aabb.mMin.x)  aabb.mMin.x = vertex.x;
		if (vertex.y < aabb.mMin.y)  aabb.mMin.y = vertex.y;
		if (vertex.z < aabb.mMin.z)  aabb.mMin.z = vertex.z;

		if (vertex.x > aabb.mMax.x)	aabb.mMax.x = vertex.x;
		if (vertex.y > aabb.mMax.y)	aabb.mMax.y = vertex.y;
		if (vertex.z > aabb.mMax.z)	aabb.mMax.z = vertex.z;

		// 텍스처 좌표 복사
		for (unsigned int j = 0; j < numTextureCoords; ++j)
		{
			if (_aiMesh->mTextureCoords[j])
			{
				_mesh->mVertices[i].mTexcoords[j].x = _aiMesh->mTextureCoords[j][i].x;
				_mesh->mVertices[i].mTexcoords[j].y = _aiMesh->mTextureCoords[j][i].y;
			}
		}
	}
}

#include <filesystem>
void FBXLoadHelper::Copy_Texture(const aiMesh* _aimesh, const aiScene* _pScene, std::string*& _textureName)
{
	if (_aimesh->mMaterialIndex >= 0)									//메쉬가 하나 이상의 재질 정보를 가지고 있는지 확인
	{
		aiMaterial* pMaterial = _pScene->mMaterials[_aimesh->mMaterialIndex];	//재질 인데스를 사용하여 _scene의 재질 배열(mMaterialIndex)에서 해당 메쉬의 재질 객체에 접근.
		int index = 0;

		//Material은 추후 더 추가할수도 있다.
		int materialNum = pMaterial->GetTextureCount(aiTextureType_DIFFUSE)
						+ pMaterial->GetTextureCount(aiTextureType_NORMALS);

		if (materialNum)
		{
			_textureName = new std::string[materialNum];
			for (int i = 0; i < pMaterial->GetTextureCount(aiTextureType_DIFFUSE); i++)
			{
				aiString aiFilePath;
				pMaterial->GetTexture(aiTextureType_DIFFUSE, i, &aiFilePath);	//diffuse텍스쳐가 존재한다면 첫번째 diffuse파일 경로를 aiString 타입의 aiFilePath에 저장.
				if (index < materialNum)
				{
					_textureName[index++] = std::filesystem::path(aiFilePath.C_Str()).filename().string();
				}
			}
			for (int i = 0; i < pMaterial->GetTextureCount(aiTextureType_NORMALS); i++)
			{
				aiString aiFilePath;
				pMaterial->GetTexture(aiTextureType_NORMALS, i, &aiFilePath);	//diffuse텍스쳐가 존재한다면 첫번째 diffuse파일 경로를 aiString 타입의 aiFilePath에 저장.
				if (index < materialNum)
				{
					_textureName[index++] = std::filesystem::path(aiFilePath.C_Str()).filename().string();
				}
			}
		}
	}
}

#include <iostream>
void FBXLoadHelper::Copy_Bone(const aiMesh* _aiMesh, Bone*& _bone, Node* _targetNode)
{
	_bone = new Bone[_aiMesh->mNumBones];
	for (int i = 0; i < _aiMesh->mNumBones; i++)
	{
		auto& aiBone = _aiMesh->mBones[i];

		_bone[i].mBoneName = aiBone->mName.C_Str();
		Copy_Matrix(aiBone->mOffsetMatrix, _bone[i].mOffsetMatrix);

		_bone[i].mNumWeight = aiBone->mNumWeights;
		_bone[i].mpVertexWeight = new VertexWeight[_bone[i].mNumWeight];
		for (int s = 0; s < aiBone->mNumWeights; s++)
		{
			_bone[i].mpVertexWeight[s].mVertexId = aiBone->mWeights[s].mVertexId;
			_bone[i].mpVertexWeight[s].mWeight = aiBone->mWeights[s].mWeight;
		}
		std::string targetName = aiBone->mName.C_Str();
		_bone[i].mTargetNode = GetTargetNode(targetName, _targetNode);
	}

}

void FBXLoadHelper::SetVertexesBoneInfo(Mesh& _mesh)
{
	for (unsigned int i = 0; i < _mesh.mNumBones; i++)
	{
		auto& bone = _mesh.mBone[i];

		for (int j = 0; j < bone.mNumWeight; j++)
		{
			unsigned int vertexIndex = bone.mpVertexWeight[j].mVertexId;
			float weight = bone.mpVertexWeight[j].mWeight;

			// 정점의 본 인덱스 배열에서 미할당된(=UINT_MAX) 슬롯을 찾습니다.
			for (unsigned int boneIndex = 0; boneIndex < MAX_BONES_INDICE; boneIndex++)
			{
				if (_mesh.mVertices[vertexIndex].mBoneIndex[boneIndex] == UINT_MAX)
				{
					_mesh.mVertices[vertexIndex].mBoneIndex[boneIndex] = i; // 현재 본의 인덱스를 할당합니다.
					_mesh.mVertices[vertexIndex].mBoneWeights[boneIndex] = weight; // 해당 본의 가중치를 할당합니다.
					break;
				}
			}
		}
	}
}

void FBXLoadHelper::Copy_Matrix(const aiMatrix4x4& _aiMatrix, Matrix& _matrix)
{
	_matrix._11 = _aiMatrix.a1;
	_matrix._12 = _aiMatrix.b1;
	_matrix._13 = _aiMatrix.c1;
	_matrix._14 = _aiMatrix.d1;
	_matrix._21 = _aiMatrix.a2;
	_matrix._22 = _aiMatrix.b2;
	_matrix._23 = _aiMatrix.c2;
	_matrix._24 = _aiMatrix.d2;
	_matrix._31 = _aiMatrix.a3;
	_matrix._32 = _aiMatrix.b3;
	_matrix._33 = _aiMatrix.c3;
	_matrix._34 = _aiMatrix.d3;
	_matrix._41 = _aiMatrix.a4;
	_matrix._42 = _aiMatrix.b4;
	_matrix._43 = _aiMatrix.c4;
	_matrix._44 = _aiMatrix.d4;

}

Animation* FBXLoadHelper::CopyAnimation(const aiScene* _pScene)
{
	if (_pScene->HasAnimations())
	{
		Animation* tempAnimation = new Animation[_pScene->mNumAnimations]();
		for (int i = 0; i < _pScene->mNumAnimations; i++)
		{
			auto& aiAnimation = _pScene->mAnimations[i];
			tempAnimation[i].mName = aiAnimation->mName.C_Str();
			tempAnimation[i].mDuration = aiAnimation->mDuration;
			tempAnimation[i].mTicksPerSecond = aiAnimation->mTicksPerSecond;
			tempAnimation[i].mNumChannels = aiAnimation->mNumChannels;
			tempAnimation[i].mChannels = new NodeAnimation[tempAnimation[i].mNumChannels];
			for (int j = 0; j < tempAnimation->mNumChannels; j++) 
			{
				auto& nodeAnim = tempAnimation->mChannels[j];
				auto& aiNodeAnim = aiAnimation->mChannels[j];
				std::string targetName = aiNodeAnim->mNodeName.C_Str();
				Copy_NodeAnimationInfo(aiNodeAnim, nodeAnim);
			}
		}
		return tempAnimation;
	}
	else
	{
	return nullptr;
	}
}

void FBXLoadHelper::Copy_NodeAnimationInfo(const aiNodeAnim* _aiNodeAnim, NodeAnimation& _nodeAnim)
{
	_nodeAnim.mName = _aiNodeAnim->mNodeName.C_Str();
	_nodeAnim.mNumPosKeys = _aiNodeAnim->mNumPositionKeys;
	_nodeAnim.mNumRotKeys = _aiNodeAnim->mNumRotationKeys;
	_nodeAnim.mNumScaKeys = _aiNodeAnim->mNumScalingKeys;
	_nodeAnim.mPosKey = new VecKey[_nodeAnim.mNumPosKeys];
	_nodeAnim.mRotKey = new QuatKey[_nodeAnim.mNumRotKeys];
	_nodeAnim.mScaKey = new VecKey[_nodeAnim.mNumScaKeys];
	for (int i = 0; i < _nodeAnim.mNumPosKeys; i++)
	{
		_nodeAnim.mPosKey[i].mTime = _aiNodeAnim->mPositionKeys[i].mTime;
		_nodeAnim.mPosKey[i].mValue.x = _aiNodeAnim->mPositionKeys[i].mValue.x;
		_nodeAnim.mPosKey[i].mValue.y = _aiNodeAnim->mPositionKeys[i].mValue.y;
		_nodeAnim.mPosKey[i].mValue.z = _aiNodeAnim->mPositionKeys[i].mValue.z;
	}
	for (int i = 0; i < _nodeAnim.mNumPosKeys; i++)
	{
		_nodeAnim.mRotKey[i].mTime = _aiNodeAnim->mPositionKeys[i].mTime;
		_nodeAnim.mRotKey[i].mValue.x = _aiNodeAnim->mRotationKeys[i].mValue.x;
		_nodeAnim.mRotKey[i].mValue.y = _aiNodeAnim->mRotationKeys[i].mValue.y;
		_nodeAnim.mRotKey[i].mValue.z = _aiNodeAnim->mRotationKeys[i].mValue.z;
		_nodeAnim.mRotKey[i].mValue.w = _aiNodeAnim->mRotationKeys[i].mValue.w;
	}
	for (int i = 0; i < _nodeAnim.mNumScaKeys; i++)
	{
		_nodeAnim.mScaKey[i].mTime = _aiNodeAnim->mScalingKeys[i].mTime;
		_nodeAnim.mScaKey[i].mValue.x = _aiNodeAnim->mScalingKeys[i].mValue.x;
		_nodeAnim.mScaKey[i].mValue.y = _aiNodeAnim->mScalingKeys[i].mValue.y;
		_nodeAnim.mScaKey[i].mValue.z = _aiNodeAnim->mScalingKeys[i].mValue.z;
	}
}

void FBXLoadHelper::Copy_AABB(const aiMesh* _aimesh, Mesh* _mesh)
{
	_mesh->mAABB.mMin.x = _aimesh->mAABB.mMin.x;
	_mesh->mAABB.mMin.y = _aimesh->mAABB.mMin.y;
	_mesh->mAABB.mMin.z = _aimesh->mAABB.mMin.z;

	_mesh->mAABB.mMax.x = _aimesh->mAABB.mMax.x;
	_mesh->mAABB.mMax.y = _aimesh->mAABB.mMax.y;
	_mesh->mAABB.mMax.z = _aimesh->mAABB.mMax.z;
}

unsigned int FBXLoadHelper::GetNumTextureCoords(const aiMesh* mesh)
{
	unsigned int numTextureCoords = 0;
	for (unsigned int i = 0; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++i)
	{
		if (mesh->mTextureCoords[i] != nullptr)
		{
			++numTextureCoords;
		}
	}
	return numTextureCoords;
}

Node* FBXLoadHelper::GetTargetNode(std::string& _name, Node* _node)
{
	if (_name == _node->mNodeName)
	{
		return _node;
	}

	// 자식 노드를 재귀적으로 탐색
	for (int i =0; i < _node->mNumNextNode; i++)
	{
		Node* targetNode = GetTargetNode(_name, &_node->mNextNode[i]);
		if (targetNode)
		{
			return targetNode;
		}
	}

	return nullptr; // 노드를 찾지 못한 경우
}
