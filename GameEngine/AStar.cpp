#include "Astar.h"

AStar::AStar()
{
	directions =
	{
		// sqrt(2) 중간 우선
		{ 1, 1}, { 1,-1}, {-1, 1}, {-1,-1},
		// sqrt(1) 후순위
		{ 1, 0}, {-1, 0}, { 0, 1}, { 0, -1},
	};
}

AStar::~AStar()
{

}

std::vector<Vector2> AStar::BasicAstar(const Vector2& _startPoint, const Vector2& _goalPoint, const std::vector<Vector2>& obstacles, const Vector2& _mapStart, const Vector2& _mapEnd)
{
	/// 방어 코드 -------------------------------------------------------------
	if (_startPoint.x < _mapStart.x || _startPoint.y < _mapStart.y ||
		_startPoint.x > _mapEnd.x || _startPoint.y > _mapEnd.y ||
		_goalPoint.x < _mapStart.x || _goalPoint.y < _mapStart.y ||
		_goalPoint.x > _mapEnd.x || _goalPoint.y > _mapEnd.y)
	{
		std::cout << "Astar() error - 시작 지점과 목표 지점이 맵의 크기를 벗어납니다." << std::endl;
		return {};
	}

	for (const auto& obs : obstacles)
	{
		if (obs == _startPoint || obs == _goalPoint)
		{
			std::cout << "Astar() error - 시작 지점과 목표 지점이 장애물과 같은 위치로 설정되었습니다." << std::endl;
			return {};
		}

		if (obs.x < _mapStart.x || obs.y < _mapStart.y || obs.x > _mapEnd.x || obs.y > _mapEnd.y)
		{
			std::cout << "Astar() error - 장애물 위치가 맵의 크기를 벗어납니다." << std::endl;
			return {};
		}
	}

	/// 방어 코드 끝 -------------------------------------------------------------

	std::priority_queue<Node*, std::vector<Node*>, NodeCompare> mOpenList;
	std::unordered_map<Vector2, Node*> mAllNodes;
	std::unordered_set<Vector2> closeList; // closedList를 지나갈 수 없는 공간으로 설정

	// 가장 첫 번째 노드의 Distance(hCost)는 goal과 가장 먼 지점이므로 가장 큰 값을 갖게 된다.
	Node* startNode = new Node();
	startNode->position = _startPoint;
	startNode->hCost = Distance(_startPoint, _goalPoint);
	mOpenList.push(startNode);
	mAllNodes[_startPoint] = startNode;

	// openList가 빌 떄까지 반복한다.
	while (!mOpenList.empty())
	{
		Node* currentNode = mOpenList.top();
		mOpenList.pop();

		// 만약 closedList에서 해당 노드의 위치가 있다면 그 위치로 가지 않는다.
		if (closeList.find(currentNode->position) != closeList.end())
		{
			closeList.insert(currentNode->position);
			continue;
		}

		if (currentNode->position == _goalPoint)
		{
			std::vector<Vector2> path;
			while (currentNode)
			{
				path.push_back(currentNode->position);
				currentNode = currentNode->parent;
			}
			std::reverse(path.begin(), path.end());

			for (auto& pair : mAllNodes)
			{
				delete pair.second;
			}

			return path;
		}

		for (const auto& dir : directions)
		{
			Vector2 neighborPos = currentNode->position + dir;

			/// 움직이지 못하면 멈춰
			if (!IsMovable(neighborPos, obstacles, _mapStart, _mapEnd))
			{
				continue;
			}

			float newGCost = currentNode->gCost + Distance(currentNode->position, neighborPos);
			auto it = mAllNodes.find(neighborPos); // 현재 위치를 맵에서 찾는다.
			if (it == mAllNodes.end()) // 노드가 없거나 새 노드의 gCost가 더 작으면 새로운 노드를 생성해 openList에 넣어준다.
			{
				Node* neighborNode = new Node;
				neighborNode->position = neighborPos;
				neighborNode->gCost = newGCost;
				// 디버깅을 위해 남겨둠
				//std::cout << neighborNode->position.x << " " << neighborNode->position.y << " " << neighborNode->position.z << ", " << neighborNode->gCost << std::endl;
				neighborNode->hCost = Distance(neighborPos, _goalPoint);
				neighborNode->parent = currentNode;
				mOpenList.push(neighborNode);
				mAllNodes[neighborPos] = neighborNode;
			}
			else if (newGCost < it->second->gCost)
			{
				Node* neighborNode = it->second;
				neighborNode->gCost = newGCost;
				neighborNode->parent = currentNode;
				mOpenList.push(neighborNode);
			}
		}
	}

	for (auto& pair : mAllNodes)
	{
		delete pair.second;
	}

	return {};
}

std::vector<Vector2> AStar::Astar(const Vector2& _startPoint, const Vector2& _goalPoint, const std::vector<std::vector<int>> _map)
{
	std::vector<Vector2> obstacles = ConvertMapToVector(_map);
	Vector2 mapStart = { 0.0f, 0.0f };
	Vector2 mapEnd = { static_cast<float>(_map[0].size()), static_cast<float>(_map.size()) };

	return BasicAstar(_startPoint, _goalPoint, obstacles, mapStart, mapEnd);
}

std::vector<Vector2> AStar::ConvertMapToVector(const std::vector<std::vector<int>> _map)
{
	std::vector<Vector2> tempObstacles;
	for (size_t i = 0; i < _map.size(); i++)
	{
		for (size_t j = 0; j < _map[i].size(); j++)
		{
			if (_map[i][j] == 1)
			{
				tempObstacles.push_back({ static_cast<float>(j), static_cast<float>(i) });
			}
		}
	}
	return tempObstacles;
}

void AStar::printAstarMap(const std::vector<std::vector<int>>& _map, std::vector<Vector2> _path)
{
	int n = 0;
	for (const auto& pos : _path)
	{
		std::cout << "----------- " << n << "번째 이동 -----------" << std::endl;
		for (int i = 0; i < _map.size(); ++i)
		{
			for (int j = 0; j < _map[i].size(); ++j)
			{
				if (_map[i][j] == 1)
				{
					std::cout << "■";
				}
				else if (i == pos.y && j == pos.x)
				{
					std::cout << "♪";
				}
				else
				{
					std::cout << "□";
				}
			}
			std::cout << std::endl;
		}
		n++;
	}
}

void AStar::PrintSimpleAstar(const std::vector<Vector2> _path)
{
	for (const auto& pos : _path)
	{
		std::cout << "pos[x, z] : (" << pos.x << ", " << pos.y << ")\n";
	}
}

float AStar::Distance(const Vector2& _pos1, const Vector2& _pos2)
{
	return (_pos1 - _pos2).Length();
}

bool AStar::IsMovable(const Vector2& _pos, const std::vector<Vector2>& _obstaclePos, const Vector2& _mapStartPos, const Vector2& _mapEndPos)
{
	// 현재 위치가 맵의 좌표 내에 있어야 함
	if (_pos.x < _mapStartPos.x || _pos.y < _mapStartPos.y ||
		_pos.x > _mapEndPos.x || _pos.y > _mapEndPos.y)
	{
		return false;
	}

	// 각 장애물에 대해 검사
	for (const auto& obs : _obstaclePos)
	{
		// 근처 장애물만 검사한다.
		if (_pos.x == obs.x && _pos.y == obs.y && Distance(_pos, obs) <= sqrt(2))
		{
			return false;
		}
	}

	return true;
}
