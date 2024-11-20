#pragma once
	
// 깊이 탐색 순으로 입력할 것 : m_pFileManager->printFolderStructure(m_pFileManager->m_resourceBaseDir, 0); 출력해서 참조
enum class FILE_NAME
{
	CUBEMAP,	// dds
	GRAPHICS,	// fbx, png
	LEVELS,
	OBJECTS,
	EQUIPMENTS,
	NPC,
	MICHELLE, 
	PALADIN,
	PLAYER,
	WEAPON,
	HLSL,		// hlsl
	SOUND,

	END
};