#include "GraphicsCommon.h"


namespace Dears {

	namespace Graphics {

		// Sampler States
		ComPtr<ID3D11SamplerState> linearWrapSS;
		ComPtr<ID3D11SamplerState> linearClampSS;
		ComPtr<ID3D11SamplerState> shadowPointSS;
		ComPtr<ID3D11SamplerState> shadowCompareSS;
		vector<ID3D11SamplerState*> sampleStates;

		// Rasterizer States
		ComPtr<ID3D11RasterizerState> solidRS;
		ComPtr<ID3D11RasterizerState> wireRS;

		// Depth Stencil States
		ComPtr<ID3D11DepthStencilState> drawDSS;       // �Ϲ������� �׸���
		ComPtr<ID3D11DepthStencilState> particleDSS;   // �Ϲ������� �׸��� + �޸鵵 ����


		ComPtr<ID3D11DepthStencilState> punchDSS;	   //��ü�� ����ũ�� ���Ľ��� ����
		ComPtr<ID3D11DepthStencilState> drawEdgeDSS;   //tempDSS���� ����� stencil�� �������� �ܰ����� �׸�   

		// Shaders
		ComPtr<ID3D11VertexShader> basicVS;
		ComPtr<ID3D11VertexShader> animeVS;
		ComPtr<ID3D11VertexShader> equimentVS;
		ComPtr<ID3D11VertexShader> skyboxVS;
		ComPtr<ID3D11VertexShader> depthOnlyVS;
		ComPtr<ID3D11VertexShader> depthAniOnlyVS;
		ComPtr<ID3D11VertexShader> depthEquipOnlyVS;
		ComPtr<ID3D11VertexShader> instanceBasicVS;
		ComPtr<ID3D11VertexShader> edgeVS;
		ComPtr<ID3D11VertexShader> testVS;
		ComPtr<ID3D11VertexShader> waterVS;
		ComPtr<ID3D11VertexShader> particleVS;
		ComPtr<ID3D11VertexShader> samplerVS;
		ComPtr<ID3D11VertexShader> postProcessingVS;


		ComPtr<ID3D11PixelShader> basicPS;
		ComPtr<ID3D11PixelShader> skyboxPS;
		ComPtr<ID3D11PixelShader> depthOnlyPS;
		ComPtr<ID3D11PixelShader> edgePS;
		ComPtr<ID3D11PixelShader> testPS;
		ComPtr<ID3D11PixelShader> particlePS;
		ComPtr<ID3D11PixelShader> samplerPS;
		ComPtr<ID3D11PixelShader> postProcessingPS;

		//ComputeShader
		ComPtr<ID3D11ComputeShader> particleComputeShader;


		// Input Layouts
		ComPtr<ID3D11InputLayout> basicIL;
		ComPtr<ID3D11InputLayout> animeIL;
		ComPtr<ID3D11InputLayout> equimentIL;
		ComPtr<ID3D11InputLayout> skyboxIL;
		ComPtr<ID3D11InputLayout> depthOnlyIL;
		ComPtr<ID3D11InputLayout> depthAniOnlyIL;
		ComPtr<ID3D11InputLayout> depthEquipOnlyIL;

		ComPtr<ID3D11InputLayout> instanceBasicIL;
		ComPtr<ID3D11InputLayout> edgeIL;
		ComPtr<ID3D11InputLayout> testIL;
		ComPtr<ID3D11InputLayout> waterIL;
		
		//Instancing Layouts
		ComPtr<ID3D11InputLayout> basicInstanceIL;
		ComPtr<ID3D11InputLayout> particleIL;

		//postprocessing Layouts
		ComPtr<ID3D11InputLayout> samplerIL;
		ComPtr<ID3D11InputLayout> postProcessingIL;

		// Graphics Pipeline States
		PipelineStateObject BasicGeometryPSO;
		PipelineStateObject AnimeGeometryPSO;
		PipelineStateObject EquipmentGeometryPSO;
		PipelineStateObject DebugGeometryPSO;
		PipelineStateObject CubeMapGeometryPSO;
		PipelineStateObject depthOnlyPSO;
		PipelineStateObject depthAniOnlyPSO;
		PipelineStateObject depthEquipOnlyPSO;
		PipelineStateObject OpacityPSO;

