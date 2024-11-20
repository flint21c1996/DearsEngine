#include "Astar.h"

AStar::AStar()
{
	directions =
	{
		// sqrt(2) �߰� �켱
		{ 1, 1}, { 1,-1}, {-1, 1}, {-1,-1},
		// sqrt(1) �ļ���
		{ 1, 0}, {-1, 0}, { 0, 1}, { 0, -1},
	};
}

AStar::~AStar()
{

}

std::vector<Vector2> AStar::BasicAstar(const Vector2& _startPoint, const Vector2& _goalPoint, const std::vector<Vector2>& obstacles, const Vector2& _mapStart, const Vector2& _mapEnd)
{
	/// ��� �ڵ� -------------------------------------------------------------
	if (_startPoint.x < _mapStart.x || _startPoint.y < _mapStart.y ||
		_startPoint.x > _mapEnd.x || _startPoint.y > _mapEnd.y ||
		_goalPoint.x < _mapStart.x || _goalPoint.y < _mapStart.y ||
		_goalPoint.x > _mapEnd.x || _goalPoint.y > _mapEnd.y)
	{
		std::cout << "Astar() error - ���� ������ ��ǥ ������ ���� ũ�⸦ ����ϴ�." << std::endl;
		return {};
	}

	for (const auto& obs : obstacles)
	{
		if (obs == _startPoint || obs == _goalPoint)
		{
			std::cout << "Astar() error - ���� ������ ��ǥ ������ ��ֹ��� ���� ��ġ�� �����Ǿ����ϴ�." << std::endl;
			return {};
		}

		if (obs.x < _mapStart.x || obs.y < _mapStart.y || obs.x > _mapEnd.x || obs.y > _mapEnd.y)
		{
			std::cout << "Astar() error - ��ֹ� ��ġ�� ���� ũ�⸦ ����ϴ�." << std::endl;
			return {};
		}
	}

	/// ��� �ڵ� �� -------------------------------------------------------------

	std::priority_queue<Node*, std::vector<Node*>, NodeCompare> mOpenList;
	std::unordered_map<Vector2, Node*> mAllNodes;
	std::unordered_set<Vector2> closeList; // closedList�� ������ �� ���� �������� ����

	// ���� ù ��° ����� Distance(hCost)�� goal�� ���� �� �����̹Ƿ� ���� ū ���� ���� �ȴ�.
	Node* startNode = new Node();
	startNode->position = _startPoint;
	startNode->hCost = Distance(_startPoint, _goalPoint);
	mOpenList.push(startNode);
	mAllNodes[_startPoint] = startNode;

	// openList�� �� ������ �ݺ��Ѵ�.
	while (!mOpenList.empty())
	{
		Node* currentNode = mOpenList.top();
		mOpenList.pop();

		// ���� closedList���� �ش� ����� ��ġ�� �ִٸ� �� ��ġ�� ���� �ʴ´�.
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

			/// �������� ���ϸ� ����
			if (!IsMovable(neighborPos, obstacles, _mapStart, _mapEnd))
			{
				continue;
			}

			float newGCost = currentNode->gCost + Distance(currentNode->position, neighborPos);
			auto it = mAllNodes.find(neighborPos); // ���� ��ġ�� �ʿ��� ã�´�.
			if (it == mAllNodes.end()) // ��尡 ���ų� �� ����� gCost�� �� ������ ���ο� ��带 ������ openList�� �־��ش�.
			{
				Node* neighborNode = new Node;
				neighborNode->position = neighborPos;
				neighborNode->gCost = newGCost;
				// ������� ���� ���ܵ�
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
		std::cout << "----------- " << n << "��° �̵� -----------" << std::endl;
		for (int i = 0; i < _map.size(); ++i)
		{
			for (int j = 0; j < _map[i].size(); ++j)
			{
				if (_map[i][j] == 1)
				{
					std::cout << "��";
				}
				else if (i == pos.y && j == pos.x)
				{
					std::cout << "��";
				}
				else
				{
					std::cout << "��";
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
	// ���� ��ġ�� ���� ��ǥ ���� �־�� ��
	if (_pos.x < _mapStartPos.x || _pos.y < _mapStartPos.y ||
		_pos.x > _mapEndPos.x || _pos.y > _mapEndPos.y)
	{
		return false;
	}

	// �� ��ֹ��� ���� �˻�
	for (const auto& obs : _obstaclePos)
	{
		// ��ó ��ֹ��� �˻��Ѵ�.
		if (_pos.x == obs.x && _pos.y == obs.y && Distance(_pos, obs) <= sqrt(2))
		{
			return false;
		}
	}

	return true;
}
