#pragma once
#include<unordered_map>
#include "Pool.h"
#include "Entity.h"
/// <summary>
/// 엔티티와 컴포넌트들을 연결해주는 클래스
/// </summary>
class EntityManager
{
public:
	EntityManager();
	~EntityManager();

private:
	void AddEntity();
	void AddComponent();

	//엔티티를 보관하는 자료구조
	//컴포넌트를 보관하는 자료구조 
	
	Pool<Entity>* EntityPool;
	
	//엔티티를 보관하는 자료구조를 뭘 써야하는가...?
	//오브젝트 풀을 써야하는가..? 아니면 그냥 벡터..?
	//어자피 인덱스가 해당 엔티티의 ID가 될텐데
	///엔티티와 컴포넌트를 연결하기 위해서는 엔티티만을 가지고 있는 자료구조 컴포넌트만을 가지고있는 자료구조가 필요하다.
	//std::unordered_map<unsigned int, unsigned int>
	//ParticlePool
	//위의 두개를 연결하는 클래스가 필요하다.
};