		PipelineStateObject BasicInstancingPSO;
		PipelineStateObject PunchingPSO;
		PipelineStateObject EdgePSO;
		PipelineStateObject WaterPSO;

		PipelineStateObject TestPSO;
		PipelineStateObject ParticlePSO;

		PipelineStateObject samplerPSO;
		PipelineStateObject postEffectPSO;

		// Blend States
		ComPtr<ID3D11BlendState> OpacityBS;



	} // namespace Graphics

	void Graphics::InitCommonStates(ComPtr<ID3D11Device>& device) {

		InitShaders(device);
		InitSamplers(device);
		InitRasterizerStates(device);
		InitBlendStates(device);
		InitDepthStencilStates(device);
		InitPipelineStates(device);
	}

	void Graphics::InitSamplers(ComPtr<ID3D11Device>& device) {

		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		device->CreateSamplerState(&sampDesc, linearWrapSS.GetAddressOf());
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		device->CreateSamplerState(&sampDesc, linearClampSS.GetAddressOf());

		// shadowPointSS
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.BorderColor[0] = 100.0f; // ū Z��
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		device->CreateSamplerState(&sampDesc, shadowPointSS.GetAddressOf());

		// shadowCompareSS, ���̴� �ȿ����� SamplerComparisonState
		// Filter = "_COMPARISON_" ����
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.BorderColor[0] = 100.0f; // ū Z��
		sampDesc.Filter =
			D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		device->CreateSamplerState(&sampDesc, shadowCompareSS.GetAddressOf());

		// ���÷� ������ "Common.hlsli"������ �ϰ��� �־�� ��
		sampleStates.push_back(linearWrapSS.Get());
		sampleStates.push_back(linearClampSS.Get());
		sampleStates.push_back(shadowPointSS.Get());
		sampleStates.push_back(shadowCompareSS.Get());
	}

	void Graphics::InitRasterizerStates(ComPtr<ID3D11Device>& device)
	{
		solidRS = RendererHelper::CreateSolidRasterizerState(device);
		wireRS = RendererHelper::CreateWireRasterizerState(device);
	}

