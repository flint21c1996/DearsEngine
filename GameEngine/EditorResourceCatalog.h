#pragma once

#include <array>
#include <string>

#include "IScene.h"

struct EditorResourceOption
{
	const char* label = "";
	const char* key = "";
};

// 에디터 UI에서 고를 수 있는 리소스 목록을 한곳에 모아둔다.
//
// 지금은 리소스 매니저가 "현재 로드된 모든 리소스 이름"을 열거하는 API를 제공하지 않는다.
// 그래서 Add Object 패널 안에 흩어져 있던 하드코딩 목록을 우선 이 파일로 옮겼다.
// 나중에 GraphicsResourceManager가 리소스 목록 조회 API를 갖게 되면,
// 이 Catalog는 그 API를 읽어서 동적으로 채우는 쪽으로 바꾸면 된다.
namespace EditorResourceCatalog
{
	inline constexpr std::array<EditorResourceOption, 9> RenderTypes =
	{{
		{ "Static Mesh", "StaticMesh" },
		{ "Skinned Mesh", "SkinnedMesh" },
		{ "Equipment Mesh", "EquipmentMesh" },
		{ "Cube Map", "CubeMap" },
		{ "Billboard", "Billboard" },
		{ "PBR Mesh", "PbrMesh" },
		{ "Directional Light", "DirectionalLight" },
		{ "Point Light", "PointLight" },
		{ "Spot Light", "SpotLight" },
	}};

	inline constexpr std::array<SceneRenderType, 9> RenderTypeValues =
	{{
		SceneRenderType::StaticMesh,
		SceneRenderType::SkinnedMesh,
		SceneRenderType::EquipmentMesh,
		SceneRenderType::CubeMap,
		SceneRenderType::Billboard,
		SceneRenderType::PbrMesh,
		SceneRenderType::DirectionalLight,
		SceneRenderType::PointLight,
		SceneRenderType::SpotLight,
	}};

	inline constexpr std::array<EditorResourceOption, 8> VertexBuffers =
	{{
		{ "Box", "MyBox" },
		{ "Sphere", "MySphere" },
		{ "Plane", "MySquare" },
		{ "Billboard Quad", "BillBoardSquare" },
		{ "Cube Map", "CubeMap" },
		{ "Rock_05", "Rock_05" },
		{ "Character 01", "Character 01" },
		{ "Axe 01", "Axe 01" },
	}};

	inline constexpr std::array<EditorResourceOption, 8> Models =
	{{
		{ "None", "" },
		{ "Rock_05", "Rock_05.fbx" },
		{ "Character 01", "Character 01.FBX" },
		{ "Axe 01", "Axe 01.FBX" },
		{ "Hat 04", "Hat 04.FBX" },
		{ "Michelle", "Michelle.fbx" },
		{ "Sung", "Sung.fbx" },
		{ "Paladin Armor", "Armor 05.FBX" },
	}};

	inline constexpr std::array<EditorResourceOption, 11> Textures =
	{{
		{ "None", "" },
		{ "White", "White.png" },
		{ "Red", "Red.png" },
		{ "Yellow", "Yellow.png" },
		{ "Blue", "Blue.png" },
		{ "Stone / ss", "ss.png" },
		{ "Water", "Water.png" },
		{ "Billboard Egg", "pngegg.png" },
		{ "Character Blue", "M-Cos-16-Blue.png" },
		{ "Hair Orange", "Hair-Orange.png" },
		{ "Weapon Black", "Weapon-Black.png" },
	}};

	inline constexpr std::array<EditorResourceOption, 7> Animations =
	{{
		{ "None", "" },
		{ "Slash Attack", "Character@Slash Attack.FBX" },
		{ "Idle", "Character@Idle.FBX" },
		{ "Run Forward", "Character@Run Forward Bare Hands In Place.FBX" },
		{ "Cast Spell", "Character@Cast Spell 02.FBX" },
		{ "Catwalk Idle", "CatwalkIdle.fbx" },
		{ "Catwalk Walk", "CatwalkWalkForward.fbx" },
	}};

	inline constexpr std::array<EditorResourceOption, 3> PbrMaterials =
	{{
		{ "Metal Ball 2", "metalball2" },
		{ "No Texture", "" },
		{ "Custom Slots", "custom" },
	}};

	inline constexpr std::array<EditorResourceOption, 7> PbrTextures =
	{{
		{ "None", "" },
		{ "Albedo", "albedo.png" },
		{ "Normal", "normal.png" },
		{ "AO", "ao.png" },
		{ "Metallic", "metallic.png" },
		{ "Roughness", "roughness.png" },
		{ "Height", "height.png" },
	}};

	inline const char* GetLabel(const EditorResourceOption* options, int count, const std::string& key)
	{
		for (int index = 0; index < count; ++index)
		{
			if (key == options[index].key)
			{
				return options[index].label;
			}
		}

		return key.empty() ? "None" : key.c_str();
	}
}

