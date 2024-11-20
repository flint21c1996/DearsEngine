#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include "LayerEnum.h"
#include "FBXLoader.h"
#include "GraphicsResourceContainer.h"
#include "GeometryGenerator.h"	//박스만들때 필요함.
#include "RendererHelper.h"

///리소스를 생성, 저장, 삭제를 도맡아하는 클래스
///FBXLoader, GraphicsResourceContainer와 긴밀하게 움직인다.
class GraphicsResourceManager
{
public:
	GraphicsResourceManager(ComPtr<ID3D11Device> _pDevice, ComPtr<ID3D11DeviceContext> _pDeviceContext);
	~GraphicsResourceManager();

private:
	ComPtr<ID3D11Device> mpDevice; 
	ComPtr<ID3D11DeviceContext> mpDeviceContext;

	FBXLoader* mpFBXLoader;
	GraphicsResourceContainer* mpGraphicsResourceContainer;
public:
	void Initialize();
	void Update();
	void Finalize();

public:
	void AddModel(std::string _basePath, std::string _fileName);
	void AddModel(MeshData _meshData, std::string _meshName);
	void AddAnimation(std::string _basePath, std::string _fileName);
	void Add3DTexture(std::string _basePath, std::string _fileName);
	void Add2DTexture(std::string _basePath, std::string _fileName);
	void AddDDSTexture(std::string _basePath, std::string _fileName);
	void Add_Font(std::string fontName, ImFont* _font);

	//컨테이너에서 필요한 리소스를 가지고 온다.
	ComPtr<ID3D11Buffer> Get_VertexBuffer(std::string _modelName);
	ComPtr<ID3D11Buffer> Get_IndexBuffer(std::string _modelName);
	unsigned int Get_NumIndex(std::string _modelName);
	Model* Get_ModelInfo(std::string _modelName);
	Animation* Get_Animation(std::string _modelName);
	ComPtr<ID3D11ShaderResourceView> Get_Textures(std::string _modelName);
	ImFont* Get_Font(std::string _modelName);
	Matrix Get_TargetBoneMatrix(std::string _modelName, std::string _boneName);
	Matrix Get_TargetBoneAboveMatrix(std::string _modelName, std::string _boneName);
	Matrix Get_TargetBoneAboveMatrix(std::string _modelName, int _index);
	int Get_TargetModelBoneIndex(std::string _modelName, std::string _boneName);

	//컨테이너에서 특정 리소스를 지운다.
	void Erase_VertexBuffer(std::string _modelName);
	void Erase_IndexBuffer(std::string _modelName);
	void Erase_NumIndex(std::string _modelName);
	void Erase_ModelInfo(std::string _modelName);
	void Erase_Animation(std::string _animName);
	void Erase_Textures(std::string _modelName);
	void Erase_Font(std::string _modelName);



private:



};