	void Graphics::InitBlendStates(ComPtr<ID3D11Device>& device)
	{
		///���� ���¸� �����ϴ� ����ü, �� ����ü�� ����Ͽ� ���� ����� �����ϰ� �̸� GPU�� �����Ѵ�.
		D3D11_BLEND_DESC BlendDesc;
		ZeroMemory(&BlendDesc, sizeof(BlendDesc));	//�����׸� 0���� �ʱ�ȭ�Ѵ�.
		BlendDesc.AlphaToCoverageEnable = true; // MSAA(Multi-Sample Anti Aliasing)�� ����Ҷ� ���ĺ����� ����� ���ΰ�?
		BlendDesc.IndependentBlendEnable = false; //�������� ���� Ÿ��(�ִ� 8��)�� ���� �������� ���� ������ ����� ���ΰ�? false�� ��� ù��° ����Ÿ���� ���� ������ ��� ����Ÿ�ٿ� ����ȴ�.

		//���� Ÿ�� ����
		BlendDesc.RenderTarget[0].BlendEnable = true;								//������ Ȱ��ȭ �ϴ� ����, true�� �����ϸ� GPU�� ������ ȥ���Ͽ� ���

		///FinalRGB = (SrcRGB * SrcBlend) [op] (DstRGB * DestBlend)						//Scr = Source(�ҽ� - ���� �������Ϸ��� �ȼ��� ����), Dst = Destination(��� - �̹� ���� Ÿ�ٿ� �����ϴ� �ȼ��� ����), [op] - RenderTarget.BlendOp
		BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_BLEND_FACTOR;			//���� �ÿ� ���� �ҽ� ������ ȥ�չ���� ����, D3D11_BLEND_BLEND_FACTOR�� GPU�� ���� ���͸� �ҽ� ������ �������ڷ� ����Ѵ�. ���� ���ʹ� ���Ŀ� "OMSetBlendState"�� ȣ���� �� ������ �� �ִ�.
		BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_BLEND_FACTOR;		//���� �ÿ� ���� ��� ����(����Ÿ�ٿ� �̹� �����ϴ� ����)�� ȥ�չ���� ����, D3D11_BLEND_INV_BLEND_FACTOR�� ���� ������ "������(1 - ���� ����)�� ���ϴ� ������� �����Ѵ�.
		BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;					//�ҽ� ����� ��� ���� ���� ���� ������ �����Ѵ�.D3D11_BLEND_OP_ADD�� �� ������ ���ϴ� ������ �����Ѵ�. ���� ���� = (�ҽ� ���� * SrcBlend) + (��� ���� * DestBlend)

		///FinalAlpha = (SrcAlpha * SrcBlendAlpha) [op] (DstAlpha* DestBlendAlpha)		//Scr = Source(�ҽ� - ���� �������Ϸ��� �ȼ��� ���İ�), Dst = Destination(��� - �̹� ���� Ÿ�ٿ� �����ϴ� �ȼ��� ���İ�), [op] - RenderTarget.BlendOpAlpha
		BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;				// ���� ���� ���� �ҽ� ������ ȥ�� ����� ����, D3D11_BLEND_ONE - �ҽ� ���İ��� �״�� ����
		BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;				// ���� ���� ���� �ҽ� ������ ȥ�� ����� ����, D3D11_BLEND_ONE - ��� ���� ���� �״�� ���
		BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;			//�����ɷ�

		// �ʿ��ϸ� RGBA ������ ���ؼ��� ���� ����
		BlendDesc.RenderTarget[0].RenderTargetWriteMask =							//RGBA ä���� � ä�ο� ���� ������ ����
			D3D11_COLOR_WRITE_ENABLE_ALL;												//RGBA ��ο� ���� ������ �����ϰڴ�.

		ThrowIfFailed(
			device->CreateBlendState(&BlendDesc, OpacityBS.GetAddressOf()));
	}

