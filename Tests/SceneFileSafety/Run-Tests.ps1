param(
    [ValidatePattern('^[a-zA-Z0-9-]+$')][string]$Label = 'latest',
    [string]$VsDevCmd = 'C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat'
)
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$taskRoot = Join-Path $root '.codex-local/harness/scene-file-safety-20261115'
$build = Join-Path $taskRoot 'tests'
$sandbox = Join-Path $taskRoot 'sandbox/Demo'
$include = Join-Path $root 'vcpkg_installed/x64-windows/include'
$msbuild = 'C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe'
New-Item -ItemType Directory -Path $build, $sandbox -Force | Out-Null
Push-Location $root
try {
    $ErrorActionPreference = 'Continue'
    & $msbuild Demo.sln /t:GameEngine /p:Configuration=Debug /p:Platform=x64 /p:VcpkgManifestInstall=false /m:1 /v:minimal /nologo 2>&1 |
        Tee-Object "$build/$Label-native.log"
    $code = $LASTEXITCODE
    "NATIVE_BUILD_EXIT=$code" | Tee-Object "$build/$Label-result.log"
    if ($code -ne 0) { exit 2 }
    $command = @"
@echo off
call "$VsDevCmd" -arch=x64 -host_arch=x64
if errorlevel 1 exit /b 2
cl.exe /nologo /std:c++20 /EHsc /source-charset:.949 /execution-charset:utf-8 /MDd /Od /W3 /D _DEBUG /D UNICODE /D _UNICODE /I "$root/ThirdParty/imgui" /I "$root/ThirdParty/imgui/backends" /I "$root/ThirdParty/ImGuizmo" /I "$include" /I "$include/directxmath" /I "$root/DearsGraphicsEngine" /I "$root/DearsTweening" /I "$root/GameEngine" "$PSScriptRoot/SceneFileSafetyTests.cpp" /Fe:"$build/SceneFileSafetyTests.exe" /Fo:"$build/" /link /LIBPATH:"$root/x64/Debug" /LIBPATH:"$root/vcpkg_installed/x64-windows/debug/lib" GameEngine.lib DearsGraphicsEngine.lib DearsTweening.lib assimp-vc143-mtd.lib DirectXTK.lib DirectXTex.lib d3d11.lib dxgi.lib d3dcompiler.lib user32.lib gdi32.lib ole32.lib windowscodecs.lib imm32.lib dwmapi.lib /SUBSYSTEM:CONSOLE
exit /b %errorlevel%
"@
    $command | Set-Content "$build/$Label-build.cmd" -Encoding ASCII
    & cmd.exe /d /c "$build/$Label-build.cmd" 2>&1 | Tee-Object "$build/$Label-build.log"
    $code = $LASTEXITCODE
    "BUILD_EXIT=$code" | Tee-Object "$build/$Label-result.log" -Append
    if ($code -ne 0) { exit 2 }
    $oldPath = $env:PATH
    $env:PATH = "$root/vcpkg_installed/x64-windows/debug/bin;$oldPath"
    Push-Location $sandbox
    try {
        & "$build/SceneFileSafetyTests.exe" 2>&1 | Tee-Object "$build/$Label-test.log"
        $code = $LASTEXITCODE
        "TEST_EXIT=$code" | Tee-Object "$build/$Label-result.log" -Append
        exit $code
    } finally { Pop-Location; $env:PATH = $oldPath }
} finally { Pop-Location }
