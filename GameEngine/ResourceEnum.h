#pragma once
	
// ���� Ž�� ������ �Է��� �� : m_pFileManager->printFolderStructure(m_pFileManager->m_resourceBaseDir, 0); ����ؼ� ����
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