	/// <summary>
	/// ���� ���Ľ� �׽�Ʈ�� �۵������ �����ϴ� ����
	/// </summary>
	/// <param name="device"></param>
	void Graphics::InitDepthStencilStates(ComPtr<ID3D11Device>& device)
	{
		// m_drawDSS: �⺻ DSS------------------------------------------------
		D3D11_DEPTH_STENCIL_DESC dsDesc;
		ZeroMemory(&dsDesc, sizeof(dsDesc));
		dsDesc.DepthEnable = true;														 // ���� �׽�Ʈ�� ������ ���ΰ�
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;								 //	���� ���ۿ� ����� �� ���ΰ�? -> ��� �� ���
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS;										 // ���� ���� ���� �Լ� -> �ȼ��� ���̰� ���̹��ۺ��� �������� ����
		dsDesc.StencilEnable = true;// Stencil ���ʿ�									 //	���Ľ� �׽�Ʈ�� �Ұ��ΰ�? false -> �����ʰڴ�. 
		dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;						 // ���Ľ� ���� �а� ���� ����� ����ũ
		dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;						 // ���Ľ� ���� �а� ���� ����� ����ũ

		// �ո鿡 ���ؼ� ��� �۵����� ����												 // ���Ľ��� �ո�� �޸鿡 ���� ���Ľ� �׽�Ʈ�� ������ ������ �� �ִ�.
		dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;							 // ���ٽ� �׽�Ʈ�� ������ ��� ���ٽ� ���� �����ϰڴ�.
		dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;					 // ���ٽ� �׽�Ʈ�� ��������� ���� �׽�Ʈ�� �������� ���
		dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;							 // ���ٽ� �׽�Ʈ�� ���� �׽�Ʈ ��� ����������
		dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;							 // ���ٽ� �׽�Ʈ�� �׻� �����ϴ� ���� ��, ���ٽ� ���� ������� �׻� ���
		// �޸鿡 ���� ��� �۵����� ���� (�޸鵵 �׸� ���)								
		dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;							 //
		dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;						 //
		dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;						 //
		dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;							 //

		ThrowIfFailed(
			device->CreateDepthStencilState(&dsDesc, drawDSS.GetAddressOf()));

		dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		ThrowIfFailed(
			device->CreateDepthStencilState(&dsDesc, particleDSS.GetAddressOf()));


		//punchDSS : �׸����� ���� ���Ľ��� �����ϴ� DSS-------------------------
		D3D11_DEPTH_STENCIL_DESC punchDsDesc;
		ZeroMemory(&punchDsDesc, sizeof(punchDsDesc));
		punchDsDesc.DepthEnable = true;
		punchDsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		punchDsDesc.DepthFunc = D3D11_COMPARISON_LESS;
		punchDsDesc.StencilEnable = true;
		punchDsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		punchDsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

		// �ո鿡 ���ؼ� ��� �۵����� ����									
		punchDsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		punchDsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		punchDsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		punchDsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		
		// �޸鿡 ���� ��� �۵����� ���� (�޸鵵 �׸� ���)					
		punchDsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		punchDsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		punchDsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		punchDsDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;

		ThrowIfFailed(
			device->CreateDepthStencilState(&punchDsDesc, punchDSS.GetAddressOf()));

		//drawEdgeDSS : �ܰ����� �����ϴ� DSS-----------------------------------
		D3D11_DEPTH_STENCIL_DESC edgeDsDesc;
		ZeroMemory(&edgeDsDesc, sizeof(edgeDsDesc));
		edgeDsDesc.DepthEnable = true;														 // ���� �׽�Ʈ�� ������ ���ΰ�
		edgeDsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;								 //	���� ���ۿ� ����� �� ���ΰ�? -> ��� �� ���
		edgeDsDesc.DepthFunc = D3D11_COMPARISON_LESS;										 // ���� ���� ���� �Լ� -> �ȼ��� ���̰� ���̹��ۺ��� �������� ����
		edgeDsDesc.StencilEnable = true;													 //	���Ľ� �׽�Ʈ�� �Ұ��ΰ�? 
		edgeDsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;						 // ���Ľ� ���� �а� ���� ����� ����ũ
		edgeDsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;						 // ���Ľ� ���� �а� ���� ����� ����ũ

		// �ո鿡 ���ؼ� ��� �۵����� ����												
		edgeDsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;							
		edgeDsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;					
		edgeDsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		edgeDsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
		// �޸鿡 ���� ��� �۵����� ���� (�޸鵵 �׸� ���)								
		edgeDsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;						
		edgeDsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;					
		edgeDsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		edgeDsDesc.BackFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;							 //�޸��� �ȱ׸���.

		ThrowIfFailed(
			device->CreateDepthStencilState(&edgeDsDesc, drawEdgeDSS.GetAddressOf()));
	}

	///D3D11_INPUT_PER_VERTEX_DATA�� ��� �ش� �����Ͱ� ���������� ó������ ��Ÿ����. ��, �� �������� �ٸ� �����͸� �����ϴ� ��� ����Ѵ�.
	///D3D11_INPUT_PER_INSTANCE_DATA�� ���  �ش� �����Ͱ� �ν���Ʈ ������ ó������ ��Ÿ����.
	/// �̴� �ѹ��� ��ο��ݷ� ������ ������Ʈ�� ���� �� �������� �� �� �ν���Ʈ���� �����Ǵ� �����͸� �ǹ��Ѵ�.
	/// ���� ��� ������ ���� ���� ��ġ�� �׸� ��, �� �ν��Ͻ����� �ٸ� ��ȯ ����� �����Ҽ� �ִ�. �̷� ��ȯ ������ �ν��Ͻ� ������ �����ȴ�.
	//	typedef
	//		enum D3D11_INPUT_CLASSIFICATION
	//	{
	//		D3D11_INPUT_PER_VERTEX_DATA = 0,
	//		D3D11_INPUT_PER_INSTANCE_DATA = 1
	//	} 	D3D11_INPUT_CLASSIFICATION;

