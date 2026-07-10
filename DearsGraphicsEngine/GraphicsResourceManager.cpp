#include "GraphicsResourceManager.h"
#include <directxtk/DDSTextureLoader.h>

#ifdef _DEBUG
#include <iostream>
#define DEBUG_LOG(message) std::cout << message << std::endl
#else
#define DEBUG_LOG(message)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace
{
Model* CreateModelFromMeshData(const MeshData& meshData, const std::string& meshName)
{
    // GeometryGenerator가 만든 MyBox/MySphere 같은 기본 도형은 FBX 파일이 아니다.
    // 그래도 피킹은 CPU에 남아있는 정점/AABB 정보가 필요하므로,
    // GPU 버퍼를 만들 때 같은 데이터로 간단한 ModelInfo도 함께 만들어 둔다.
    Model* model = new Model();
    model->mNumMesh = 1;
    model->mMeshData = new Mesh[1];

    Mesh& mesh = model->mMeshData[0];
    mesh.mMeshName = meshName;
    mesh.mNumVertices = static_cast<unsigned int>(meshData.vertices.size());
    mesh.mNumIndices = static_cast<unsigned int>(meshData.indices.size());

    if (mesh.mNumVertices > 0)
    {
        mesh.mVertices = new Vertex[mesh.mNumVertices];
        for (unsigned int i = 0; i < mesh.mNumVertices; ++i)
        {
            mesh.mVertices[i] = meshData.vertices[i];

            const Vector3& position = mesh.mVertices[i].mPosition;
            mesh.mAABB.mMin.x = (std::min)(mesh.mAABB.mMin.x, position.x);
            mesh.mAABB.mMin.y = (std::min)(mesh.mAABB.mMin.y, position.y);
            mesh.mAABB.mMin.z = (std::min)(mesh.mAABB.mMin.z, position.z);

            mesh.mAABB.mMax.x = (std::max)(mesh.mAABB.mMax.x, position.x);
            mesh.mAABB.mMax.y = (std::max)(mesh.mAABB.mMax.y, position.y);
            mesh.mAABB.mMax.z = (std::max)(mesh.mAABB.mMax.z, position.z);
        }
    }

    if (mesh.mNumIndices > 0)
    {
        mesh.mIndices = new unsigned int[mesh.mNumIndices];
        for (unsigned int i = 0; i < mesh.mNumIndices; ++i)
        {
            mesh.mIndices[i] = meshData.indices[i];
        }
    }

    return model;
}
}

GraphicsResourceManager::GraphicsResourceManager(ComPtr<ID3D11Device> _pDevice, ComPtr<ID3D11DeviceContext> _pDeviceContext)
    : mpDevice(_pDevice)
    , mpDeviceContext(_pDeviceContext)
    , mpFBXLoader(std::make_unique<FBXLoader>())
{
}

GraphicsResourceManager::~GraphicsResourceManager()
{
    for (auto& pair : mModelAnimations)
        delete[] pair.second;

    for (auto& pair : mModels)
        delete pair.second;
}

void GraphicsResourceManager::Initialize() {}
void GraphicsResourceManager::Update()     {}
void GraphicsResourceManager::Finalize()   {}

// ??? Add ????????????????????????????????????????????????????????????????????

void GraphicsResourceManager::AddModel(std::string _basePath, std::string _fileName)
{
    Model* tempModel = mpFBXLoader->LoadModel(_basePath + _fileName);
    if (!tempModel) return;

    for (int i = 0; i < tempModel->mNumMesh; i++)
    {
        auto& mesh = tempModel->mMeshData[i];

        if (mesh.mVertices && !CheckResource(mesh.mMeshName, mVertexBuffers))
        {
            mVertexBuffers[mesh.mMeshName] = RendererHelper::CreateVertexBuffer(mpDevice, mesh.mNumVertices, mesh.mVertices);
        }
        if (mesh.mIndices && !CheckResource(mesh.mMeshName, mIndexBuffers))
        {
            mIndexBuffers[mesh.mMeshName] = RendererHelper::CreateIndexBuffer(mpDevice, mesh.mNumIndices, mesh.mIndices);
            mNumIndex[mesh.mMeshName]     = mesh.mNumIndices;
        }
        if (mesh.mBone)
        {
            for (unsigned int j = 0; j < mesh.mNumBones; j++)
                mModelBoneName[mesh.mMeshName].push_back(mesh.mBone[j].mBoneName);
        }
    }

    if (!CheckResource(_fileName, mModels))
        mModels[_fileName] = tempModel;
}

