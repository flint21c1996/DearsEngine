# AStar demo

## In the running engine

Open `Demo.sln` in Visual Studio, select **Debug / x64**, set **Demo** as the
startup project, then press **F5**. The scene automatically registers an
**A* Demo** ImGui panel near the upper left. It can also be toggled from
**Window > A* Demo**. An already-running older Demo must be closed and rebuilt
to show the newly added panel.

The panel starts with the same 5x5 example below: green `S`, red `G`, gray `#`,
and gold `*` cells from the existing AStar result. Text starts at **1.5x**;
**Text scale** adjusts it from **1.0x to 2.5x**, including Ctrl-click entry.

- Enter **Width** and **Height** (each **1..20**), then press **Apply**.
  Editing the numbers alone does not resize the map. Invalid sizes show a
  validation message and disable Apply.
- Resize preserves obstacles in the overlapping area and clears new cells.
  Start stays at the top left, goal moves to the bottom right, and both are
  protected from obstacles. A 1x1 map shows `S/G` and zero steps.
- Click any other cell to toggle an obstacle. Every committed edit immediately
  reruns the existing AStar algorithm and redraws its current path. When blocked,
  **No path found.** replaces the step count and old stars disappear.
- Scroll the grid vertically and horizontally for large maps or smaller panels.
  **Recalculate** reruns the current map. Edits are session-local; they are not
  saved to disk. The standalone console example below stays fixed at 5x5.

The panel is movable and does not require scene objects. Its state lives in
`GameEngine/AStarDemoModel.h` and its UI in `GameEngine/AStarDemoPanel.h`.
The existing eight-way pathfinding algorithm and diagonal/corner behavior are
unchanged; this editor makes no general optimality claim for obstacle maps.

Focused editor tests use real ImGui frames, mouse/keyboard events, draw data and
the actual production model/pathfinder, without a DX11 device:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Tests\AStarEditor\Run-Tests.ps1 -Label local
```

This requires the existing ImGui 1.90.7 source in the repository's vcpkg build
tree plus the installed compiler/math headers. It does not install packages.
Logs are under `.codex-local/harness/astar-editor/`. These checks do not replace
a rebuilt engine's DX11 visual inspection.

## Standalone console alternative

From the repository root, build and run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Examples\AStarConsole\Run-Demo.ps1
```

Actual example output:

```text
S=start G=goal #=obstacle *=path .=empty
S.#..
.*#..
.*#..
.*#..
..**G
```

The fixed 5x5 grid starts at (0,0), ends at (4,4), and has a wall at
x=2, y=0..3. The opening is at (2,4). `*` comes from the existing
`GameEngine/AStar.cpp` result; the route is not hardcoded. The engine uses
eight-way movement, including diagonals. Equal-cost route choices may differ.
`S` and `G` take precedence over path markers; `#` remains an obstacle and
`.` is an empty cell. No graphics device or window is created.

The runner compiles only the demo and real AStar source, using installed
VS2022 Professional C++ tools, Windows SDK and the existing repository-local
vcpkg DirectXTK/DirectXMath headers. It does not install dependencies or modify
the solution. Use `-VsDevCmd <path>` for another Visual Studio installation.
Compiler logs, output and exit codes are under
`.codex-local/harness/astar-demo/`. A build or no-path failure returns nonzero.

To repeat the existing 23 regression cases:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Tests\AStar\Run-Tests.ps1
```

This demonstration uses the current obstacle behavior without changing the
algorithm or asserting general obstacle-path optimality. Empty/ragged maps,
fractional/nonfinite inputs and the algorithm's previously recorded risks are
outside this demo.
