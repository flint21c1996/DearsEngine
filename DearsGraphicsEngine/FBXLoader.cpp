#include "FBXLoader.h"

FBXLoader::FBXLoader()
{
	mpFBXLoadHelper = new FBXLoadHelper();
}

FBXLoader::~FBXLoader()
{
	delete mpFBXLoadHelper;
}

Model* FBXLoader::LoadModel(std::string filepath)
{
	// Create an instance of the Importer class
	Assimp::Importer importer;		//�������� �о���̰�, �Ľ��ϴµ� ���

	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	// ������ ���� �� �� ��ó�� �÷��׵�.
	const aiScene* pScene = importer.ReadFile(filepath,
		aiProcess_Triangulate |									 // ��� �޽��� �ﰢ������ �����Ѵ�.
		aiProcess_GenNormals |							 // �ε巯�� ����� �����Ѵ�.
		aiProcess_JoinIdenticalVertices |						 // ������ ���ؽ��� ��ģ��.
		aiProcess_ConvertToLeftHanded |							 // �޼� ��ǥ��� ��ȯ�Ѵ�.
		aiProcess_CalcTangentSpace                                  // ź��Ʈ ������ ����Ѵ� (AABB�� ����� �� ����)
	);
	if (pScene)
		return mpFBXLoadHelper->CopyModeldata(pScene);
	else
		return nullptr;
}

Animation* FBXLoader::LoadAnimation(std::string filepath)
{
	// Create an instance of the Importer class
	Assimp::Importer importer;		//�������� �о���̰�, �Ľ��ϴµ� ���

	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	// ������ ���� �� �� ��ó�� �÷��׵�.
	const aiScene* pScene = importer.ReadFile(filepath,
		aiProcess_Triangulate |									 // ��� �޽��� �ﰢ������ �����Ѵ�.
		aiProcess_GenSmoothNormals |							 // �ε巯�� ����� �����Ѵ�.
		aiProcess_JoinIdenticalVertices |						 // ������ ���ؽ��� ��ģ��.
		aiProcess_ConvertToLeftHanded							 // �޼� ��ǥ��� ��ȯ�Ѵ�.
	);

	return mpFBXLoadHelper->CopyAnimation(pScene);
}

