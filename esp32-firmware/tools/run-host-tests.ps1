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
    if ($testName -eq 'connection_events') {
        $compileArguments += @('-I', (Join-Path $projectRoot 'test/support'), (Join-Path $projectRoot 'src/Bluetooth/ConnectionEvents.cpp'))
    }
    if ($testName -eq 'bluetooth_service') {
        $compileArguments += Join-Path $projectRoot 'src/Bluetooth/BluetoothService.cpp'
    }
    $compileArguments += @('-o', $executable)
    if ($testName -eq 'ui_input') {
        $compileArguments = @('-I', (Join-Path $projectRoot 'test/support/oled')) + $compileArguments + (Join-Path $projectRoot 'src/OledUI/UiInput.cpp')
    }
    if ($testName -eq 'connection_toast') {
        $compileArguments = @('-I', (Join-Path $projectRoot 'test/support/oled')) + $compileArguments + (Join-Path $projectRoot 'src/OledUI/ConnectionToast.cpp')
    }
    if ($testName -eq 'ui_renderer') {
        $compileArguments = @('-I', (Join-Path $projectRoot 'test/support/oled')) + $compileArguments + @((Join-Path $projectRoot 'src/OledUI/UiRenderer.cpp'), (Join-Path $projectRoot 'src/OledUI/OledMenus.cpp'))
    }
    if ($testName -eq 'ui_dispatch') {
        $compileArguments = @('-I', (Join-Path $projectRoot 'test/support/oled')) + $compileArguments + @((Join-Path $projectRoot 'src/OledUI/OledUI.cpp'), (Join-Path $projectRoot 'src/OledUI/UiPages/UiPages.cpp'), (Join-Path $projectRoot 'src/OledUI/OledMenus.cpp'))
    }
    if ($testName -eq 'app_lifecycle') {
        $compileArguments = @('-I', (Join-Path $projectRoot 'test/support/app')) + $compileArguments + (Join-Path $projectRoot 'src/App/App.cpp')
    }
    if ($testName -eq 'app_tasks') {
        $compileArguments = @('-I', (Join-Path $projectRoot 'test/support/app')) + $compileArguments + (Join-Path $projectRoot 'src/App/AppTasks.cpp')
    }
    & $compilerCommand.Source @compileArguments
    if ($LASTEXITCODE -ne 0) { throw "Compile failed: $testName" }
    & $executable
    if ($LASTEXITCODE -ne 0) { throw "Test failed: $testName" }
    if ($testName -eq 'app_lifecycle') {
        foreach ($failure in @(2, 3, 4)) {
            & $executable $failure
            if ($LASTEXITCODE -ne 0) { throw "Test failed: $testName failure=$failure" }
        }
    }
    Write-Output "PASS $testName"
    if ($testName -eq 'ui_input') {
        & $compilerCommand.Source @compileArguments '-D' 'ENABLE_SERIAL_INPUT_DEBUG=1'
        if ($LASTEXITCODE -ne 0) { throw 'Compile failed: ui_input serial enabled' }
        & $executable
        if ($LASTEXITCODE -ne 0) { throw 'Test failed: ui_input serial enabled' }
    }
    if ($testName -eq 'app_tasks') {
        foreach ($failure in @(1, 2, 3, 4)) {
            & $executable $failure
            if ($LASTEXITCODE -ne 0) { throw "Test failed: $testName failure=$failure" }
        }
    }
    $passed++
}
if ($passed -eq 0) { throw 'No tests selected.' }
Write-Output "PASS $passed test suites"
