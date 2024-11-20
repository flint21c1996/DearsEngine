#pragma once
#include "ResourceEnum.h"
#include <filesystem>
#include <string>
#include <iostream>
#include <unordered_set>
#include <list>
#include <unordered_map>

/// <summary>
/// 리소스 경로를 관리하기 위한 매니저
/// 사용하기 더 편하게 바꿨다.
/// 2024.6.4.
/// </summary>
class FileManager
{
public:
	FileManager();
	~FileManager() {};

private:
	std::filesystem::path m_resourceBaseDir;					// 리소스 위치

	struct DirData
	{
		std::string dirName;									// 디렉토리 이름(= 폴더 이름)
		std::list<std::string>* pInDirFiles;					// 내부의 값이 바뀌지 않을 예정이므로 포인터 통째로 받음
	};

	//std::list<std::string> testList;							// 디렉토리 내의 모든 파일을 저장하는 리스트
	
public:
	std::list<DirData*> dirDataList;								// 모든 디렉토리의 파일 정보를 가진 리스트
	void Initialize();

public:
	// 재귀적으로 "폴더를 탐색하고 폴더 내의 모든 파일을 탐색"하는 함수 -> 폴더는 이름만 검색하고 폴더 내의 파일은 전부 출력
	void SearchOneFolderFile(const std::string& _folderName);

	// 재귀적으로 "특정 폴더 안의 1개의 파일만 탐색"하는 함수
	std::string SearchOneFile(const std::string& _folderName, const std::string& _fileName);

	// BasePath의 경로를 뽑아낸다. 폴더 이름만 입력하면 알아서 \\까지 출력함..
	std::string SearchBasePath(const std::string& _folderName);

	// 파일 전체 구조 출력
	void printFolderStructure(const std::filesystem::path& directory, int depth = 0);

	// 파일 전체 구조 출력 및 디렉토리 데이터를 각 리스트에 전부 저장함
	void SaveFolderStructure(const std::filesystem::path& directory);

	/// 테스트용
	void printAll(std::list<DirData*> _dirDataList);


private:
	// 폴더만 검색해서 경로를 뽑아냄 -> 1개의 파일만을 검색함, SerchOneFile()에 사용함
	std::filesystem::path SearchFolderPath(const std::string& _folderName);

	// 특정 폴더의 내부를 처리하는 함수 (Object라면 Object 내부 전체 탐색) | string 값으로 반환
	std::string ProcessFolder(const std::filesystem::path& _folderPath);

	// 파일 처리
	std::string ProcessFilePath(const std::filesystem::path& _filePath);

};

// 파일 시스템의 디렉토리 구조는 트리 구조와 매우 유사하다.
// 이 때문에 파일 시스템의 디렉토리 구조를 활용하여 트리 구조를 간접적으로 사용하는 것이 가능하다.
// 따로 unordered_map을 사용해서 매칭시킬 이유가 그다지 없다. 그냥 디렉터리만 찾는 함수를 이용해 이름을 매칭시키는 건 크게 낭비 없음

// 자료구조로 그냥 리스트를 쓴 이유 : 데이터 개수가 우리가 많아봤자 대부분 100개 이하라고 판단했다.