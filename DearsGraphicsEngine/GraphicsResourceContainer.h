#pragma once
#include <wrl/client.h>
#include <string>
#include <d3d11.h>
#include <unordered_map>
#include <map>
#include <imgui.h>
#include "ModelBuffer.h"
#include "ModelInfo.h"
#include "BufferData.h"
#include "ObjectPool.h"
#include "BufferEnum.h"
#include "Texture.h"
#include "UIEnum.h"

#ifdef _DEBUG
#include <iostream>
#define DEBUG_LOG(message) std::cout<<message<<std::endl
#endif // DEBUG

#define BASIC_SIZE 10

using Microsoft::WRL::ComPtr;

///그래픽스 리소스를 저장할 컨테이너***
///리소스의 저장, 삭제만 한다.
class GraphicsResourceContainer
{
public:
	GraphicsResourceContainer();
	~GraphicsResourceContainer();

	//Constant Buffer -> 렌더 시에 직접적으로 사용된다.
private:
	//[모델명, struct{VertexBuffer, IndexCount}], 버텍스 버퍼와 인덱스 버퍼는 하나를 돌려써도 된다.
	std::unordered_map<std::string, ComPtr<ID3D11Buffer>> mVertexBuffers;

	//[모델명, struct{IndexBuffer, IndexCount}], 버텍스 버퍼와 인덱스 버퍼는 하나를 돌려써도 된다.
	std::unordered_map<std::string, ComPtr<ID3D11Buffer>> mIndexBuffers;
	std::unordered_map<std::string, unsigned int> mNumIndex;

	//[모델명, node*], 노드 계산을 위해 필요하다
	std::unordered_map<std::string, Model*> mModels;

	//[애니메이션 이름, Animation*]
	std::unordered_map<std::string, Animation*> mModelAnimations;

	//[텍스쳐 이름, ID3D11ShaderResourceView]
	std::unordered_map<std::string, ComPtr<ID3D11ShaderResourceView>> mTextures;

	//[Font 이름, ImFont*] 폰트 저장
	std::unordered_map<std::string, ImFont*> mFont;

	//[모델명, 본이름 배열]
	std::unordered_map<std::string, std::vector<std::string>> mModelBoneName;

public:
	//리소스 들을 추가한다.
	void Add_VertexBuffer(std::string _modelName, ComPtr<ID3D11Buffer>_vertexBuffers);
	void Add_IndexBuffer(std::string _modelName, ComPtr<ID3D11Buffer>_indexBuffers);
	void Add_NumIndex(std::string _modelName, unsigned int _numIndex);
	void Add_ModelInfo(std::string _modelName, Model* _rootNode);
	void Add_BoneName(std::string _modelName, std::string _boneName);
	void Add_Animation(std::string _animName, Animation* _animation);
	void Add_Textures(std::string _textureName, ComPtr<ID3D11ShaderResourceView> _shaderResourceView);
	void Add_Font(std::string _fontName, ImFont* _font);

	//특정 리소스를 지운다.
	void Erase_VertexBuffer(std::string _modelName);
	void Erase_IndexBuffer(std::string _modelName);
	void Erase_NumIndex(std::string _modelName);
	void Erase_ModelInfo(std::string _modelName);
	void Erase_BoneNameVec(std::string _modelName);
	void Erase_Animation(std::string _animName);
	void Erase_Textures(std::string _textureName);
	void Erase_Font(std::string _fontName);

	//특정 리소스의 정보를 내보낸다.
	ComPtr<ID3D11Buffer> Get_VertexBuffer(std::string _modelName);
	ComPtr<ID3D11Buffer> Get_IndexBuffer(std::string _modelName);
	unsigned int Get_NumIndex(std::string _modelName);
	Model* Get_ModelInfo(std::string _modelName);
	int Get_ModelBoneIndex(std::string _modelName, std::string boneName);
	Animation* Get_Animation(std::string _animName);
	ComPtr<ID3D11ShaderResourceView> Get_Textures(std::string _textureName);
	ImFont* Get_Font(std::string _fontName);
	Matrix Get_TargetBoneMatrix(std::string _modelName, std::string _boneName);
	Matrix Get_TargetBoneAboveMatrix(std::string _modelName, std::string _boneName);
	Matrix Get_TargetBoneAboveMatrix(std::string _modelName, int _index);

private:
	Matrix SearchBoneMatrix(std::string _modelName, std::string _boneName);
	Matrix SearchBoneAboveMatrix(std::string _modelName, std::string _boneName);
	inline Matrix SearchBoneAboveMatrix(std::string _modelName, int _index);
private:
	template<typename Resource, typename Key>
	bool CheckResource(const Key& _key, const Resource& _resource);
};


template<typename Resource, typename Key>
bool GraphicsResourceContainer::CheckResource(const Key& _key, const Resource& _resource)
{
	const auto& iter = _resource.find(_key);
	//이미 있는 모델일 경우
	if (iter != _resource.end())
	{
		return true;
	}
	//모델이 존재하지 않을 경우
	else
	{
		return false;
	}
}
