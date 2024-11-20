#include "ParticleManager.h"
#include "RendererHelper.h"
#include "GraphicsCommon.h"
#include "ModelInfo.h"
namespace particleEnum
{
	enum ParticleState
	{
		NotUse,
		Use,
	};
}

using namespace Dears;
ParticleManager::ParticleManager(ComPtr<ID3D11Device> _pDevice, ComPtr<ID3D11DeviceContext> _pDeviceContext, unsigned int _scale)
{
	m_pDevice = _pDevice;
	m_pDeviceContext = _pDeviceContext;
	m_pParticleObjectpool = new ObjectPool<CSParticleData>(1024);
	m_pParticleObjectpool->ChangeState(0, m_pParticleObjectpool->GetSize() - 1, static_cast<unsigned int>(particleEnum::NotUse));
}

void ParticleManager::Initialize()
{
	CreateParticleStructedBufferandUAVandSRV();
	CreateStagingBuffer();
}

void ParticleManager::CreateParticleStructedBufferandUAVandSRV()
{
	CSParticleData tempParticleData;
	m_pParticleStructedBuffer = RendererHelper::CreateStructuredBuffer(m_pDevice, tempParticleData, m_pParticleObjectpool->GetSize());
	m_pParticleUAV = RendererHelper::CreateUnorderedAccessView(m_pDevice, m_pParticleStructedBuffer, m_pParticleObjectpool->GetSize());
	m_pParticleSRV = RendererHelper::CreateShaderResourceView(m_pDevice, m_pParticleStructedBuffer, m_pParticleObjectpool->GetSize());

	unsigned int tempIndex;
	m_pParticleIndexBuffer = RendererHelper::CreateStructuredBuffer(m_pDevice, tempIndex, m_pParticleObjectpool->GetSize());
	m_pParticleIndexUAV = RendererHelper::CreateUnorderedAccessView(m_pDevice, m_pParticleIndexBuffer, m_pParticleObjectpool->GetSize());
	m_pParticleIndexSRV = RendererHelper::CreateShaderResourceView(m_pDevice, m_pParticleIndexBuffer, m_pParticleObjectpool->GetSize());
	
	ParticleVertexInput tempVertexContantdata;
	m_pParticleVertexinfoBuffer = RendererHelper::CreateStructuredBuffer(m_pDevice, tempVertexContantdata, m_pParticleObjectpool->GetSize());
	m_pParticleVertexInfoUAV = RendererHelper::CreateUnorderedAccessView(m_pDevice, m_pParticleVertexinfoBuffer, m_pParticleObjectpool->GetSize());
	m_pParticleVertexInfoSRV = RendererHelper::CreateShaderResourceView(m_pDevice, m_pParticleVertexinfoBuffer, m_pParticleObjectpool->GetSize());

}

void ParticleManager::CreateStagingBuffer()
{
	m_pIndexStagingBuffer = RendererHelper::CreateStagingBuffer(m_pDevice, m_pParticleObjectpool->GetSize(),
															sizeof(unsigned int), m_pParticleObjectpool->GetAvailableVec()->data());
	CSParticleData tempParticleData;
	m_pParticleInfoStagingBuffer = RendererHelper::CreateStagingBuffer(m_pDevice, m_pParticleObjectpool->GetSize(),
													sizeof(tempParticleData), m_pParticleObjectpool->GetObjectPool()->data());
}

void ParticleManager::Update()
{
	vector<CSParticleData>* temp = m_pParticleObjectpool->GetObjectPool();

	RendererHelper::UpdateDataOnGPU(m_pDeviceContext, m_pParticleStructedBuffer, m_pParticleInfoStagingBuffer,
		m_pParticleObjectpool->GetObjectVecSize(), m_pParticleObjectpool->GetObjectPool()->data());

	RendererHelper::UpdateDataOnGPU(m_pDeviceContext, m_pParticleIndexBuffer, m_pIndexStagingBuffer,
		m_pParticleObjectpool->GetAvailableVecMemorySize(), m_pParticleObjectpool->GetAvailableVec()->data());

	// UAV를 컴퓨트 셰이더에 바인딩
	m_pDeviceContext->CSSetUnorderedAccessViews(0, 1, m_pParticleUAV.GetAddressOf(), nullptr);			//Compute Shader에서 Particle 위치 변경
	m_pDeviceContext->CSSetUnorderedAccessViews(1, 1, m_pParticleIndexUAV.GetAddressOf(), nullptr);		//Compute Shader에서 Particle 위치 변경
	m_pDeviceContext->CSSetUnorderedAccessViews(2, 1, m_pParticleVertexInfoUAV.GetAddressOf(), nullptr);		//Compute Shader에서 Particle 위치 변경
	//m_pDeviceContext->CSSetShaderResources(0, 1, m_pParticleSRV.GetAddressOf());

	// Dispatch 호출하여 컴퓨트 셰이더 실행
	const UINT threadsPerGroup = 256;
	const UINT totalParticles = 1024;
	UINT dispatchCount = (totalParticles + threadsPerGroup - 1) / threadsPerGroup;
	m_pDeviceContext->CSSetShader(Dears::Graphics::particleComputeShader.Get(), nullptr, 0);		//[설정한 컴퓨트 셰이더, HLSL클래스 인스턴스를 사용할 경우 이를 전달하는 포인터 배열, 배열에 포함된 클래스 인스턴스의 개수]
	m_pDeviceContext->Dispatch(dispatchCount, 1, 1);

	// UAV 해제
	ComputeShaderBarrier();

	//인덱스버퍼의 내용을 스테이징 버퍼로 복사한다.
	m_pDeviceContext->CopyResource(m_pIndexStagingBuffer.Get(), m_pParticleIndexBuffer.Get());
	RendererHelper::CopyFromStagingBuffer(m_pDeviceContext, m_pIndexStagingBuffer, m_pParticleObjectpool->GetAvailableVecMemorySize(), m_pParticleObjectpool->GetAvailableVec()->data());

	m_pDeviceContext->CopyResource(m_pParticleInfoStagingBuffer.Get(), m_pParticleStructedBuffer.Get());
	RendererHelper::CopyFromStagingBuffer(m_pDeviceContext, m_pParticleInfoStagingBuffer, m_pParticleObjectpool->GetObjectVecSize(), m_pParticleObjectpool->GetObjectPool()->data());


}

