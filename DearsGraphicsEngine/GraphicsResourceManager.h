#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <unordered_map>
#include <string>
#include <vector>
#include "LayerEnum.h"
#include "FBXLoader.h"
#include "GeometryGenerator.h"
#include "RendererHelper.h"
#include "ModelInfo.h"
#include "BufferData.h"
#include "ObjectPool.h"
#include "BufferEnum.h"
#include <imgui.h>
#include <memory>

using Microsoft::WRL::ComPtr;

/// 洹몃옒?쎌뒪 由ъ냼?ㅼ쓽 濡쒕뵫, ??? 議고쉶, ??젣瑜??대떦.
/// FBXLoader濡??뚯씪???쎄퀬 GPU 踰꾪띁瑜??앹꽦???대? 留듭뿉 蹂닿??쒕떎.
/// (GraphicsResourceContainer瑜??≪닔 ?듯빀)
class GraphicsResourceManager
{
public:
    GraphicsResourceManager(ComPtr<ID3D11Device> _pDevice, ComPtr<ID3D11DeviceContext> _pDeviceContext);
    ~GraphicsResourceManager();

    void Initialize();
    void Update();
    void Finalize();

    // ----- 濡쒕뱶 -----
    void AddModel(std::string _basePath, std::string _fileName);
    void AddModel(MeshData _meshData, std::string _meshName);
    void AddAnimation(std::string _basePath, std::string _fileName);
    void Add3DTexture(std::string _basePath, std::string _fileName);
    void Add2DTexture(std::string _basePath, std::string _fileName);
    void Add2DMipMapTexture(std::string _basePath, std::string _fileName);
    void AddDDSTexture(std::string _basePath, std::string _fileName, bool isCubeMap = true);
    void Add_Font(std::string fontName, ImFont* _font);

    // ----- 議고쉶 -----
    ComPtr<ID3D11Buffer> Get_VertexBuffer(std::string _modelName);
    ComPtr<ID3D11Buffer> Get_IndexBuffer(std::string _modelName);
    unsigned int         Get_NumIndex(std::string _modelName);
    Model*               Get_ModelInfo(std::string _modelName);
    Animation*           Get_Animation(std::string _animName);
    ComPtr<ID3D11ShaderResourceView> Get_Textures(std::string _textureName);
    ImFont*              Get_Font(std::string _fontName);
    Matrix               Get_TargetBoneMatrix(std::string _modelName, std::string _boneName);
    Matrix               Get_TargetBoneAboveMatrix(std::string _modelName, std::string _boneName);
    Matrix               Get_TargetBoneAboveMatrix(std::string _modelName, int _index);
    int                  Get_TargetModelBoneIndex(std::string _modelName, std::string _boneName);

    // ----- ??젣 -----
    void Erase_VertexBuffer(std::string _modelName);
    void Erase_IndexBuffer(std::string _modelName);
    void Erase_NumIndex(std::string _modelName);
    void Erase_ModelInfo(std::string _modelName);
    void Erase_Animation(std::string _animName);
    void Erase_Textures(std::string _textureName);
    void Erase_Font(std::string _fontName);

private:
    ComPtr<ID3D11Device>        mpDevice;
    ComPtr<ID3D11DeviceContext> mpDeviceContext;
    std::unique_ptr<FBXLoader>  mpFBXLoader;

    // 由ъ냼????μ냼
    std::unordered_map<std::string, ComPtr<ID3D11Buffer>>              mVertexBuffers;
    std::unordered_map<std::string, ComPtr<ID3D11Buffer>>              mIndexBuffers;
    std::unordered_map<std::string, unsigned int>                      mNumIndex;
    std::unordered_map<std::string, Model*>                            mModels;
    std::unordered_map<std::string, Animation*>                        mModelAnimations;
    std::unordered_map<std::string, ComPtr<ID3D11ShaderResourceView>>  mTextures;
    std::unordered_map<std::string, ImFont*>                           mFont;
    std::unordered_map<std::string, std::vector<std::string>>          mModelBoneName;

    template<typename Resource, typename Key>
    bool CheckResource(const Key& _key, const Resource& _resource);

    Matrix SearchBoneMatrix(std::string _modelName, std::string _boneName);
    Matrix SearchBoneAboveMatrix(std::string _modelName, std::string _boneName);
    Matrix SearchBoneAboveMatrix(std::string _modelName, int _index);
};

template<typename Resource, typename Key>
bool GraphicsResourceManager::CheckResource(const Key& _key, const Resource& _resource)
{
    return _resource.find(_key) != _resource.end();
}
