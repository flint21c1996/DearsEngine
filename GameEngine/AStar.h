#pragma once
#include <directxTk/SimpleMath.h>
#include <queue>
#include <unordered_set>
#include <iostream>

using DirectX::SimpleMath::Vector2;

class AStar
{
public:
	AStar();
	~AStar();

	// Node ����ü ����
	struct Node
	{
		Vector2 position;
		float gCost; // ���� ��忡�� �ش� �������� ���� �ҿ� ���
		float hCost; // ���谪
		float GetFCost() const { return gCost + hCost; }
		Node* parent = nullptr;
	};

	// Node �� ������ ����
	// std::priority_queue�� �⺻������ �ִ� ���� ����Ͽ� ���� ū ��Ҹ� ��Ʈ�� ����������, 
	// Ŀ���͸���¡�� �� ������ �����Ͽ� �ּ� ���� ����ų� �ٸ� ���ؿ� ���� ����� �켱������ ������ �� �ִ�.
	struct NodeCompare
	{
		bool operator()(const Node* lhs, const Node* rhs)
		{
			if (lhs)
				return lhs->GetFCost() > rhs->GetFCost();
		}
	};


	// [������, ��ǥ��, �� ����] XZ��ǥ�� A* �˰��� ���� 
	std::vector<Vector2> Astar(const Vector2& _startPoint, const Vector2& _goalPoint, const std::vector<std::vector<int>> _map);

	// [map, ���] �Է��� ���� ������� �����̴� ��θ� print 
	void printAstarMap(const std::vector<std::vector<int>>& _map, std::vector<Vector2> _path);
	// [���] �ܼ��� �����̴� ��θ� ���ڷ� Ȯ�� 
	void PrintSimpleAstar(const std::vector<Vector2> _path);

private:
	// ���� ��ġ���� ������ �� �ִ� ���� ����
	std::vector<Vector2> directions;

	// ���� �⺻���� AStar
	std::vector<Vector2> BasicAstar(const Vector2& _startPoint, const Vector2& _goalPoint, const std::vector<Vector2>& obstacles, const Vector2& _mapStart, const Vector2& _mapEnd);
	
	// ���� Astar���� ����ϴ� ��ֹ� ��ǥ�� ��ȯ���� ����ϱ� ���� �Լ�
	std::vector<Vector2> ConvertMapToVector(const std::vector<std::vector<int>> _map);

	// �Ÿ��� ��� �Լ�
	float Distance(const Vector2& _pos1, const Vector2& _pos2);

	// ��ֹ� ���ο� ���� ������ �� �ִ��� Ȯ���ϴ� �Լ�
	bool IsMovable(const Vector2& _pos, const std::vector<Vector2>& _obstaclePos, const Vector2& _mapStartPos, const Vector2& _mapEndPos);
};

// �ؽ� �Լ� : Vector2�� ���� Ű���� ����ϱ� ���� ����
template <>
struct std::hash<Vector2>
{
	size_t operator()(const Vector2& v) const noexcept
	{
		return std::hash<float>()(v.x) ^ (std::hash<float>()(v.y) << 1);
	}
};
