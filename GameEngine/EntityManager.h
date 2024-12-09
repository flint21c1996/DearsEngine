#pragma once
#include<unordered_map>
#include "Pool.h"
#include "Entity.h"
/// <summary>
/// ��ƼƼ�� ������Ʈ���� �������ִ� Ŭ����
/// </summary>
class EntityManager
{
public:
	EntityManager();
	~EntityManager();

private:
	void AddEntity();
	void AddComponent();

	//��ƼƼ�� �����ϴ� �ڷᱸ��
	//������Ʈ�� �����ϴ� �ڷᱸ�� 
	
	Pool<Entity>* EntityPool;
	
	//��ƼƼ�� �����ϴ� �ڷᱸ���� �� ����ϴ°�...?
	//������Ʈ Ǯ�� ����ϴ°�..? �ƴϸ� �׳� ����..?
	//������ �ε����� �ش� ��ƼƼ�� ID�� ���ٵ�
	///��ƼƼ�� ������Ʈ�� �����ϱ� ���ؼ��� ��ƼƼ���� ������ �ִ� �ڷᱸ�� ������Ʈ���� �������ִ� �ڷᱸ���� �ʿ��ϴ�.
	//std::unordered_map<unsigned int, unsigned int>
	//ParticlePool
	//���� �ΰ��� �����ϴ� Ŭ������ �ʿ��ϴ�.
};

