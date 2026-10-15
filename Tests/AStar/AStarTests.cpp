#include "CompileSupport.h"
#include "AStar.h"
#include <cmath>
#include <stdexcept>
#include <string>

namespace {
int passed = 0;
int failed = 0;

void Require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

template<class Test> void Run(const std::string& name, Test test) {
    try {
        test();
        ++passed;
        std::cout << "PASS " << name << '\n';
    } catch (const std::exception& error) {
        ++failed;
        std::cout << "FAIL " << name << ": " << error.what() << '\n';
    }
}

void ValidPath(int width, int height, Vector2 start, Vector2 goal) {
    const std::vector<std::vector<int>> map(height, std::vector<int>(width, 0));
    AStar astar;
    const auto path = astar.Astar(start, goal, map);
    Require(!path.empty(), "expected a path");
    Require(path.front() == start && path.back() == goal, "endpoint mismatch");
    if (start == goal) Require(path.size() == 1, "expected singleton path");
    double cost = 0;
    for (size_t i = 0; i < path.size(); ++i) {
        const auto cell = path[i];
        Require(std::isfinite(cell.x) && std::isfinite(cell.y), "nonfinite cell");
        Require(std::floor(cell.x) == cell.x && std::floor(cell.y) == cell.y, "noninteger cell");
        Require(cell.x >= 0 && cell.x < width && cell.y >= 0 && cell.y < height,
                "path leaves grid");
        if (i == 0) continue;
        const double dx = std::abs(cell.x - path[i - 1].x);
        const double dy = std::abs(cell.y - path[i - 1].y);
        Require(dx <= 1 && dy <= 1 && dx + dy > 0, "invalid eight-way step");
        cost += std::hypot(dx, dy);
    }
    const double dx = std::abs(goal.x - start.x);
    const double dy = std::abs(goal.y - start.y);
    const double expected = std::min(dx, dy) * std::sqrt(2.0) + std::abs(dx - dy);
    Require(std::abs(cost - expected) <= 1e-5, "cost differs from octile minimum");
}

void RejectEndpoint(int width, int height, Vector2 start, Vector2 goal) {
    AStar astar;
    const auto path = astar.Astar(start, goal,
        std::vector<std::vector<int>>(height, std::vector<int>(width, 0)));
    Require(path.empty(), "expected no path; returned " + std::to_string(path.size()) + " cells");
}
}

int main() {
    Run("singleton 1x1", [] { ValidPath(1, 1, {0, 0}, {0, 0}); });
    Run("singleton interior", [] { ValidPath(7, 4, {3, 2}, {3, 2}); });
    Run("wide rectangular", [] { ValidPath(7, 4, {0, 0}, {6, 3}); });
    Run("wide rectangular reverse", [] { ValidPath(7, 4, {6, 3}, {0, 0}); });
    Run("tall rectangular", [] { ValidPath(3, 8, {0, 7}, {2, 0}); });
    Run("tall rectangular reverse", [] { ValidPath(3, 8, {2, 0}, {0, 7}); });
    Run("horizontal narrow", [] { ValidPath(8, 1, {0, 0}, {7, 0}); });
    Run("horizontal narrow reverse", [] { ValidPath(8, 1, {7, 0}, {0, 0}); });
    Run("vertical narrow", [] { ValidPath(1, 8, {0, 0}, {0, 7}); });
    Run("vertical narrow reverse", [] { ValidPath(1, 8, {0, 7}, {0, 0}); });
    Run("diagonal", [] { ValidPath(5, 5, {0, 0}, {4, 4}); });
    Run("all valid endpoint pairs on small grids", [] {
        for (int height = 1; height <= 4; ++height)
            for (int width = 1; width <= 4; ++width)
                for (int sy = 0; sy < height; ++sy)
                    for (int sx = 0; sx < width; ++sx)
                        for (int gy = 0; gy < height; ++gy)
                            for (int gx = 0; gx < width; ++gx)
                                ValidPath(width, height, {float(sx), float(sy)}, {float(gx), float(gy)});
    });
    Run("reject start x=width", [] { RejectEndpoint(3, 2, {3, 0}, {1, 1}); });
    Run("reject goal x=width", [] { RejectEndpoint(3, 2, {1, 1}, {3, 0}); });
    Run("reject start y=height", [] { RejectEndpoint(3, 2, {0, 2}, {1, 1}); });
    Run("reject goal y=height", [] { RejectEndpoint(3, 2, {1, 1}, {0, 2}); });
    Run("reject exterior singleton", [] { RejectEndpoint(3, 2, {3, 2}, {3, 2}); });
    Run("reject negative start x", [] { RejectEndpoint(3, 2, {-1, 0}, {1, 1}); });
    Run("reject negative start y", [] { RejectEndpoint(3, 2, {0, -1}, {1, 1}); });
    Run("reject negative goal x", [] { RejectEndpoint(3, 2, {1, 1}, {-1, 0}); });
    Run("reject negative goal y", [] { RejectEndpoint(3, 2, {1, 1}, {0, -1}); });
    Run("reject goal beyond width", [] { RejectEndpoint(3, 2, {0, 0}, {4, 0}); });
    Run("reject goal beyond height", [] { RejectEndpoint(3, 2, {0, 0}, {0, 3}); });
    std::cout << "RESULT " << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
