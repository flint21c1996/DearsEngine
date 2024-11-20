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
	Assimp::Importer importer;		//모델파일을 읽어들이고, 파싱하는데 사용

	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	// 파일을 읽을 때 쓸 후처리 플래그들.
	const aiScene* pScene = importer.ReadFile(filepath,
		aiProcess_Triangulate |									 // 모든 메쉬를 삼각형으로 분할한다.
		aiProcess_GenNormals |							 // 부드러운 노멀을 생성한다.
		aiProcess_JoinIdenticalVertices |						 // 동일한 버텍스를 합친다.
		aiProcess_ConvertToLeftHanded |							 // 왼손 좌표계로 변환한다.
		aiProcess_CalcTangentSpace                                  // 탄젠트 공간을 계산한다 (AABB를 계산할 때 유용)
	);
	if (pScene)
		return mpFBXLoadHelper->CopyModeldata(pScene);
	else
		return nullptr;
}

Animation* FBXLoader::LoadAnimation(std::string filepath)
{
	// Create an instance of the Importer class
	Assimp::Importer importer;		//모델파일을 읽어들이고, 파싱하는데 사용

	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	// 파일을 읽을 때 쓸 후처리 플래그들.
	const aiScene* pScene = importer.ReadFile(filepath,
		aiProcess_Triangulate |									 // 모든 메쉬를 삼각형으로 분할한다.
		aiProcess_GenSmoothNormals |							 // 부드러운 노멀을 생성한다.
		aiProcess_JoinIdenticalVertices |						 // 동일한 버텍스를 합친다.
		aiProcess_ConvertToLeftHanded							 // 왼손 좌표계로 변환한다.
	);

	return mpFBXLoadHelper->CopyAnimation(pScene);
}

