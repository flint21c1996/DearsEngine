#include "FileManager.h"
#include "ResourceEnum.h"

FileManager::FileManager()
{
}

void FileManager::Initialize()
{
	// 현재 경로 얻어오기
	std::filesystem::path m_currentPath = std::filesystem::current_path();

	// 리소스 위치로 이동하기
	m_resourceBaseDir = m_currentPath.parent_path().parent_path() / "Resources";		///절대경로

	// 디렉토리 구조를 출력하는 함수
	//printFolderStructure(m_resourceBaseDir, 0);
}

void FileManager::SearchOneFolderFile(const std::string& _folderName)
{
	for (const auto& folderEntry : std::filesystem::recursive_directory_iterator(m_resourceBaseDir))
	{
		//폴더인 경우 처리
		if (!folderEntry.is_regular_file() && folderEntry.path().filename() == _folderName)
		{
			// 폴더 처리 작업 수행
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
	//// 파일 탐색 및 처리 작업 수행
	for (const auto& entry : std::filesystem::recursive_directory_iterator(SearchFolderPath(_folderName)))
	{
		// 파일인 경우 처리
		if (entry.is_regular_file() && entry.path().filename() == _fileName)
		{
			// 파일 처리 작업 수행
			return ProcessFilePath(entry.path());
		}
	}
}

std::filesystem::path FileManager::SearchFolderPath(const std::string& _folderName)
{
	for (const auto& folderEntry : std::filesystem::recursive_directory_iterator(m_resourceBaseDir))
	{
		//폴더인 경우 처리
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
		//폴더인 경우 처리
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
			// 디렉토리인 경우 이름 출력
			std::cout << std::string(depth * 5, ' ') << "|___" << entry.path().filename() << "\n";
			// 모든 하위 디렉토리 구조 출력 (재귀 호출)
			printFolderStructure(entry.path(), depth + 1);
		}
	}
}

/// 나중에 여유 되면 더 만들어보기..
void FileManager::SaveFolderStructure(const std::filesystem::path& directory)
{
	for (const auto& entry : std::filesystem::directory_iterator(m_resourceBaseDir))
	{
		DirData tempDirData;
		std::list<std::string> testList;
		if (entry.is_directory())
		{
			// 디렉토리인 경우 이름 출력
			std::cout << entry.path().filename() << "\n";
			tempDirData.dirName = entry.path().filename().string();

			testList.push_back(ProcessFilePath(entry.path()));

			// 모든 하위 디렉토리 구조 출력 (재귀 호출)
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

