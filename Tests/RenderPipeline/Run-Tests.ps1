param(
    [string]$VsDevCmd = 'C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat'
)
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$build = Join-Path $root '.codex-local/harness/render-pipeline-tests'
$include = Join-Path $root 'vcpkg_installed/x64-windows/include'
foreach ($required in @($VsDevCmd, "$include/directxtk/SimpleMath.h", "$include/directxmath/DirectXMath.h")) {
    if (-not (Test-Path -LiteralPath $required)) { throw "Required build input missing: $required" }
}
New-Item -ItemType Directory -Path $build -Force | Out-Null
$command = @"
@echo off
call "$VsDevCmd" -arch=x64 -host_arch=x64
if errorlevel 1 exit /b 2
cl.exe /nologo /std:c++20 /EHsc /source-charset:.949 /execution-charset:utf-8 /MD /Od /W3 /D NOMINMAX /I "$include" /I "$include/directxmath" /I "$root/DearsGraphicsEngine" /I "$root/GameEngine" "$PSScriptRoot/RenderPipelineTests.cpp" "$root/DearsGraphicsEngine/RenderPipeline.cpp" /Fe:"$build/RenderPipelineTests.exe" /Fo:"$build/" /link /SUBSYSTEM:CONSOLE
exit /b %errorlevel%
"@
$command | Set-Content -LiteralPath "$build/build.cmd" -Encoding ASCII
Push-Location $build
try {
    $ErrorActionPreference = 'Continue'
    & cmd.exe /d /c "$build/build.cmd" 2>&1 | Tee-Object -FilePath "$build/build.log"
    $buildExit = $LASTEXITCODE
    "BUILD_EXIT=$buildExit" | Tee-Object -FilePath "$build/result.log"
    if ($buildExit -ne 0) { exit 2 }
    & "$build/RenderPipelineTests.exe" 2>&1 | Tee-Object -FilePath "$build/test.log"
    $testExit = $LASTEXITCODE
    "TEST_EXIT=$testExit" | Tee-Object -FilePath "$build/result.log" -Append
    exit $testExit
} finally {
    Pop-Location
}
