param(
    [ValidatePattern('^[a-zA-Z0-9-]+$')][string]$Label = 'latest',
    [string]$VsDevCmd = 'C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat'
)
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$build = Join-Path $root '.codex-local/harness/astar-editor'
$include = Join-Path $root 'vcpkg_installed/x64-windows/include'
$imgui = Join-Path $root 'vcpkg_installed/vcpkg/blds/imgui/src/v1.90.7-cda29176bb.clean'
foreach ($required in @($VsDevCmd, "$imgui/imgui.cpp", "$include/directxtk/SimpleMath.h")) {
    if (-not (Test-Path -LiteralPath $required)) { throw "Required test input missing: $required" }
}
New-Item -ItemType Directory -Path $build -Force | Out-Null
$command = @"
@echo off
call "$VsDevCmd" -arch=x64 -host_arch=x64
if errorlevel 1 exit /b 2
cl.exe /nologo /std:c++20 /EHsc /source-charset:.949 /execution-charset:utf-8 /MD /Od /W3 /D IMGUI_ENABLE_TEST_ENGINE /I "$imgui" /I "$include" /I "$include/directxmath" /I "$root/GameEngine" /I "$root/DearsGraphicsEngine" "$PSScriptRoot/PanelTests.cpp" "$PSScriptRoot/EngineIncludeProbe.cpp" "$root/GameEngine/AStar.cpp" "$imgui/imgui.cpp" "$imgui/imgui_draw.cpp" "$imgui/imgui_widgets.cpp" "$imgui/imgui_tables.cpp" /Fe:"$build/PanelTests.exe" /Fo:"$build/" /link /SUBSYSTEM:CONSOLE user32.lib
exit /b %errorlevel%
"@
$command | Set-Content "$build/$Label-build.cmd" -Encoding ASCII
Push-Location $build
try {
    $ErrorActionPreference = 'Continue'
    & cmd.exe /d /c "$build/$Label-build.cmd" 2>&1 | Tee-Object "$build/$Label-build.log"
    $buildExit = $LASTEXITCODE
    "BUILD_EXIT=$buildExit" | Tee-Object "$build/$Label-result.log"
    if ($buildExit -ne 0) { exit 2 }
    & "$build/PanelTests.exe" 2>&1 | Tee-Object "$build/$Label-test.log"
    $testExit = $LASTEXITCODE
    "TEST_EXIT=$testExit" | Tee-Object "$build/$Label-result.log" -Append
    exit $testExit
} finally { Pop-Location }
