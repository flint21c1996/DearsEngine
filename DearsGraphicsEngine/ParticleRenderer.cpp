#include "ParticleRenderer.h"

#include "GraphicsResourceManager.h"
#include "ParticleManager.h"

ParticleRenderer::ParticleRenderer(
	ComPtr<ID3D11Device> device,
	ComPtr<ID3D11DeviceContext> deviceContext,
	GraphicsResourceManager* resourceManager,
	unsigned int maxParticleCount)
	: m_pDevice(device)
	, m_pDeviceContext(deviceContext)
	, m_pResourceManager(resourceManager)
	, m_maxParticleCount(maxParticleCount)
{
}

ParticleRenderer::~ParticleRenderer() = default;

void ParticleRenderer::Initialize()
{
	m_pParticleManager = std::make_unique<ParticleManager>(m_pDevice, m_pDeviceContext, m_maxParticleCount);
	m_pParticleManager->Initialize();

	// 현재 파티클은 빌보드 사각형 하나를 인스턴싱해서 그린다.
	// 그래서 렌더링에 필요한 vertex/index buffer는 BillBoardSquare 모델에서 가져온다.
	m_pParticleManager->SetVertexBufferAndIndexBuffer(
		m_pResourceManager->Get_VertexBuffer("BillBoardSquare"),
		m_pResourceManager->Get_IndexBuffer("BillBoardSquare"),
		m_pResourceManager->Get_NumIndex("BillBoardSquare"));
}

void ParticleRenderer::Update()
{
	m_pParticleManager->Update();
}

void ParticleRenderer::Render()
{
	m_pParticleManager->Render();
}

void ParticleRenderer::AddParticle(unsigned int particleNum, CSParticleData& particleData)
{
	m_pParticleManager->AddParticle(particleNum, particleData);
}
