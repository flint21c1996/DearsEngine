#pragma once
#include <assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>
#include<string>
#include<variant>
#include "FBXLoadHelper.h"
#include"ModelInfo.h"
///외부 파일을 읽는 클래스, assimpLibrary를 쓴다.
class FBXLoader
{
public:
	FBXLoader();
	~FBXLoader();
public:
	FBXLoadHelper* mpFBXLoadHelper;
public:
	Model* LoadModel(std::string filepath);
	Animation* LoadAnimation(std::string filepath);
};