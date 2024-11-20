#include "FileManager.h"
#include "ResourceEnum.h"

FileManager::FileManager()
{
}

void FileManager::Initialize()
{
	// ���� ��� ������
	std::filesystem::path m_currentPath = std::filesystem::current_path();

	// ���ҽ� ��ġ�� �̵��ϱ�
	m_resourceBaseDir = m_currentPath.parent_path().parent_path() / "Resources";		///������

	// ���丮 ������ ����ϴ� �Լ�
	//printFolderStructure(m_resourceBaseDir, 0);
}

void FileManager::SearchOneFolderFile(const std::string& _folderName)
{
	for (const auto& folderEntry : std::filesystem::recursive_directory_iterator(m_resourceBaseDir))
	{
		//������ ��� ó��
		if (!folderEntry.is_regular_file() && folderEntry.path().filename() == _folderName)
		{
			// ���� ó�� �۾� ����
			std::cout << "[Folder name] : " << folderEntry.path() << std::endl;

			for (const auto& fileEntry : std::filesystem::directory_iterator(folderEntry.path()))
			{
				ProcessFilePath(fileEntry.path());
			}
		}
	}
}

std::string FileManager::SearchOneFile(const std::string& _folderName, const std::string& _fileName)
{
	//// ���� Ž�� �� ó�� �۾� ����
	for (const auto& entry : std::filesystem::recursive_directory_iterator(SearchFolderPath(_folderName)))
	{
		// ������ ��� ó��
		if (entry.is_regular_file() && entry.path().filename() == _fileName)
		{
			// ���� ó�� �۾� ����
			return ProcessFilePath(entry.path());
		}
	}
}

std::filesystem::path FileManager::SearchFolderPath(const std::string& _folderName)
{
	for (const auto& folderEntry : std::filesystem::recursive_directory_iterator(m_resourceBaseDir))
	{
		//������ ��� ó��
		if (!folderEntry.is_regular_file() && folderEntry.path().filename() == _folderName)
		{
			return folderEntry.path();
		}
	}
}

std::string FileManager::ProcessFolder(const std::filesystem::path& _folderPath)
{
	size_t fileCount = 0;
	for (const auto& fileEntry : std::filesystem::directory_iterator(_folderPath))
	{
		fileCount++;
		std::cout << "File count: " << fileCount << std::endl;
		return ProcessFilePath(fileEntry.path());
	}
}

std::string FileManager::SearchBasePath(const std::string& _folderName)
{
	for (const auto& folderEntry : std::filesystem::recursive_directory_iterator(m_resourceBaseDir))
	{
		//������ ��� ó��
		if (!folderEntry.is_regular_file() && folderEntry.path().filename() == _folderName)
		{
			return folderEntry.path().string() + "\\";
		}
	}
}

void FileManager::printFolderStructure(const std::filesystem::path& directory, int depth /*= 0*/)
{
	for (const auto& entry : std::filesystem::directory_iterator(directory))
	{
		if (entry.is_directory())
		{
			// ���丮�� ��� �̸� ���
			std::cout << std::string(depth * 5, ' ') << "|___" << entry.path().filename() << "\n";
			// ��� ���� ���丮 ���� ��� (��� ȣ��)
			printFolderStructure(entry.path(), depth + 1);
		}
	}
}

/// ���߿� ���� �Ǹ� �� ������..
void FileManager::SaveFolderStructure(const std::filesystem::path& directory)
{
	for (const auto& entry : std::filesystem::directory_iterator(m_resourceBaseDir))
	{
		DirData tempDirData;
		std::list<std::string> testList;
		if (entry.is_directory())
		{
			// ���丮�� ��� �̸� ���
			std::cout << entry.path().filename() << "\n";
			tempDirData.dirName = entry.path().filename().string();

			testList.push_back(ProcessFilePath(entry.path()));

			// ��� ���� ���丮 ���� ��� (��� ȣ��)
			SaveFolderStructure(entry.path());
		}
		tempDirData.pInDirFiles = &testList;
		dirDataList.push_back(&tempDirData);
	}
}

void FileManager::printAll(std::list<DirData*> _dirDataList)
{
	for (const auto& e : _dirDataList)
	{
		std::cout << e->dirName << std::endl;
		std::cout << e->pInDirFiles << std::endl;
	}
}

std::string FileManager::ProcessFilePath(const std::filesystem::path& _filePath)
{
	std::cout << "File Path: " << _filePath << std::endl;
	return _filePath.string();
}

