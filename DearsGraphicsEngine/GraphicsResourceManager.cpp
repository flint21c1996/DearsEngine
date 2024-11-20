#include "GraphicsResourceManager.h"
#include <directxtk/DDSTextureLoader.h> // 큐브맵 읽을 때 필요

#ifdef _DEBUG
#include <iostream>
#define DEBUG_LOG(message) std::cout<<message<<std::endl
#endif // DEBUG

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GraphicsResourceManager::GraphicsResourceManager(ComPtr<ID3D11Device> _pDevice, ComPtr<ID3D11DeviceContext> _pDeviceContext)
{
	mpFBXLoader = new FBXLoader();
	mpGraphicsResourceContainer = new GraphicsResourceContainer();
	mpDevice = _pDevice;
	mpDeviceContext = _pDeviceContext;
}

GraphicsResourceManager::~GraphicsResourceManager()
{
	delete mpFBXLoader;
	delete mpGraphicsResourceContainer; // 메모리 해제
}

void GraphicsResourceManager::Initialize()
{

}

void GraphicsResourceManager::Update()
{

}

void GraphicsResourceManager::Finalize()
{

}

ComPtr<ID3D11Buffer> GraphicsResourceManager::Get_VertexBuffer(std::string _modelName)
{
	return mpGraphicsResourceContainer->Get_VertexBuffer(_modelName);
}

ComPtr<ID3D11Buffer> GraphicsResourceManager::Get_IndexBuffer(std::string _modelName)
{
	return mpGraphicsResourceContainer->Get_IndexBuffer(_modelName);
}

unsigned int GraphicsResourceManager::Get_NumIndex(std::string _modelName)
{
	return mpGraphicsResourceContainer->Get_NumIndex(_modelName);
}

Model* GraphicsResourceManager::Get_ModelInfo(std::string _modelName)
{
	return mpGraphicsResourceContainer->Get_ModelInfo(_modelName);
}

Animation* GraphicsResourceManager::Get_Animation(std::string _modelName)
{
	return mpGraphicsResourceContainer->Get_Animation(_modelName);
}

ComPtr<ID3D11ShaderResourceView> GraphicsResourceManager::Get_Textures(std::string _modelName)
{
	return mpGraphicsResourceContainer->Get_Textures(_modelName);
}

ImFont* GraphicsResourceManager::Get_Font(std::string _modelName)
{
	return mpGraphicsResourceContainer->Get_Font(_modelName);
}

Matrix GraphicsResourceManager::Get_TargetBoneMatrix(std::string _modelName, std::string _boneName)
{
	return mpGraphicsResourceContainer->Get_TargetBoneMatrix(_modelName,_boneName);
}

Matrix GraphicsResourceManager::Get_TargetBoneAboveMatrix(std::string _modelName, std::string _boneName)
{
	return mpGraphicsResourceContainer->Get_TargetBoneAboveMatrix(_modelName, _boneName);
}

Matrix GraphicsResourceManager::Get_TargetBoneAboveMatrix(std::string _modelName, int _index)
{
	return mpGraphicsResourceContainer->Get_TargetBoneAboveMatrix(_modelName, _index);
}

int GraphicsResourceManager::Get_TargetModelBoneIndex(std::string _modelName, std::string _boneName)
{
	return mpGraphicsResourceContainer->Get_ModelBoneIndex(_modelName, _boneName);
}

void GraphicsResourceManager::Erase_VertexBuffer(std::string _modelName)
{
	mpGraphicsResourceContainer->Erase_VertexBuffer(_modelName);
}

void GraphicsResourceManager::Erase_IndexBuffer(std::string _modelName)
{
	mpGraphicsResourceContainer->Erase_IndexBuffer(_modelName);
}

void GraphicsResourceManager::Erase_NumIndex(std::string _modelName)
{
	mpGraphicsResourceContainer->Erase_NumIndex(_modelName);
}

void GraphicsResourceManager::Erase_ModelInfo(std::string _modelName)
{
	mpGraphicsResourceContainer->Erase_ModelInfo(_modelName);
}

void GraphicsResourceManager::Erase_Animation(std::string _animName)
{
	mpGraphicsResourceContainer->Erase_Animation(_animName);
}

