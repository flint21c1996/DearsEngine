#pragma once
#include<wrl.h>
#include<d3d11.h>
#include "ModelInfo.h"
#include "BufferData.h"
using Microsoft::WRL::ComPtr;

// enum class AnimationPlayType
// {
// 	Loop,
// 	Once,
// };

struct ModelBuffer // 1개의 모델에 대해 여러 개의 객체를 가질 경우를 대비해 만드는 버퍼
{
	ComPtr<ID3D11Buffer> m_pVertexBuffer;					// 버텍스 버퍼
	ComPtr<ID3D11Buffer> m_pIndexBuffer;					// 인덱스 버퍼

	ComPtr<ID3D11Buffer> m_pVSConstantBuffer;				// 버텍스 컨스턴트 버퍼
	ComPtr<ID3D11Buffer> m_pPSConstantBuffer;				// 픽셸 컨스턴트 버퍼
	ComPtr<ID3D11Buffer> m_BoneConstantBuffer;				// 본 컨스턴트 버퍼	   
	ComPtr<ID3D11Buffer> m_TargetBoneConstantBuffer;		// 타겟 본 컨스턴트 버퍼	   

	ComPtr<ID3D11Buffer> m_InstanceConstantBuffer;			//인스턴싱을 할때 필요한 인스턴스 컨스턴트 버퍼	   
	unsigned int mNumInstances = 0;								//인스턴스 개수

	ComPtr<ID3D11Buffer> m_pVSEdgeConstantBuffer;			// 외곽선 컨스턴트 버퍼
	ComPtr<ID3D11Buffer> m_pPSEdgeConstantBuffer;			// 외곽선 컨스턴트 버퍼

	ComPtr<ID3D11Buffer> m_pVSWaterConstantBuffer;			//물 컨스턴트 버퍼 -> 후에 물 쉐이딩으로 발전시킨다.

	ComPtr<ID3D11ShaderResourceView> m_diffusetexture;		// 텍스처와 쉐이더 리소스 뷰 구조체
	ComPtr<ID3D11ShaderResourceView> m_normaltexture;
	ComPtr<ID3D11ShaderResourceView> m_cubeMapDiffuseResourceView;
	ComPtr<ID3D11ShaderResourceView> m_cubeMapSpecularResourceView;
	
	UINT offset = 0;										// 이거는 Mesh에 있는 게 더 어울리는 거 같은데..

	unsigned int mNumIndices = 0;
												
	Model* mpTargetModel;

	Animation* mpTargetAnimation = nullptr;						   //어떤 애니메이션을 재생시킬 것인지
	Animation* mpNextTargetAnimation = nullptr;					   //어떤 애니메이션을 다음으로 재생시킬 것인지

	double mAnimationPlaytime = 0;
	double mNextAnimationPlaytime = 0;

// 	AnimationPlayType mAnimationPlayType = AnimationPlayType::Loop;
// 	AnimationPlayType mNextAnimationPlayType = AnimationPlayType::Loop;

	void operator=(ModelBuffer& _modelBuffer)
	{
		m_pVertexBuffer = _modelBuffer.m_pVertexBuffer;
		m_pIndexBuffer = _modelBuffer.m_pIndexBuffer;
		m_pVSConstantBuffer = _modelBuffer.m_pVSConstantBuffer;
		m_pPSConstantBuffer = _modelBuffer.m_pPSConstantBuffer;
		m_BoneConstantBuffer = _modelBuffer.m_BoneConstantBuffer;
		m_diffusetexture = _modelBuffer.m_diffusetexture;
		m_normaltexture = _modelBuffer.m_normaltexture;
		offset = _modelBuffer.offset;
	}
};


