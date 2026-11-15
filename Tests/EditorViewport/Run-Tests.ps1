param(
    [ValidatePattern('^[a-zA-Z0-9-]+$')][string]$Label = 'latest',
    [string]$VsDevCmd = 'C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat',
    [switch]$Resources
)
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$build = Join-Path $root '.codex-local/harness/editor-layout-20261114/tests'
$include = Join-Path $root 'vcpkg_installed/x64-windows/include'
$imgui = Join-Path $root 'vcpkg_installed/vcpkg/blds/imgui/src/v1.90.7-cda29176bb.clean'
New-Item -ItemType Directory -Path $build -Force | Out-Null
$sources = '"' + "$PSScriptRoot/ViewportTests.cpp" + '" "' + "$root/DearsGraphicsEngine/Camera.cpp" +
    '" "' + "$imgui/imgui.cpp" + '" "' + "$imgui/imgui_draw.cpp" + '" "' + "$imgui/imgui_widgets.cpp" + '" "' + "$imgui/imgui_tables.cpp" + '"'
if ($Resources) {
    $sources = '"' + "$PSScriptRoot/ResourceTests.cpp" + '" "' + "$root/DearsGraphicsEngine/RenderTargetPool.cpp" +
        '" "' + "$root/DearsGraphicsEngine/GBuffer.cpp" + '"'
}
$executable = if ($Resources) { 'ResourceTests.exe' } else { 'ViewportTests.exe' }
$command = @"
@echo off
call "$VsDevCmd" -arch=x64 -host_arch=x64
if errorlevel 1 exit /b 2
cl.exe /nologo /std:c++20 /EHsc /source-charset:.949 /execution-charset:utf-8 /MD /Od /W3 /D UNICODE /D _UNICODE /I "$include" /I "$include/directxmath" /I "$root/DearsGraphicsEngine" $sources /Fe:"$build/$executable" /Fo:"$build/" /link /LIBPATH:"$root/vcpkg_installed/x64-windows/lib" DirectXTK.lib user32.lib d3d11.lib /SUBSYSTEM:CONSOLE
exit /b %errorlevel%
"@
$command | Set-Content "$build/$Label-build.cmd" -Encoding ASCII
Push-Location $build
try {
    $ErrorActionPreference = 'Continue'
    & cmd.exe /d /c "$build/$Label-build.cmd" 2>&1 | Tee-Object "$build/$Label-build.log"
    $code = $LASTEXITCODE
    "BUILD_EXIT=$code" | Tee-Object "$build/$Label-result.log"
    if ($code -ne 0) { exit 2 }
    & "$build/$executable" 2>&1 | Tee-Object "$build/$Label-test.log"
    $code = $LASTEXITCODE
    "TEST_EXIT=$code" | Tee-Object "$build/$Label-result.log" -Append
    exit $code
} finally { Pop-Location }
