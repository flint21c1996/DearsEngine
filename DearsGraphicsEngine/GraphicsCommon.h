#pragma once
#include "RendererHelper.h"
#include "PipelineStateObject.h"
#include <vector>
using std::vector;

///이거 나중에 외부에서 막 가져다 쓸수도 있으니까 나중엔 막아둬야함 const를 붙이든, 함수로 접근하도록 구현하든.. 해서.. 

namespace Dears {

	// 참고: DirectX_Graphic-Samples 미니엔진
	// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/GraphicsCommon.h

	namespace Graphics {

		// Samplers
		extern ComPtr<ID3D11SamplerState> linearWrapSS;
		extern ComPtr<ID3D11SamplerState> linearClampSS;
		extern ComPtr<ID3D11SamplerState> shadowPointSS;
		extern ComPtr<ID3D11SamplerState> shadowCompareSS;
		extern vector<ID3D11SamplerState*> sampleStates;

		// Rasterizer States
		extern ComPtr<ID3D11RasterizerState> solidRS;
		extern ComPtr<ID3D11RasterizerState> wireRS;

		// Depth Stencil States
		extern ComPtr<ID3D11DepthStencilState> drawDSS; // 일반적으로 그리기

		// Shaders
		extern ComPtr<ID3D11VertexShader> basicVS;
		extern ComPtr<ID3D11VertexShader> animeVS;
		extern ComPtr<ID3D11VertexShader> equimentVS;
		extern ComPtr<ID3D11VertexShader> skyboxVS;
		extern ComPtr<ID3D11VertexShader> depthOnlyVS;
		extern ComPtr<ID3D11VertexShader> depthAniOnlyVS;
		extern ComPtr<ID3D11VertexShader> depthEquipOnlyVS;
		extern ComPtr<ID3D11VertexShader> instanceBasicVS;

		extern ComPtr<ID3D11PixelShader> basicPS;
		extern ComPtr<ID3D11PixelShader> skyboxPS;

		//ComputeShader
		extern ComPtr<ID3D11ComputeShader> particleComputeShader;


		// Input Layouts
		extern ComPtr<ID3D11InputLayout> basicIL;
		extern ComPtr<ID3D11InputLayout> animeIL;
		extern ComPtr<ID3D11InputLayout> skyboxIL;
		extern ComPtr<ID3D11InputLayout> depthOnlyIL;
		extern ComPtr<ID3D11InputLayout> depthAniOnlyIL;
		extern ComPtr<ID3D11InputLayout> depthEquipOnlyIL;

		extern ComPtr<ID3D11InputLayout> instanceBasicIL;


		// Graphics Pipeline States
		extern PipelineStateObject BasicGeometryPSO;
		extern PipelineStateObject AnimeGeometryPSO;
		extern PipelineStateObject EquipmentGeometryPSO;
		extern PipelineStateObject DebugGeometryPSO;
		extern PipelineStateObject CubeMapGeometryPSO;
		extern PipelineStateObject depthOnlyPSO;
		extern PipelineStateObject depthAniOnlyPSO;
		extern PipelineStateObject depthEquipOnlyPSO;
		extern PipelineStateObject OpacityPSO;

		extern PipelineStateObject BasicInstancingPSO;

		extern PipelineStateObject PunchingPSO;
		extern PipelineStateObject EdgePSO;
		extern PipelineStateObject WaterPSO;
		
		extern PipelineStateObject TestPSO;
		extern PipelineStateObject ParticlePSO;
		extern PipelineStateObject samplerPSO;
		extern PipelineStateObject postEffectPSO;

		// Blend States
		extern ComPtr<ID3D11BlendState> OpacityBS;


		void InitCommonStates(ComPtr<ID3D11Device>& device);

		// 내부적으로 InitCommonStates()에서 사용
		void InitSamplers(ComPtr<ID3D11Device>& device);
		void InitRasterizerStates(ComPtr<ID3D11Device>& device);
		void InitBlendStates(ComPtr<ID3D11Device>& device);
		void InitDepthStencilStates(ComPtr<ID3D11Device>& device);
		void InitPipelineStates(ComPtr<ID3D11Device>& device);
		void InitShaders(ComPtr<ID3D11Device>& device);

	} // namespace Graphics

} // namespace hlab
