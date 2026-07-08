#pragma once

#include <d3d11.h>
#include <memory>
#include <wrl.h>

#include "BufferData.h"

class GraphicsResourceManager;
class ParticleManager;

using Microsoft::WRL::ComPtr;

// 파티클 시스템의 초기화, 업데이트, 렌더링 요청을 묶어 관리하는 클래스이다.
//
// ParticleManager는 실제 DX11 버퍼, UAV/SRV, 컴퓨트 셰이더 실행 같은 낮은 단계의 일을 처리한다.
// ParticleRenderer는 그 ParticleManager를 그래픽스 엔진의 렌더 흐름에 붙이는 역할을 맡는다.
//
// 이렇게 한 단계 감싸두면 DearsGraphicsEngine은 "파티클 패스를 실행한다" 정도만 알면 되고,
// 파티클이 어떤 메시 버퍼를 쓰는지, 어떤 순서로 초기화되는지는 이 클래스 안으로 숨길 수 있다.
class ParticleRenderer
{
public:
	ParticleRenderer(
		ComPtr<ID3D11Device> device,
		ComPtr<ID3D11DeviceContext> deviceContext,
		GraphicsResourceManager* resourceManager,
		unsigned int maxParticleCount);
	~ParticleRenderer();

	void Initialize();
	void Update();
	void Render();
	void AddParticle(unsigned int particleNum, CSParticleData& particleData);

private:
	ComPtr<ID3D11Device> m_pDevice;
	ComPtr<ID3D11DeviceContext> m_pDeviceContext;
	GraphicsResourceManager* m_pResourceManager = nullptr;
	unsigned int m_maxParticleCount = 0;

	std::unique_ptr<ParticleManager> m_pParticleManager;
};
