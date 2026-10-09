$ErrorActionPreference='Stop'
$projectRoot=Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$fixtureRoot=Join-Path $projectRoot '.pio/migration-safety'
New-Item -ItemType Directory -Path (Join-Path $fixtureRoot 'tools') -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'fixture/src') -Destination $fixtureRoot -Recurse -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'tools/split-oled.ps1') -Destination (Join-Path $fixtureRoot 'tools/split-oled.ps1') -Force
$contextPath=Join-Path $fixtureRoot 'src/OledUI/UiContext.h'
$expected=[IO.File]::ReadAllText($contextPath)
$rejected=$false
try { & (Join-Path $fixtureRoot 'tools/split-oled.ps1') pages } catch { $rejected=$true }
if(!$rejected){throw 'Repeated migration should be rejected'}
if([IO.File]::ReadAllText($contextPath) -ne $expected){throw 'Rejected migration damaged UiContext.h'}
Write-Output 'PASS migration safety'
