#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include"ModelInfo.h"

///FBX로드를 도와줄 클래스.
///안의 함수를 template화 시키자.
using DirectX::SimpleMath::Matrix;

class FBXLoadHelper
{
public:
	/// <summary>
	/// assimp 모델의 데이터를 복사한다.
	/// </summary>
	/// <param name="_pScene"></param>
	/// <returns></returns>
	Model* CopyModeldata(const aiScene* _pScene);

	/// <summary>
	/// 버텍스를 복사한다.
	/// </summary>
	/// <param name="_aiMesh"></param>
	/// <param name="_mesh"></param>
	void Copy_Vertex(const aiMesh* _aiMesh, Mesh* _mesh);

	/// <summary>
	/// 인덱스를 복사한다.
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="_aiMesh"></param>
	/// <param name="_mesh"></param>
	template <typename T>
	void Copy_Index(const aiMesh* _aiMesh, T* _mesh);

	/// <summary>
	/// 텍스쳐를 복사한다.
	/// </summary>
	/// <param name="_aimesh"></param>
	/// <param name="_pScene"></param>
	/// <param name="_texturename"></param>
	void Copy_Texture(const aiMesh* _aimesh, const aiScene* _pScene, std::string*& _texturename);

	/// <summary>
	/// 본을 복사한다.
	/// </summary>
	/// <param name="_aiMesh"></param>
	/// <param name="_bone"></param>
	/// <param name="_targetNode"></param>
	void Copy_Bone(const aiMesh* _aiMesh, Bone*& _bone, Node* _targetNode);

	/// <summary>
	/// 노드의 정보를 세팅하고 복사한다.
	/// </summary>
	/// <typeparam name="Node"></typeparam>
	/// <param name="_ainode"></param>
	/// <param name="_node"></param>
	template<typename Node>
	void ProcessNode(const aiNode* _ainode, Node* _node);

	/// <summary>
	/// 버텍스가 가중치를 받는 본을 세팅한다.
	/// </summary>
	/// <param name="_mesh"></param>
	void SetVertexesBoneInfo(Mesh& _mesh);

	/// <summary>
	/// aiMatrix를 매트릭스를 SimpleMath::Matrix로 복사한다.
	/// </summary>
	/// <param name="_aiMatrix"></param>
	/// <param name="_matrix"></param>
	void Copy_Matrix(const aiMatrix4x4& _aiMatrix, Matrix& _matrix);

	/// <summary>
	/// 애니메이션 정보를 복사해 반환한다.
	/// </summary>
	/// <param name="_pScene"></param>
	/// <returns></returns>
	Animation* CopyAnimation(const aiScene* _pScene);

	/// <summary>
	/// NOdeAnimation의 정보를 복사한다. CopyAnimation함수에서 사용한다.
	/// </summary>
	/// <param name="_aiNodeAnim"></param>
	/// <param name="_nodeAnim"></param>
	void Copy_NodeAnimationInfo(const aiNodeAnim* _aiNodeAnim, NodeAnimation& _nodeAnim);

	/// <summary>
	/// 메쉬의 AABB를 복사한다.
	/// </summary>
	/// <param name="_aiNodeAnim"></param>
	/// <param name="_AABB"></param>
	void Copy_AABB(const aiMesh* _aiNodeAnim, Mesh* _AABB);
private:
	unsigned int GetNumTextureCoords(const aiMesh* mesh);
	Node* GetTargetNode(std::string& _name, Node* _node);

};



template <typename T>
void FBXLoadHelper::Copy_Index(const aiMesh* _aiMesh, T* _mesh)
{
	_mesh->mNumIndices = _aiMesh->mNumFaces * 3;	// 모든 면이 삼각형
	_mesh->mIndices = new unsigned int[_mesh->mNumIndices];

	unsigned int index = 0;
	for (unsigned int i = 0; i < _aiMesh->mNumFaces; i++)
	{
		const aiFace& face = _aiMesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			if (index < _mesh->mNumIndices)		// 버퍼 오버런 방지
			{
				_mesh->mIndices[index++] = face.mIndices[j];
			}
		}
	}
}

template<typename Node>
void FBXLoadHelper::ProcessNode(const aiNode* _ainode, Node* _node)
{
	_node->mNodeName = _ainode->mName.C_Str();
	Copy_Matrix(_ainode->mTransformation, _node->mTransformation);
	_node->mNumNextNode = _ainode->mNumChildren;
	_node->mNumMeshes = _ainode->mNumMeshes;
	_node->mMeshes = new unsigned int[_node->mNumMeshes];
	for (int i = 0; i < _node->mNumMeshes; i++)
	{
		_node->mMeshes[i] = _ainode->mMeshes[i];
	}
	if (_node->mNumNextNode)
	{
		_node->mNextNode = new Node[_node->mNumNextNode];

		for (int i = 0; i < _node->mNumNextNode; i++)
		{
			ProcessNode(_ainode->mChildren[i], &_node->mNextNode[i]);
		}
	}
}