void GraphicsResourceManager::AddModel(MeshData _meshData, std::string _meshName)
{
    const unsigned int vertexCount = static_cast<unsigned int>(_meshData.vertices.size());
    const unsigned int indexCount = static_cast<unsigned int>(_meshData.indices.size());

    if (!CheckResource(_meshName, mVertexBuffers))
        mVertexBuffers[_meshName] = RendererHelper::CreateVertexBuffer(mpDevice, vertexCount, _meshData.vertices.data());

    if (!CheckResource(_meshName, mIndexBuffers))
    {
        mIndexBuffers[_meshName] = RendererHelper::CreateIndexBuffer(mpDevice, indexCount, _meshData.indices.data());
        mNumIndex[_meshName] = indexCount;
    }

    if (!CheckResource(_meshName, mModels))
        mModels[_meshName] = CreateModelFromMeshData(_meshData, _meshName);
}

void GraphicsResourceManager::AddAnimation(std::string _basePath, std::string _fileName)
{
    if (!CheckResource(_fileName, mModelAnimations))
        mModelAnimations[_fileName] = mpFBXLoader->LoadAnimation(_basePath + _fileName);
}

void GraphicsResourceManager::Add3DTexture(std::string _basePath, std::string _fileName)
{
    if (!CheckResource(_fileName, mTextures))
        mTextures[_fileName] = RendererHelper::Create3DTexture(mpDevice, _basePath + _fileName);
}

void GraphicsResourceManager::Add2DTexture(std::string _basePath, std::string _fileName)
{
    if (!CheckResource(_fileName, mTextures))
        mTextures[_fileName] = RendererHelper::Create2DTexture(mpDevice, _basePath + _fileName);
}

void GraphicsResourceManager::Add2DMipMapTexture(std::string _basePath, std::string _fileName)
{
    if (!CheckResource(_fileName, mTextures))
        mTextures[_fileName] = RendererHelper::Create2DMipMapTexture(mpDevice, mpDeviceContext, _basePath + _fileName);
}

void GraphicsResourceManager::AddDDSTexture(std::string _basePath, std::string _fileName, bool isCubeMap)
{
    if (!CheckResource(_fileName, mTextures))
        mTextures[_fileName] = RendererHelper::CreateDDSTexture(mpDevice, _basePath + _fileName, isCubeMap);
}

void GraphicsResourceManager::Add_Font(std::string fontName, ImFont* _font)
{
    if (!CheckResource(fontName, mFont))
        mFont[fontName] = _font;
}

// ??? Get ????????????????????????????????????????????????????????????????????

ComPtr<ID3D11Buffer> GraphicsResourceManager::Get_VertexBuffer(std::string _modelName)
{
    if (CheckResource(_modelName, mVertexBuffers)) return mVertexBuffers[_modelName];
    DEBUG_LOG(_modelName << " not found _ Get_VertexBuffer");
    return nullptr;
}

ComPtr<ID3D11Buffer> GraphicsResourceManager::Get_IndexBuffer(std::string _modelName)
{
    if (CheckResource(_modelName, mIndexBuffers)) return mIndexBuffers[_modelName];
    DEBUG_LOG(_modelName << " not found _ Get_IndexBuffer");
    return nullptr;
}

unsigned int GraphicsResourceManager::Get_NumIndex(std::string _modelName)
{
    if (CheckResource(_modelName, mNumIndex)) return mNumIndex[_modelName];
    DEBUG_LOG(_modelName << " not found _ Get_NumIndex");
    return 0;
}

Model* GraphicsResourceManager::Get_ModelInfo(std::string _modelName)
{
    if (CheckResource(_modelName, mModels)) return mModels[_modelName];
    DEBUG_LOG(_modelName << " not found _ Get_ModelInfo");
    return nullptr;
}

Animation* GraphicsResourceManager::Get_Animation(std::string _animName)
{
    if (CheckResource(_animName, mModelAnimations)) return mModelAnimations[_animName];
    DEBUG_LOG(_animName << " not found _ Get_Animation");
    return nullptr;
}

ComPtr<ID3D11ShaderResourceView> GraphicsResourceManager::Get_Textures(std::string _textureName)
{
    if (CheckResource(_textureName, mTextures)) return mTextures[_textureName];
    DEBUG_LOG(_textureName << " not found _ Get_Textures");
    return nullptr;
}

ImFont* GraphicsResourceManager::Get_Font(std::string _fontName)
{
    if (CheckResource(_fontName, mFont)) return mFont[_fontName];
    DEBUG_LOG(_fontName << " not found _ Get_Font");
    return nullptr;
}

