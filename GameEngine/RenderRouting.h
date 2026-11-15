#pragma once

#include <string>

#include "IDearsRenderer.h"
#include "IScene.h"
#include "ShaderCatalog.h"

// 오브젝트 하나를 "어느 패스에서, 어떤 셰이더 조합으로" 그릴지 결정한 결과이다.
struct MeshRoute
{
	// false면 CubeMap/라이트처럼 셰이더를 고를 수 없는 종류이다.
	bool isConfigurable = false;
	// true면 Geometry 패스에서 G-Buffer에 기록하고 Lighting 패스가 조명을 계산한다.
	// false면 Forward 패스에서 최종 색상까지 직접 그린다.
	bool deferred = false;
	// true면 vertexShader/pixelShader 이름으로 그린다(DrawMeshWithShaders).
	// false면 오브젝트 종류의 기본 경로(DrawMesh + MeshDrawMode)를 그대로 쓴다.
	bool useNamedShaders = false;
	// 실제로 적용되는 셰이더 이름이다. 재정의가 없으면 오브젝트 종류의 기본값이 들어간다.
	std::string vertexShader;
	std::string pixelShader;
};

// 오브젝트 종류가 원래 사용하는 Vertex Shader 이름이다. 셰이더를 고를 수 없는 종류는 nullptr이다.
inline const char* DefaultVertexShaderFor(SceneRenderType renderType)
{
	switch (renderType)
	{
	case SceneRenderType::StaticMesh:
		return ShaderCatalog::VertexShader::Static;
	case SceneRenderType::SkinnedMesh:
		return ShaderCatalog::VertexShader::Skinned;
	case SceneRenderType::EquipmentMesh:
		return ShaderCatalog::VertexShader::Equipment;
	case SceneRenderType::Billboard:
		return ShaderCatalog::VertexShader::Billboard;
	case SceneRenderType::PbrMesh:
		return ShaderCatalog::VertexShader::Pbr;
	default:
		return nullptr;
	}
}

inline const char* DefaultPixelShaderFor(SceneRenderType renderType, MaterialShadingModel shadingModel)
{
	switch (renderType)
	{
	case SceneRenderType::StaticMesh:
	case SceneRenderType::SkinnedMesh:
	case SceneRenderType::EquipmentMesh:
		return ShaderCatalog::PixelShader::Lit;
	case SceneRenderType::Billboard:
		return ShaderCatalog::PixelShader::Unlit;
	case SceneRenderType::PbrMesh:
		return shadingModel == MaterialShadingModel::ThinFilm
			? ShaderCatalog::PixelShader::ThinFilm
			: ShaderCatalog::PixelShader::PbrLit;
	default:
		return nullptr;
	}
}

// 에디터에서 고른 설정(렌더 경로, 셰이더 재정의)을 실제 렌더 경로로 바꾼다.
//
// Inspector와 RenderDispatcher가 같은 함수를 쓰기 때문에
// "패널에 표시되는 셰이더"와 "실제로 그려지는 셰이더"가 어긋나지 않는다.
//
// 규칙:
// - 재정의가 비어 있거나 목록에 없는 이름이면 오브젝트 종류의 기본 셰이더를 쓴다.
// - Vertex/Pixel Shader의 계열(ShaderLinkage)이 다르면 Vertex Shader 계열의 기본 Pixel Shader로 바꾼다.
// - Deferred는 G-Buffer를 채울 수 있는 PBR 계열에서만 가능하다. Thin Film은 최종 색상을 직접
//   계산하는 셰이더라 Forward로만 그린다. 조건이 안 맞으면 Deferred를 골라도 Forward로 그린다.
// - Deferred가 되면 Pixel Shader는 항상 G-Buffer 기록용으로 고정된다.
inline MeshRoute ResolveMeshRoute(
	SceneRenderType renderType,
	SceneRenderPath renderPath,
	MaterialShadingModel shadingModel,
	const std::string& vertexShaderOverride,
	const std::string& pixelShaderOverride)
{
	MeshRoute route;

	const char* defaultVertexShader = DefaultVertexShaderFor(renderType);
	const char* defaultPixelShader = DefaultPixelShaderFor(renderType, shadingModel);
	if (!defaultVertexShader || !defaultPixelShader)
	{
		return route;
	}
	route.isConfigurable = true;

	const ShaderInfo* vertexShader = ShaderCatalog::FindVertexShader(vertexShaderOverride);
	const bool hasVertexOverride = vertexShader != nullptr;
	if (!vertexShader)
	{
		vertexShader = ShaderCatalog::FindVertexShader(defaultVertexShader);
	}

	// G-Buffer 기록용 셰이더는 Deferred 경로가 자동으로 고르는 것이라 재정의로 받지 않는다.
	const ShaderInfo* pixelShader = ShaderCatalog::FindPixelShader(pixelShaderOverride);
	if (pixelShader && pixelShader->writesGBuffer)
	{
		pixelShader = nullptr;
	}
	const bool hasPixelOverride = pixelShader != nullptr;
	if (!pixelShader)
	{
		pixelShader = ShaderCatalog::FindPixelShader(defaultPixelShader);
	}

	if (pixelShader->linkage != vertexShader->linkage)
	{
		pixelShader = ShaderCatalog::FindPixelShader(
			ShaderCatalog::DefaultPixelShaderFor(vertexShader->linkage));
	}

	route.useNamedShaders = hasVertexOverride || hasPixelOverride;
	route.vertexShader = vertexShader->name;
	route.pixelShader = pixelShader->name;

	const bool isThinFilm = route.pixelShader == ShaderCatalog::PixelShader::ThinFilm;
	route.deferred =
		renderPath == SceneRenderPath::Deferred &&
		vertexShader->linkage == ShaderLinkage::Pbr &&
		!isThinFilm;
	if (route.deferred)
	{
		route.pixelShader = ShaderCatalog::PixelShader::GBuffer;
	}

	return route;
}

// Shadow Map에는 화면에 그릴 때와 같은 정점 변형을 적용해야 실루엣이 일치한다.
// Vertex Shader를 재정의했으면 그 셰이더에 맞는 그림자 경로를 고르고,
// 아니면 오브젝트 종류에 맞는 기존 경로를 쓴다.
inline ShadowCasterMode ResolveShadowCasterMode(
	SceneRenderType renderType,
	const std::string& vertexShaderOverride)
{
	const ShaderInfo* vertexShader = ShaderCatalog::FindVertexShader(vertexShaderOverride);
	if (vertexShader)
	{
		const std::string name = vertexShader->name;
		if (name == ShaderCatalog::VertexShader::Skinned)
		{
			return ShadowCasterMode::Skinned;
		}
		if (name == ShaderCatalog::VertexShader::Equipment)
		{
			return ShadowCasterMode::Equipment;
		}
		if (name == ShaderCatalog::VertexShader::Pbr)
		{
			return ShadowCasterMode::Pbr;
		}
		return ShadowCasterMode::Static;
	}

	switch (renderType)
	{
	case SceneRenderType::PbrMesh:
		return ShadowCasterMode::Pbr;
	case SceneRenderType::SkinnedMesh:
		return ShadowCasterMode::Skinned;
	case SceneRenderType::EquipmentMesh:
		return ShadowCasterMode::Equipment;
	default:
		return ShadowCasterMode::Static;
	}
}
