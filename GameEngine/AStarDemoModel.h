#pragma once

#include <algorithm>
#include <utility>
#include "AStar.h"

// Bounded, rectangular editor state. The existing pathfinder owns path semantics.
class AStarDemoModel
{
public:
    static constexpr int MinSize = 1;
    static constexpr int MaxSize = 20;

    AStarDemoModel() { Recalculate(); }

    int Width() const { return static_cast<int>(m_map.front().size()); }
    int Height() const { return static_cast<int>(m_map.size()); }
    Vector2 Start() const { return {0, 0}; }
    Vector2 Goal() const { return {float(Width() - 1), float(Height() - 1)}; }
    const std::vector<std::vector<int>>& Map() const { return m_map; }
    const std::vector<Vector2>& Path() const { return m_path; }

    static bool ValidSize(int width, int height)
    {
        return width >= MinSize && width <= MaxSize && height >= MinSize && height <= MaxSize;
    }

    bool IsProtected(int x, int y) const
    {
        return (x == 0 && y == 0) || (x == Width() - 1 && y == Height() - 1);
    }

    bool Resize(int width, int height)
    {
        if (!ValidSize(width, height)) return false;
        std::vector<std::vector<int>> resized(height, std::vector<int>(width, 0));
        for (int y = 0; y < (std::min)(height, Height()); ++y)
            std::copy_n(m_map[y].begin(), (std::min)(width, Width()), resized[y].begin());
        resized[0][0] = resized[height - 1][width - 1] = 0;
        m_map = std::move(resized);
        Recalculate();
        return true;
    }

    bool Toggle(int x, int y)
    {
        if (x < 0 || y < 0 || x >= Width() || y >= Height() || IsProtected(x, y)) return false;
        m_map[y][x] = m_map[y][x] == 0 ? 1 : 0;
        Recalculate();
        return true;
    }

    void Recalculate()
    {
        AStar astar;
        m_path = astar.Astar(Start(), Goal(), m_map);
    }

private:
    std::vector<std::vector<int>> m_map = {
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0},
    };
    std::vector<Vector2> m_path;
};
