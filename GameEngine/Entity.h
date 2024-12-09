#pragma once
/// <summary>
/// 엔티티 컴포넌트 시트템을 만들기 위한 Entity
/// </summary>
class Entity
{
public:
	Entity(unsigned int _id);
	~Entity();

public:
	unsigned int mEntityId;
};

