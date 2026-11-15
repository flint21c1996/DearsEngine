param(
    [ValidatePattern('^[a-zA-Z0-9-]+$')][string]$Label = 'latest',
    [string]$VsDevCmd = 'C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat'
)
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$build = Join-Path $root '.codex-local/harness/shadow-toggle-20261115/tests'
$include = Join-Path $root 'vcpkg_installed/x64-windows/include'
New-Item -ItemType Directory -Path $build -Force | Out-Null
$command = @"
@echo off
call "$VsDevCmd" -arch=x64 -host_arch=x64
if errorlevel 1 exit /b 2
cl.exe /nologo /std:c++20 /EHsc /source-charset:.949 /execution-charset:utf-8 /MD /Od /W3 /I "$include" /I "$include/directxmath" /I "$root/DearsGraphicsEngine" "$PSScriptRoot/ShadowToggleTests.cpp" "$root/DearsGraphicsEngine/RenderPipeline.cpp" /Fe:"$build/ShadowToggleTests.exe" /Fo:"$build/" /link /LIBPATH:"$root/vcpkg_installed/x64-windows/lib" DirectXTK.lib d3d11.lib d3dcompiler.lib /SUBSYSTEM:CONSOLE
exit /b %errorlevel%
"@
$command | Set-Content "$build/$Label-build.cmd" -Encoding ASCII
Push-Location $root
try {
    $ErrorActionPreference = 'Continue'
    & cmd.exe /d /c "$build/$Label-build.cmd" 2>&1 | Tee-Object "$build/$Label-build.log"
    $code = $LASTEXITCODE
    "BUILD_EXIT=$code" | Tee-Object "$build/$Label-result.log"
    if ($code -ne 0) { exit 2 }
    & "$build/ShadowToggleTests.exe" 2>&1 | Tee-Object "$build/$Label-test.log"
    $code = $LASTEXITCODE
    "TEST_EXIT=$code" | Tee-Object "$build/$Label-result.log" -Append
    exit $code
} finally { Pop-Location }