void ParticleManager::Render()
{
	auto _pso = Dears::Graphics::ParticlePSO;
	m_pDeviceContext->VSSetShader(_pso.m_pVertexShader.Get(), 0, 0);
	m_pDeviceContext->PSSetShader(_pso.m_pPixelShader.Get(), 0, 0);
	m_pDeviceContext->IASetInputLayout(_pso.m_pInputLayout.Get());
	m_pDeviceContext->RSSetState(_pso.m_pRasterizerState.Get());
	m_pDeviceContext->OMSetBlendState(_pso.m_pBlendState.Get(), _pso.m_blendFactor, 0xffffffff);		 //0xffffffff - 모든 샘플을 활성화 시키겠다.
	m_pDeviceContext->OMSetDepthStencilState(_pso.m_pDepthStencilState.Get(), _pso.m_stencilRef);
	m_pDeviceContext->IASetPrimitiveTopology(_pso.m_primitiveTopology);


	UINT stride = sizeof(Vertex); // 정점 데이터의 크기
	UINT offset = 0;

	m_pDeviceContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset); // 정점 버퍼 설정
	m_pDeviceContext->IASetIndexBuffer(mIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0); // 인덱스 버퍼 설정
	UINT instanceCount = m_pParticleObjectpool->GetSize(); // 인스턴스 개수



	m_pDeviceContext->VSSetShaderResources(0, 1, m_pParticleVertexInfoSRV.GetAddressOf());
	m_pDeviceContext->VSSetShaderResources(1, 1, m_pParticleIndexSRV.GetAddressOf());
	m_pDeviceContext->DrawIndexedInstanced(mNumIndices, instanceCount, 0, 0, 0);


}

/// <summary>
/// 다음 번에 컴퓨트 셰이더나 그래픽 셰이더가 새로운 리소스를 할당할 때 이전 UAV나 SRV의 영향을 받지 않도록 하기 위한 방어적인 조치
/// </summary>
void ParticleManager::ComputeShaderBarrier() {

	// 참고: BreadcrumbsDirectX-Graphics-Samples (DX12)
	// void CommandContext::InsertUAVBarrier(GpuResource & Resource, bool FlushImmediate)

	// 최대 사용하는 SRV, UAV 갯수가 n개, 지금은 6으로 한다.
	ID3D11ShaderResourceView* nullSRV[6] = {
		0,
	};
	m_pDeviceContext->CSSetShaderResources(0, 6, nullSRV);

	ID3D11UnorderedAccessView* nullUAV[6] = {
		0,
	};
	m_pDeviceContext->CSSetUnorderedAccessViews(0, 6, nullUAV, NULL);
}

void ParticleManager::SetVertexBufferAndIndexBuffer(ComPtr<ID3D11Buffer> _vertexBuffer, ComPtr<ID3D11Buffer> _indexBuffer, unsigned int _numIndices)
{
	mVertexBuffer = _vertexBuffer;
	mIndexBuffer = _indexBuffer;
	mNumIndices = _numIndices;
}

void ParticleManager::AddParticle(unsigned int _particleNum, CSParticleData& _particleData)
{
	unsigned int particleNum = _particleNum;
	std::vector<unsigned int>* availalveVec = m_pParticleObjectpool->GetAvailableVec();
	std::vector<CSParticleData>* ObjectPoolVec = m_pParticleObjectpool->GetObjectPool();

	for (unsigned int i = 0; i < m_pParticleObjectpool->GetSize()&& particleNum > 0 ; i++)
	{
		if ((*availalveVec)[i] == particleEnum::NotUse)
		{
			(*ObjectPoolVec)[i] = _particleData;
			(*availalveVec)[i] = particleEnum::Use;
			particleNum--;
		}
	}
}
