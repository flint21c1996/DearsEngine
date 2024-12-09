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
		unsigned int _size = 10;		//�⺻ ũ��� 10���� �Ѵ�.
		ResizeVec(_size);
		mSize = _size;
	}
	~Pool()
	{
		ClearPool();
	}
private:
	std::vector<T> mObjectVec;
	std::vector<unsigned int> misAvailableVec;	//��밡���Ѱ�? 1 = ��밡���ϴ�, 0 = �̹� ������̴�. �׿ܴ� �Ը��� �°�..
	std::queue<unsigned int> misAvailableIndex;
	unsigned int mSize = 0;
public:
	//������Ʈ�� �߰��Ѵ�.
	unsigned int AddObject(T& _object);

	//������Ʈ�� �����.
	bool EraseObject(unsigned int _index);

	//Ǯ�� ����.
	void ClearPool();

	//������Ʈ Ǯ�� ����� �����Ѵ�.
	void ResizeVec(unsigned int _size);

	//Ư�� ������Ʈ�� �ּҸ� ��ȯ�Ѵ�.
	T& GetObject(unsigned int _index);

	//Ư�� ������Ʈ�� ������Ʈ�Ѵ�.
	bool UpdateObject(unsigned int _index, T& _object);

	//������Ʈ Ǯ�� ��ȯ�Ѵ�.
	const std::tuple<std::vector<T>*, std::vector<unsigned int>*> GetPoolAndAvailableVec();
	std::vector<T>* GetPool();
	std::vector<unsigned int>* GetAvailableVec();

	//������Ʈ�� �������� ��� �ش� �޸𸮸� �����Ѵ�.
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
