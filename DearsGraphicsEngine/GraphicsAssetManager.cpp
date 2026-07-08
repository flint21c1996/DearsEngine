#include "GraphicsAssetManager.h"

#include "GraphicsResourceManager.h"
#include "UiRenderer.h"

GraphicsAssetManager::GraphicsAssetManager(GraphicsResourceManager* resourceManager, UiRenderer* uiRenderer)
	: m_pResourceManager(resourceManager)
	, m_pUiRenderer(uiRenderer)
{
}

void GraphicsAssetManager::LoadModel(std::string basePath, std::string fileName)
{
	m_pResourceManager->AddModel(basePath, fileName);
}

void GraphicsAssetManager::LoadAnimation(std::string basePath, std::string fileName)
{
	m_pResourceManager->AddAnimation(basePath, fileName);
}

void GraphicsAssetManager::Load3DTexture(std::string basePath, std::string fileName)
{
	m_pResourceManager->Add3DTexture(basePath, fileName);
}

void GraphicsAssetManager::Load2DTexture(std::string basePath, std::string fileName)
{
	m_pResourceManager->Add2DTexture(basePath, fileName);
}

void GraphicsAssetManager::Load2DMipMapTexture(std::string basePath, std::string fileName)
{
	m_pResourceManager->Add2DMipMapTexture(basePath, fileName);
}

void GraphicsAssetManager::LoadDDSTexture(std::string basePath, std::string fileName, bool isCubeMap)
{
	m_pResourceManager->AddDDSTexture(basePath, fileName, isCubeMap);
}

void GraphicsAssetManager::LoadFont(std::string basePath, std::string fileName, float size, bool isKorean)
{
	// 폰트는 ImGui 폰트 아틀라스에 등록되어야 하므로 UiRenderer를 통해 로딩한다.
	// 로딩된 ImFont 포인터는 내부적으로 GraphicsResourceManager에도 등록된다.
	m_pUiRenderer->AddFont(basePath, fileName, size, isKorean);
}