void GraphicsResourceManager::Erase_Textures(std::string _modelName)
{
	mpGraphicsResourceContainer->Erase_Textures(_modelName);
}

void GraphicsResourceManager::Erase_Font(std::string _modelName)
{
	mpGraphicsResourceContainer->Erase_Font(_modelName);
}

void GraphicsResourceManager::AddModel(std::string _basePath, std::string _fileName)
{
	if (Model* tempModel = mpFBXLoader->LoadModel(_basePath + _fileName))
	{
		if (tempModel)
		{
			for (int i = 0; i < tempModel->mNumMesh; i++)
			{
				auto& mesh = tempModel->mMeshData[i];
				if (mesh.mVertices)
				{
					ComPtr<ID3D11Buffer> tempVertexBuffer = RendererHelper::CreateVertexBuffer(mpDevice, mesh.mNumVertices, mesh.mVertices);
					mpGraphicsResourceContainer->Add_VertexBuffer(mesh.mMeshName, tempVertexBuffer);
					//delete[] mesh.mVertices;
					//mesh.mVertices = nullptr;
				}
				if (mesh.mIndices)
				{
					ComPtr<ID3D11Buffer> tempIndexBuffer = RendererHelper::CreateIndexBuffer(mpDevice, mesh.mNumIndices, mesh.mIndices);
					mpGraphicsResourceContainer->Add_IndexBuffer(mesh.mMeshName, tempIndexBuffer);
					mpGraphicsResourceContainer->Add_NumIndex(mesh.mMeshName, mesh.mNumIndices);
				}
				if (mesh.mBone)
				{
					for (unsigned int j = 0; j < mesh.mNumBones; j++)
					{
						auto& bone = mesh.mBone[j];
						mpGraphicsResourceContainer->Add_BoneName(mesh.mMeshName, bone.mBoneName);
						//DEBUG_LOG(bone.mBoneName);
					}
				}
			}
					mpGraphicsResourceContainer->Add_ModelInfo(_fileName, tempModel);
		}
	}
}

void GraphicsResourceManager::AddModel(MeshData _meshData, std::string _meshName)
{
	ComPtr<ID3D11Buffer> tempVertexBuffer = RendererHelper::CreateVertexBuffer(mpDevice, _meshData.vertices.size(), _meshData.vertices.data());
	mpGraphicsResourceContainer->Add_VertexBuffer(_meshName, tempVertexBuffer);

	ComPtr<ID3D11Buffer> tempIndexBuffer = RendererHelper::CreateIndexBuffer(mpDevice, _meshData.indices.size(), _meshData.indices.data());
	mpGraphicsResourceContainer->Add_IndexBuffer(_meshName, tempIndexBuffer);
	mpGraphicsResourceContainer->Add_NumIndex(_meshName, _meshData.indices.size());

}

void GraphicsResourceManager::AddAnimation(std::string _basePath, std::string _fileName)
{
	Animation* tempAnimation = mpFBXLoader->LoadAnimation(_basePath + _fileName);
	mpGraphicsResourceContainer->Add_Animation(_fileName, tempAnimation);
}


void GraphicsResourceManager::Add3DTexture(std::string _basePath, std::string _fileName)
{
	auto texture = RendererHelper::Create3DTexture(mpDevice, _basePath + _fileName);
	mpGraphicsResourceContainer->Add_Textures(_fileName, texture);
}

void GraphicsResourceManager::Add2DTexture(std::string _basePath, std::string _fileName)
{
	auto texture = RendererHelper::Create2DTexture(mpDevice, _basePath + _fileName);
	mpGraphicsResourceContainer->Add_Textures(_fileName, texture);
}

void GraphicsResourceManager::AddDDSTexture(std::string _basePath, std::string _fileName)
{
	auto texture = RendererHelper::CreateDDSTexture(mpDevice, _basePath + _fileName);
	mpGraphicsResourceContainer->Add_Textures(_fileName, texture);
}

void GraphicsResourceManager::Add_Font(std::string fontName, ImFont* _font)
{
	mpGraphicsResourceContainer->Add_Font(fontName, _font);
}

