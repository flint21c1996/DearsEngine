param(
    [string]$VsDevCmd = 'C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat'
)
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$build = Join-Path $root '.codex-local/harness/astar-demo'
$include = Join-Path $root 'vcpkg_installed/x64-windows/include'
foreach ($required in @($VsDevCmd, "$include/directxtk/SimpleMath.h", "$include/directxmath/DirectXMath.h")) {
    if (-not (Test-Path -LiteralPath $required)) { throw "Required build input missing: $required" }
}
New-Item -ItemType Directory -Path $build -Force | Out-Null
$command = @"
@echo off
call "$VsDevCmd" -arch=x64 -host_arch=x64
if errorlevel 1 exit /b 2
cl.exe /nologo /std:c++20 /EHsc /source-charset:.949 /execution-charset:utf-8 /MD /Od /W3 /D NOMINMAX /I "$include" /I "$include/directxmath" /I "$root/GameEngine" "$PSScriptRoot/AStarDemo.cpp" "$root/GameEngine/AStar.cpp" /Fe:"$build/AStarDemo.exe" /Fo:"$build/" /link /SUBSYSTEM:CONSOLE
exit /b %errorlevel%
"@
$command | Set-Content -LiteralPath "$build/build.cmd" -Encoding ASCII
Push-Location $build
try {
    # Redirect native stderr in cmd so PowerShell 5.1 still records the exit code.
    & cmd.exe /d /c "`"$build/build.cmd`" > `"$build/build.log`" 2>&1"
    $buildExit = $LASTEXITCODE
    "BUILD_EXIT=$buildExit" | Set-Content "$build/result.log"
    if ($buildExit -ne 0) {
        Get-Content "$build/build.log"
        exit $buildExit
    }
    & cmd.exe /d /c "`"$build/AStarDemo.exe`" > `"$build/demo.log`" 2>&1"
    $demoExit = $LASTEXITCODE
    Get-Content "$build/demo.log"
    "DEMO_EXIT=$demoExit" | Add-Content "$build/result.log"
    exit $demoExit
} finally {
    Pop-Location
}
