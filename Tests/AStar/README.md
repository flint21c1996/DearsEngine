# Headless AStar tests

Run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Tests\AStar\Run-Tests.ps1
```

Requires installed Visual Studio 2022 Professional C++ tools/Windows SDK and
existing `vcpkg_installed/x64-windows/include` DirectXTK/DirectXMath headers.
Use `-VsDevCmd <path>` for another Visual Studio installation.

Compiles the actual `GameEngine/AStar.cpp` with the test translation unit into
an x64 C++20 console executable. Real SimpleMath vector operations are inline;
no graphics device/window or engine library build is needed. CompileSupport.h
supplies only standard includes normally available transitively to the engine.
Source charset 949 matches existing Korean product files; execution strings
use UTF-8. No dependencies are installed or modified.

Logs and binary: `.codex-local/harness/astar-tests/`. `build.cmd` records the
expanded compiler command; `build.log`, `test.log`, `result.log` record evidence.
Build failure stops before tests and is not behavior RED. With BUILD_EXIT=0,
TEST_EXIT=0 means pass and TEST_EXIT=1 means assertion failure.

Checks endpoint inclusion, finite integer cells, bounds, nonzero eight-way
steps, and independent octile minimum cost (1e-5 tolerance). Includes rectangles,
narrow grids, reverse routes, diagonals, singleton, and all 900 endpoint pairs
on grids with width/height 1..4. Invalid cases cover each endpoint at width or
height, exterior singleton, negative coordinates and goals beyond dimensions.
No exact route tie-break required. Obstacles, empty/ragged maps, fractional or
nonfinite inputs, rendering, performance and memory safety are deferred.
