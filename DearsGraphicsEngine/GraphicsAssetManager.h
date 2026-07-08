#pragma once

#include <string>

class GraphicsResourceManager;
class UiRenderer;

// 그래픽스 자산 로딩 요청을 한곳에 모으는 클래스이다.
//
// GraphicsResourceManager는 실제 GPU 리소스 저장소에 가깝고,
// GraphicsAssetManager는 "어떤 종류의 자산을 어떤 로딩 경로로 보낼지"를 결정하는 입구 역할을 한다.
//
// 지금은 대부분 GraphicsResourceManager로 단순 위임하지만,
// 나중에 RHI가 들어오면 이 클래스에서 DX11/DX12/Vulkan별 텍스처/버퍼 생성 경로를
// 감추거나, 비동기 로딩/중복 로딩 정책을 추가할 수 있다.
class GraphicsAssetManager
{
public:
	GraphicsAssetManager(GraphicsResourceManager* resourceManager, UiRenderer* uiRenderer);

	void LoadModel(std::string basePath, std::string fileName);
	void LoadAnimation(std::string basePath, std::string fileName);
	void Load3DTexture(std::string basePath, std::string fileName);
	void Load2DTexture(std::string basePath, std::string fileName);
	void Load2DMipMapTexture(std::string basePath, std::string fileName);
	void LoadDDSTexture(std::string basePath, std::string fileName, bool isCubeMap = true);
	void LoadFont(std::string basePath, std::string fileName, float size, bool isKorean);

private:
	GraphicsResourceManager* m_pResourceManager = nullptr;
	UiRenderer* m_pUiRenderer = nullptr;
};
