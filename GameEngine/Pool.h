#pragma once
#include <vector>
#include <type_traits>	//is_pointer
#include <tuple>
#include <queue>

#ifdef _DEBUG
#include <iostream>
#define DEBUG_LOG(message) std::cout<<message<<std::endl
#else
#define DEBUG_LOG(message)
#endif // DEBUG

template<typename T>
class Pool
{
public:
	Pool(int _size)
	{
		ResizeVec(_size);
		mSize = _size;

	};
	Pool()
	{
		unsigned int _size = 10;		//기본 크기는 10으로 한다.
		ResizeVec(_size);
		mSize = _size;
	}
	~Pool()
	{
		ClearPool();
	}
private:
	std::vector<T> mObjectVec;
	std::vector<unsigned int> misAvailableVec;	//사용가능한가? 1 = 사용가능하다, 0 = 이미 사용중이다. 그외는 입맛에 맞게..
	std::queue<unsigned int> misAvailableIndex;
	unsigned int mSize = 0;
public:
	//오브젝트를 추가한다.
	unsigned int AddObject(T& _object);

	//오브젝트를 지운다.
	bool EraseObject(unsigned int _index);

	//풀을 비운다.
	void ClearPool();

	//오브젝트 풀의 사이즈를 변경한다.
	void ResizeVec(unsigned int _size);

	//특정 오브젝트의 주소를 반환한다.
	T& GetObject(unsigned int _index);

	//특정 오브젝트를 업데이트한다.
	bool UpdateObject(unsigned int _index, T& _object);

	//오브젝트 풀을 반환한다.
	const std::tuple<std::vector<T>*, std::vector<unsigned int>*> GetPoolAndAvailableVec();
	std::vector<T>* GetPool();
	std::vector<unsigned int>* GetAvailableVec();

	//오브젝트가 포인터일 경우 해당 메모리를 해제한다.
	bool DeleteObjMemory(unsigned int _index);

	unsigned int GetSize();
	unsigned int GetObjectVecSize();
	unsigned int GetAvailableVecMemorySize();

	void ChangeState(unsigned int _startIndex, unsigned int _endIndex, unsigned int value);
	void ChangeState(unsigned int _index, unsigned int value);
};

template<typename T>
unsigned int Pool<T>::GetAvailableVecMemorySize()
{
	return sizeof(unsigned int) * mSize;
}

template<typename T>
unsigned int Pool<T>::GetObjectVecSize()
{
	return sizeof(T) * mSize;
}

template<typename T>
std::vector<T>* Pool<T>::GetPool()
{
	return &mObjectVec;
}

template<typename T>
std::vector<unsigned int>* Pool<T>::GetAvailableVec()
{
	return &misAvailableVec;
}

template<typename T>
const std::tuple<std::vector<T>*, std::vector<unsigned int>*> Pool<T>::GetPoolAndAvailableVec()
{
	return { &mObjectVec, &misAvailableVec };
}

template<typename T>
void Pool<T>::ChangeState(unsigned int _index, unsigned int value)
{
	misAvailableVec[_index] = value;
}

template<typename T>
void Pool<T>::ChangeState(unsigned int _startIndex, unsigned int _endIndex, unsigned int value)
{
	for (unsigned int i = _startIndex; i <= _endIndex; i++)
	{
		ChangeState(i, value);
	}
}

template<typename T>
unsigned int Pool<T>::GetSize()
{
	return mSize;
}

template<typename T>
bool Pool<T>::DeleteObjMemory(unsigned int _index)
{
	if constexpr (std::is_pointer<T>::value)
	{
		if (mObjectVec[_index])
		{
			delete mObjectVec[_index];
			mObjectVec[_index] = nullptr;
			return true;
		}
		else
		{
			DEBUG_LOG("DeleteObjMemory - AlreadyNull");
			return false;
		}
	}
	else
	{
		DEBUG_LOG("DeleteObjMemory - NotPointer");
		return false;
	}
}

template<typename T>
bool Pool<T>::UpdateObject(unsigned int _index, T& _object)
{
	if (mSize <= _index)
	{
		DEBUG_LOG("UpdateObject - NotExist");
		return false;
	}
	if (misAvailableVec[_index])
	{
		DEBUG_LOG("UpdateObject - NotExist");
		return false;
	}
	else
	{
		mObjectVec[_index] = _object;
		return true;
	}
}



template<typename T>
T& Pool<T>::GetObject(unsigned int _index)
{
	if (mSize <= _index)
	{
		DEBUG_LOG("UpdateObject - OverSize");
	}
	if (misAvailableVec[_index])
	{
		DEBUG_LOG("GetObject - NotExist");
	}
	else
	{
		return mObjectVec[_index];
	}
}

template<typename T>
void Pool<T>::ResizeVec(unsigned int _size)
{
	mObjectVec.resize(_size);
	misAvailableVec.resize(_size);

	if (_size > mSize)
	{
		for (unsigned int i = mSize; i < _size; i++)
		{
			misAvailableVec[i] = true;
			misAvailableIndex.push(i);
		}
		mSize = _size;
	}
}


template<typename T>
void Pool<T>::ClearPool()
{
	for (int i = 0; i < mObjectVec.size(); i++)
	{
		EraseObject(i);
	}
	ResizeVec(0);
}


template<typename T>
bool Pool<T>::EraseObject(unsigned int _index)
{
	if (mSize <= _index)
	{
		DEBUG_LOG("UpdateObject - OverSize");
		return false;
	}

	if (misAvailableVec[_index])
	{
		DEBUG_LOG("EraseObject - NotExist");
		return false;
	}
	else
	{
		misAvailableVec[_index] = true;
		misAvailableIndex.push(_index);
		return true;
	}
}

template<typename T>
unsigned int Pool<T>::AddObject(T& _object)
{
	unsigned int targetIndex;
	if (misAvailableIndex.empty())
	{
		ResizeVec(mSize + 10);
	}
	targetIndex = misAvailableIndex.front();
	misAvailableIndex.pop();
	mObjectVec[targetIndex] = _object;
	misAvailableVec[targetIndex] = false;
	return targetIndex;
}
