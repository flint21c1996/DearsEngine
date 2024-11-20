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

	// Node 구조체 정의
	struct Node
	{
		Vector2 position;
		float gCost; // 시작 노드에서 해당 노드까지의 실제 소요 경비값
		float hCost; // 경험값
		float GetFCost() const { return gCost + hCost; }
		Node* parent = nullptr;
	};

	// Node 비교 연산자 정의
	// std::priority_queue는 기본적으로 최대 힙을 사용하여 가장 큰 요소를 루트로 유지하지만, 
	// 커스터마이징된 비교 기준을 제공하여 최소 힙을 만들거나 다른 기준에 따라 요소의 우선순위를 결정할 수 있다.
	struct NodeCompare
	{
		bool operator()(const Node* lhs, const Node* rhs)
		{
			if (lhs)
				return lhs->GetFCost() > rhs->GetFCost();
		}
	};


	// [시작점, 목표점, 맵 벡터] XZ좌표의 A* 알고리즘 구현 
	std::vector<Vector2> Astar(const Vector2& _startPoint, const Vector2& _goalPoint, const std::vector<std::vector<int>> _map);

	// [map, 경로] 입력한 맵을 기반으로 움직이는 경로를 print 
	void printAstarMap(const std::vector<std::vector<int>>& _map, std::vector<Vector2> _path);
	// [경로] 단순히 움직이는 경로만 숫자로 확인 
	void PrintSimpleAstar(const std::vector<Vector2> _path);

private:
	// 현재 위치에서 움직일 수 있는 방향 설정
	std::vector<Vector2> directions;

	// 가장 기본적인 AStar
	std::vector<Vector2> BasicAstar(const Vector2& _startPoint, const Vector2& _goalPoint, const std::vector<Vector2>& obstacles, const Vector2& _mapStart, const Vector2& _mapEnd);
	
	// 맵을 Astar에서 사용하는 장애물 좌표로 변환시켜 사용하기 위한 함수
	std::vector<Vector2> ConvertMapToVector(const std::vector<std::vector<int>> _map);

	// 거리를 재는 함수
	float Distance(const Vector2& _pos1, const Vector2& _pos2);

	// 장애물 여부에 따라 움직일 수 있는지 확인하는 함수
	bool IsMovable(const Vector2& _pos, const std::vector<Vector2>& _obstaclePos, const Vector2& _mapStartPos, const Vector2& _mapEndPos);
};

// 해시 함수 : Vector2에 대한 키값을 사용하기 위한 정의
template <>
struct std::hash<Vector2>
{
	size_t operator()(const Vector2& v) const noexcept
	{
		return std::hash<float>()(v.x) ^ (std::hash<float>()(v.y) << 1);
	}
};
