param([string]$Only = '')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$compilerCommand = Get-Command g++, clang++ -ErrorAction SilentlyContinue | Select-Object -First 1
if (!$compilerCommand) { throw 'No installed host C++ compiler found; do not install automatically.' }
$buildRoot = Join-Path $projectRoot '.pio/host-tests'
New-Item -ItemType Directory -Path $buildRoot -Force | Out-Null
$testFiles = Get-ChildItem -LiteralPath (Join-Path $projectRoot 'test') -Recurse -Filter 'test_*.cpp'
$passed = 0
foreach ($testFile in $testFiles) {
    $testName = $testFile.Directory.Name
    if ($Only -and $testName -ne $Only) { continue }
    $executable = Join-Path $buildRoot "$testName.exe"
    $compileArguments = @('-std=c++11', '-Wall', '-Wextra', '-Werror', '-I', (Join-Path $projectRoot 'src'), $testFile.FullName)
    if ($testName -eq 'gamepad_input') {
        $compileArguments += Join-Path $projectRoot 'src/Input/GamepadInput.cpp'
    }
    if ($testName -eq 'input_queue') {
        $compileArguments += @('-I', (Join-Path $projectRoot 'test/support'), (Join-Path $projectRoot 'src/Input/InputQueue.cpp'))
    }
    $compileArguments += @('-o', $executable)
    & $compilerCommand.Source @compileArguments
    if ($LASTEXITCODE -ne 0) { throw "Compile failed: $testName" }
    & $executable
    if ($LASTEXITCODE -ne 0) { throw "Test failed: $testName" }
    Write-Output "PASS $testName"
    $passed++
}
if ($passed -eq 0) { throw 'No tests selected.' }
Write-Output "PASS $passed test suites"