	void Graphics::InitShaders(ComPtr<ID3D11Device>& device)
	{
		// Shaders, InputLayouts
		vector<D3D11_INPUT_ELEMENT_DESC> basicInputElements = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		std::vector<D3D11_INPUT_ELEMENT_DESC> AnimeInputElements =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},				   //��ġ
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},				   //�븻
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},				   //�ؽ��� ��ǥ
			{"BONEINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},		   //�� �ε���
			{"BONEINDICES", 1, DXGI_FORMAT_R32G32B32A32_UINT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},		   //�� �ε���2
			{"BONEWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 64, D3D11_INPUT_PER_VERTEX_DATA, 0},		   //���� ����ġ
			{"BONEWEIGHTS", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 80, D3D11_INPUT_PER_VERTEX_DATA, 0}		   //���� ����ġ2
		};
		//DirectX::BoundingBox& objectBounds

		vector<D3D11_INPUT_ELEMENT_DESC> skyboxInputElements = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
			 D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
			 D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
			 D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		vector<D3D11_INPUT_ELEMENT_DESC> EquipmentInputElements = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
			 D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
			 D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
			 D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		vector<D3D11_INPUT_ELEMENT_DESC> BasicInstanceElements = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "SV_InstanceID", 0, DXGI_FORMAT_R32_UINT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
		};
		
		vector< D3D11_INPUT_ELEMENT_DESC> dummyElement =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		std::string VSfilename = "../DearsGraphicsEngine/Shader/VertexShader.hlsl";
		RendererHelper::CreateVertexShaderAndInputLayout(device, VSfilename,
			basicInputElements, basicVS, basicIL);

		VSfilename = "../DearsGraphicsEngine/Shader/AniVertexShader.hlsl";
		RendererHelper::CreateVertexShaderAndInputLayout(device, VSfilename,
			AnimeInputElements, animeVS, animeIL);

		VSfilename = "../DearsGraphicsEngine/Shader/CubeMapVertexShader.hlsl";
		RendererHelper::CreateVertexShaderAndInputLayout(device, VSfilename,
			skyboxInputElements, skyboxVS, skyboxIL);

		VSfilename = "../DearsGraphicsEngine/Shader/ShadowVertexShader.hlsl";
		RendererHelper::CreateVertexShaderAndInputLayout(device, VSfilename,
			basicInputElements, depthOnlyVS, depthOnlyIL);

		VSfilename = "../DearsGraphicsEngine/Shader/ShadowAniVertexShader.hlsl";
		RendererHelper::CreateVertexShaderAndInputLayout(device, VSfilename,
			AnimeInputElements, depthAniOnlyVS, depthAniOnlyIL);

		VSfilename = "../DearsGraphicsEngine/Shader/ShadowEquipmentVertexShader.hlsl";
		RendererHelper::CreateVertexShaderAndInputLayout(device, VSfilename,
			basicInputElements, depthEquipOnlyVS, depthEquipOnlyIL);

		VSfilename = "../DearsGraphicsEngine/Shader/EquipmentVertexShader.hlsl";
		RendererHelper::CreateVertexShaderAndInputLayout(device, VSfilename,
			EquipmentInputElements, equimentVS, equimentIL);

		VSfilename = "../DearsGraphicsEngine/Shader/InstanceVertexShader.hlsl";
		RendererHelper::CreateVertexShaderAndInputLayout(device, VSfilename,
			BasicInstanceElements, instanceBasicVS, instanceBasicIL);

		VSfilename = "../DearsGraphicsEngine/Shader/EdgeVertexShader.hlsl";
		RendererHelper::CreateVertexShaderAndInputLayout(device, VSfilename,
			BasicInstanceElements, edgeVS, edgeIL);

		VSfilename = "../DearsGraphicsEngine/Shader/TestVertexShader.hlsl";
		RendererHelper::CreateVertexShaderAndInputLayout(device, VSfilename,
			basicInputElements, testVS, testIL);


		VSfilename = "../DearsGraphicsEngine/Shader/WaterVertexShader.hlsl";
		RendererHelper::CreateVertexShaderAndInputLayout(device, VSfilename,
			basicInputElements, waterVS, waterIL);

		VSfilename = "../DearsGraphicsEngine/Shader/ParticleVertexShader.hlsl";
		RendererHelper::CreateVertexShaderAndInputLayout(device, VSfilename,
			dummyElement, particleVS, particleIL);

		VSfilename = "../DearsGraphicsEngine/Shader/SamplerVertexShader.hlsl";
		RendererHelper::CreateVertexShaderAndInputLayout(device, VSfilename,
			basicInputElements, samplerVS, samplerIL);

		VSfilename = "../DearsGraphicsEngine/Shader/PostProcessingVertexShader.hlsl";
		RendererHelper::CreateVertexShaderAndInputLayout(device, VSfilename,
			basicInputElements, postProcessingVS, postProcessingIL);

		//Create PixelSahder
		std::string PSfilename = "../DearsGraphicsEngine/Shader/PixelShader.hlsl";
		RendererHelper::CreatePixelShader(device, PSfilename, basicPS);

		PSfilename = "../DearsGraphicsEngine/Shader/CubeMapPixelShader.hlsl";
		RendererHelper::CreatePixelShader(device, PSfilename, skyboxPS);

		PSfilename = "../DearsGraphicsEngine/Shader/ShadowPixelShader.hlsl";
		RendererHelper::CreatePixelShader(device, PSfilename, depthOnlyPS);

		PSfilename = "../DearsGraphicsEngine/Shader/EdgePixelShader.hlsl";
		RendererHelper::CreatePixelShader(device, PSfilename, edgePS);

		PSfilename = "../DearsGraphicsEngine/Shader/TestPixelShader.hlsl";
		RendererHelper::CreatePixelShader(device, PSfilename, testPS);

		PSfilename = "../DearsGraphicsEngine/Shader/ParticlePixelShader.hlsl";
		RendererHelper::CreatePixelShader(device, PSfilename, particlePS);

		PSfilename = "../DearsGraphicsEngine/Shader/SamplerPixelShader.hlsl";
		RendererHelper::CreatePixelShader(device, PSfilename, samplerPS);

		PSfilename = "../DearsGraphicsEngine/Shader/PostProcessingPixelShader.hlsl";
		RendererHelper::CreatePixelShader(device, PSfilename, postProcessingPS);

		//Create ComputeShader
		std::string CSfilename = "../DearsGraphicsEngine/Shader/TestComputeShader.hlsl";
		RendererHelper::CreateComputeShader(device, CSfilename, particleComputeShader);




		//RendererHelper::CreateGeometryShader(device, L"NormalGS.hlsl", normalGS);
	}

	void Graphics::InitPipelineStates(ComPtr<ID3D11Device>& device) {

		// BasicGeometryPSO;
		BasicGeometryPSO.m_pVertexShader = basicVS;
		BasicGeometryPSO.m_pInputLayout = basicIL;
		BasicGeometryPSO.m_pPixelShader = basicPS;
		BasicGeometryPSO.m_pRasterizerState = solidRS;
		BasicGeometryPSO.m_pDepthStencilState = drawDSS;
		BasicGeometryPSO.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		BasicGeometryPSO.m_pBlendState = OpacityBS;


		// DebugGeometryPSO
		DebugGeometryPSO = BasicGeometryPSO;
		DebugGeometryPSO.m_pRasterizerState = wireRS;

		// AnimeGeometryPSO
		AnimeGeometryPSO.m_pVertexShader = animeVS;
		AnimeGeometryPSO.m_pInputLayout = animeIL;
		AnimeGeometryPSO.m_pPixelShader = basicPS;
		AnimeGeometryPSO.m_pRasterizerState = solidRS;
		AnimeGeometryPSO.m_pDepthStencilState = drawDSS;
		AnimeGeometryPSO.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		// AnimeGeometryPSO
		EquipmentGeometryPSO.m_pVertexShader = equimentVS;
		EquipmentGeometryPSO.m_pInputLayout = equimentIL;
		EquipmentGeometryPSO.m_pPixelShader = basicPS;
		EquipmentGeometryPSO.m_pRasterizerState = solidRS;
		EquipmentGeometryPSO.m_pDepthStencilState = drawDSS;
		EquipmentGeometryPSO.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		CubeMapGeometryPSO.m_pVertexShader = skyboxVS;
		CubeMapGeometryPSO.m_pInputLayout = skyboxIL;
		CubeMapGeometryPSO.m_pPixelShader = skyboxPS;
		CubeMapGeometryPSO.m_pRasterizerState = solidRS;
		CubeMapGeometryPSO.m_pDepthStencilState = drawDSS;
		CubeMapGeometryPSO.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		// depthOnlyPSO
		depthOnlyPSO = BasicGeometryPSO;
		depthOnlyPSO.m_pVertexShader = depthOnlyVS;
		depthOnlyPSO.m_pPixelShader = depthOnlyPS;

		// depthAniOnlyPSO
		depthAniOnlyPSO = AnimeGeometryPSO;
		depthAniOnlyPSO.m_pVertexShader = depthAniOnlyVS;
		depthAniOnlyPSO.m_pPixelShader = depthOnlyPS;

		// depthEquipOnlyPSO
		depthEquipOnlyPSO = BasicGeometryPSO;
		depthEquipOnlyPSO.m_pVertexShader = depthEquipOnlyVS;
		depthEquipOnlyPSO.m_pPixelShader = depthOnlyPS;

		//OpacityPso
		OpacityPSO = BasicGeometryPSO;
		OpacityPSO.m_pBlendState = OpacityBS;

		//InstancingPSO
		BasicInstancingPSO = BasicGeometryPSO;
		BasicInstancingPSO.m_pVertexShader = instanceBasicVS;
		BasicInstancingPSO.m_pInputLayout = instanceBasicIL;
		
		//PunchingPSO
		PunchingPSO = BasicGeometryPSO;
		PunchingPSO.m_pVertexShader = depthOnlyVS;
		PunchingPSO.m_pPixelShader = depthOnlyPS;
		PunchingPSO.m_pDepthStencilState = punchDSS;
		PunchingPSO.m_stencilRef = 1;

		//EdgePSO
		EdgePSO = BasicGeometryPSO;
		EdgePSO.m_pVertexShader = edgeVS;
		EdgePSO.m_pPixelShader = edgePS;
		EdgePSO.m_pDepthStencilState = drawEdgeDSS;
		EdgePSO.m_stencilRef = 1;

		//WaterPSO
		WaterPSO = BasicGeometryPSO;
		WaterPSO.m_pVertexShader = waterVS;
		WaterPSO.m_pInputLayout = waterIL;

		//TestPSO
		TestPSO = BasicGeometryPSO;
		TestPSO.m_pVertexShader = testVS;
		TestPSO.m_pPixelShader = testPS;

		ParticlePSO = BasicInstancingPSO;
		ParticlePSO.m_pVertexShader = particleVS;
		ParticlePSO.m_pInputLayout = particleIL;
		ParticlePSO.m_pPixelShader = particlePS;
		//ParticlePSO.m_pRasterizerState = wireRS;
		ParticlePSO.m_pDepthStencilState = particleDSS;
		
		samplerPSO = BasicGeometryPSO;
		samplerPSO.m_pVertexShader = samplerVS;
		samplerPSO.m_pInputLayout = samplerIL;
		samplerPSO.m_pPixelShader = samplerPS;

		postEffectPSO = BasicGeometryPSO;
		postEffectPSO.m_pVertexShader = postProcessingVS;
		postEffectPSO.m_pInputLayout = postProcessingIL;
		postEffectPSO.m_pPixelShader = postProcessingPS;
	}

}