Matrix GraphicsResourceManager::Get_TargetBoneMatrix(std::string _modelName, std::string _boneName)
{
    if (CheckResource(_modelName, mModels)) return SearchBoneMatrix(_modelName, _boneName);
    DEBUG_LOG(_modelName << " Get_TargetBoneMatrix - not found");
    return Matrix();
}

Matrix GraphicsResourceManager::Get_TargetBoneAboveMatrix(std::string _modelName, std::string _boneName)
{
    if (CheckResource(_modelName, mModels)) return SearchBoneAboveMatrix(_modelName, _boneName);
    DEBUG_LOG(_modelName << " Get_TargetBoneAboveMatrix - not found");
    return Matrix();
}

Matrix GraphicsResourceManager::Get_TargetBoneAboveMatrix(std::string _modelName, int _index)
{
    if (CheckResource(_modelName, mModels)) return SearchBoneAboveMatrix(_modelName, _index);
    DEBUG_LOG(_modelName << " Get_TargetBoneAboveMatrix - not found");
    return Matrix();
}

int GraphicsResourceManager::Get_TargetModelBoneIndex(std::string _modelName, std::string _boneName)
{
    if (!CheckResource(_modelName, mModelBoneName))
    {
        DEBUG_LOG("Get_TargetModelBoneIndex - model not found: " << _modelName);
        return -1;
    }
    int i = 0;
    for (const auto& name : mModelBoneName[_modelName])
    {
        if (name == _boneName) return i;
        i++;
    }
    DEBUG_LOG("Get_TargetModelBoneIndex - bone not found: " << _boneName);
    return -1;
}

// ??? Erase ??????????????????????????????????????????????????????????????????

void GraphicsResourceManager::Erase_VertexBuffer(std::string _modelName)
{
    mVertexBuffers.erase(_modelName);
}

void GraphicsResourceManager::Erase_IndexBuffer(std::string _modelName)
{
    mIndexBuffers.erase(_modelName);
}

void GraphicsResourceManager::Erase_NumIndex(std::string _modelName)
{
    mNumIndex.erase(_modelName);
}

void GraphicsResourceManager::Erase_ModelInfo(std::string _modelName)
{
    auto it = mModels.find(_modelName);
    if (it != mModels.end())
    {
        delete it->second;
        mModels.erase(it);
    }
}

void GraphicsResourceManager::Erase_Animation(std::string _animName)
{
    auto it = mModelAnimations.find(_animName);
    if (it != mModelAnimations.end())
    {
        delete[] it->second;
        mModelAnimations.erase(it);
    }
}

void GraphicsResourceManager::Erase_Textures(std::string _textureName)
{
    mTextures.erase(_textureName);
}

void GraphicsResourceManager::Erase_Font(std::string _fontName)
{
    auto it = mFont.find(_fontName);
    if (it != mFont.end())
    {
        delete it->second;
        mFont.erase(it);
    }
}

// ??? ?대? 堉??먯깋 ?????????????????????????????????????????????????????????????

Matrix GraphicsResourceManager::SearchBoneMatrix(std::string _modelName, std::string _boneName)
{
    auto& tempModel = mModels[_modelName];
    for (int i = 0; i < tempModel->mNumMesh; i++)
    {
        auto& tempMesh = tempModel->mMeshData[i];
        for (int j = 0; j < tempMesh.mNumBones; j++)
        {
            if (tempMesh.mBone[j].mBoneName == _boneName)
                return tempMesh.mBone[j].mTargetNode->mTransformation
                     * tempMesh.mBone[j].mTargetNode->mWorldTransformation;
        }
    }
    DEBUG_LOG("SearchBoneMatrix - bone not found: " << _boneName);
    return Matrix();
}

Matrix GraphicsResourceManager::SearchBoneAboveMatrix(std::string _modelName, std::string _boneName)
{
    auto& tempModel = mModels[_modelName];
    for (int i = 0; i < tempModel->mNumMesh; i++)
    {
        auto& tempMesh = tempModel->mMeshData[i];
        for (int j = 0; j < tempMesh.mNumBones; j++)
        {
            if (tempMesh.mBone[j].mBoneName == _boneName)
                return tempMesh.mBone[j].mTargetNode->mTransformation
                     * tempMesh.mBone[j].mTargetNode->mWorldTransformation;
        }
    }
    DEBUG_LOG("SearchBoneAboveMatrix - bone not found: " << _boneName);
    return Matrix();
}

Matrix GraphicsResourceManager::SearchBoneAboveMatrix(std::string _modelName, int _index)
{
    auto& tempMesh = mModels[_modelName]->mMeshData[0];
    return tempMesh.mBone[_index].mTargetNode->mTransformation
         * tempMesh.mBone[_index].mTargetNode->mWorldTransformation;
}
