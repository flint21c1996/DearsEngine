#pragma once
#include "ResourceEnum.h"
#include <filesystem>
#include <string>
#include <iostream>
#include <unordered_set>
#include <list>
#include <unordered_map>

/// <summary>
/// ���ҽ� ��θ� �����ϱ� ���� �Ŵ���
/// ����ϱ� �� ���ϰ� �ٲ��.
/// 2024.6.4.
/// </summary>
class FileManager
{
public:
	FileManager();
	~FileManager() {};

private:
	std::filesystem::path m_resourceBaseDir;					// ���ҽ� ��ġ

	struct DirData
	{
		std::string dirName;									// ���丮 �̸�(= ���� �̸�)
		std::list<std::string>* pInDirFiles;					// ������ ���� �ٲ��� ���� �����̹Ƿ� ������ ��°�� ����
	};

	//std::list<std::string> testList;							// ���丮 ���� ��� ������ �����ϴ� ����Ʈ
	
public:
	std::list<DirData*> dirDataList;								// ��� ���丮�� ���� ������ ���� ����Ʈ
	void Initialize();

public:
	// ��������� "������ Ž���ϰ� ���� ���� ��� ������ Ž��"�ϴ� �Լ� -> ������ �̸��� �˻��ϰ� ���� ���� ������ ���� ���
	void SearchOneFolderFile(const std::string& _folderName);

	// ��������� "Ư�� ���� ���� 1���� ���ϸ� Ž��"�ϴ� �Լ�
	std::string SearchOneFile(const std::string& _folderName, const std::string& _fileName);

	// BasePath�� ��θ� �̾Ƴ���. ���� �̸��� �Է��ϸ� �˾Ƽ� \\���� �����..
	std::string SearchBasePath(const std::string& _folderName);

	// ���� ��ü ���� ���
	void printFolderStructure(const std::filesystem::path& directory, int depth = 0);

	// ���� ��ü ���� ��� �� ���丮 �����͸� �� ����Ʈ�� ���� ������
	void SaveFolderStructure(const std::filesystem::path& directory);

	/// �׽�Ʈ��
	void printAll(std::list<DirData*> _dirDataList);


private:
	// ������ �˻��ؼ� ��θ� �̾Ƴ� -> 1���� ���ϸ��� �˻���, SerchOneFile()�� �����
	std::filesystem::path SearchFolderPath(const std::string& _folderName);

	// Ư�� ������ ���θ� ó���ϴ� �Լ� (Object��� Object ���� ��ü Ž��) | string ������ ��ȯ
	std::string ProcessFolder(const std::filesystem::path& _folderPath);

	// ���� ó��
	std::string ProcessFilePath(const std::filesystem::path& _filePath);

};

// ���� �ý����� ���丮 ������ Ʈ�� ������ �ſ� �����ϴ�.
// �� ������ ���� �ý����� ���丮 ������ Ȱ���Ͽ� Ʈ�� ������ ���������� ����ϴ� ���� �����ϴ�.
// ���� unordered_map�� ����ؼ� ��Ī��ų ������ �״��� ����. �׳� ���͸��� ã�� �Լ��� �̿��� �̸��� ��Ī��Ű�� �� ũ�� ���� ����

// �ڷᱸ���� �׳� ����Ʈ�� �� ���� : ������ ������ �츮�� ���ƺ��� ��κ� 100�� ���϶�� �Ǵ��ߴ�.