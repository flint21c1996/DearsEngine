#include "AStar.h"
#include <algorithm>

int main()
{
    const std::vector<std::vector<int>> map = {
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0},
    };
    const Vector2 start{0, 0};
    const Vector2 goal{4, 4};
    AStar astar;
    const auto path = astar.Astar(start, goal, map);
    if (path.empty()) {
        std::cerr << "No path found.\n";
        return 1;
    }

    std::cout << "S=start G=goal #=obstacle *=path .=empty\n";
    for (int y = 0; y < 5; ++y) {
        for (int x = 0; x < 5; ++x) {
            char cell = map[y][x] == 1 ? '#' : '.';
            if (cell == '.' && std::find(path.begin(), path.end(), Vector2(float(x), float(y))) != path.end())
                cell = '*';
            if (Vector2(float(x), float(y)) == start) cell = 'S';
            if (Vector2(float(x), float(y)) == goal) cell = 'G';
            std::cout << cell;
        }
        std::cout << '\n';
    }
}
