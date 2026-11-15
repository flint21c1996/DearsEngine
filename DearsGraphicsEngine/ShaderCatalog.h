#pragma once

#include <string>
#include <vector>

// 에디터에서 오브젝트마다 고를 수 있는 Vertex/Pixel Shader 목록이다.
//
// 이 헤더는 "어떤 셰이더가 있고, 어떤 것끼리 짝이 맞는지"만 설명하며 DX11 타입이 없다.
// 실제 셰이더 객체와 PSO 조합은 MeshRenderer가 이름으로 찾아서 만든다.
// 셰이더를 새로 추가할 때는 여기에 항목을 넣고 MeshRenderer::RenderWithShaders()의 매핑을 한 줄 추가한다.

// Vertex Shader의 출력 구조체와 Pixel Shader의 입력 구조체가 같아야 두 셰이더를 함께 쓸 수 있다.
// Common.hlsli에는 두 가지 구조체가 있으며, 같은 계열끼리만 조합할 수 있다.
enum class ShaderLinkage
{
	// PixelShaderInput: 위치/노멀/UV. 일반 메시용.
	Basic,
	// PBRPixelShaderInput: 위치/노멀/UV/탄젠트. 노멀 맵을 쓰는 PBR 계열용.
	Pbr,
};

struct ShaderInfo
{
	const char* name;
	ShaderLinkage linkage;
	// true면 최종 색상이 아니라 G-Buffer(Albedo/Normal/Material)를 출력한다.
	// Deferred 경로 전용이라 사용자가 직접 고르는 목록에서는 숨긴다.
	bool writesGBuffer;
	// 에디터 툴팁에 보여 줄 설명이다.
	const char* description;
};

namespace ShaderCatalog
{
	namespace VertexShader
	{
		constexpr const char* Static = "Static";
		constexpr const char* Skinned = "Skinned";
		constexpr const char* Equipment = "Equipment";
		constexpr const char* Water = "Water";
		constexpr const char* Billboard = "Billboard";
		constexpr const char* Pbr = "Pbr";
	}

	namespace PixelShader
	{
		constexpr const char* Lit = "Lit";
		constexpr const char* Unlit = "Unlit";
		constexpr const char* PbrLit = "PbrLit";
		constexpr const char* ThinFilm = "ThinFilm";
		constexpr const char* GBuffer = "GBuffer";
	}

	inline const std::vector<ShaderInfo>& VertexShaders()
	{
		static const std::vector<ShaderInfo> shaders =
		{
			{ VertexShader::Static, ShaderLinkage::Basic, false, "움직이지 않는 일반 메시" },
			{ VertexShader::Skinned, ShaderLinkage::Basic, false, "본 애니메이션 메시 (Bone 상수 버퍼 필요)" },
			{ VertexShader::Equipment, ShaderLinkage::Basic, false, "캐릭터 본에 붙는 장비 메시 (Target Bone 상수 버퍼 필요)" },
			{ VertexShader::Water, ShaderLinkage::Basic, false, "UV가 흐르는 물 표면 (Water 상수 버퍼 필요)" },
			{ VertexShader::Billboard, ShaderLinkage::Basic, false, "항상 카메라를 바라보는 평면" },
			{ VertexShader::Pbr, ShaderLinkage::Pbr, false, "탄젠트/Height Map을 쓰는 PBR 메시 (PBR 상수 버퍼 필요)" },
		};
		return shaders;
	}

	inline const std::vector<ShaderInfo>& PixelShaders()
	{
		static const std::vector<ShaderInfo> shaders =
		{
			{ PixelShader::Lit, ShaderLinkage::Basic, false, "Diffuse 텍스처 + 기본 조명" },
			{ PixelShader::Unlit, ShaderLinkage::Basic, false, "조명 없이 Diffuse 텍스처 색만 출력" },
			{ PixelShader::PbrLit, ShaderLinkage::Pbr, false, "Forward PBR 조명 (IBL + 그림자)" },
			{ PixelShader::ThinFilm, ShaderLinkage::Pbr, false, "박막 간섭 무지갯빛 표면. Forward 전용" },
			{ PixelShader::GBuffer, ShaderLinkage::Pbr, true, "G-Buffer 기록. 조명은 Lighting 패스가 계산" },
		};
		return shaders;
	}

	inline const ShaderInfo* Find(const std::vector<ShaderInfo>& shaders, const std::string& name)
	{
		for (const ShaderInfo& shader : shaders)
		{
			if (name == shader.name)
			{
				return &shader;
			}
		}
		return nullptr;
	}

	inline const ShaderInfo* FindVertexShader(const std::string& name) { return Find(VertexShaders(), name); }
	inline const ShaderInfo* FindPixelShader(const std::string& name) { return Find(PixelShaders(), name); }

	// 두 셰이더가 모두 존재하고 입출력 구조체 계열이 같을 때만 함께 쓸 수 있다.
	inline bool AreCompatible(const std::string& vertexShader, const std::string& pixelShader)
	{
		const ShaderInfo* vs = FindVertexShader(vertexShader);
		const ShaderInfo* ps = FindPixelShader(pixelShader);
		return vs && ps && vs->linkage == ps->linkage;
	}

	// 계열마다 "일단 화면에 제대로 나오는" 기본 Pixel Shader이다.
	// 사용자가 고른 조합이 맞지 않을 때 대신 사용한다.
	inline const char* DefaultPixelShaderFor(ShaderLinkage linkage)
	{
		return linkage == ShaderLinkage::Pbr ? PixelShader::PbrLit : PixelShader::Lit;
	}
}
