param([string]$Stage)
$ErrorActionPreference='Stop'
$sourcePath=Join-Path $PSScriptRoot '../src/OledUI/OledUI.cpp'
$script:source=[IO.File]::ReadAllText($sourcePath)
function Move-Functions([string]$target,[string[]]$names,[string]$includes) {
    $content=$includes+"`n"
    foreach($name in $names) {
        $pattern='(?m)^(?:static )?(?:void|bool|AnimationCurve) '+[regex]::Escape($name)+'\([^\{;]*\{.*?^\}'
        $match=[regex]::Match($script:source,$pattern,[Text.RegularExpressions.RegexOptions]::Singleline)
        if(!$match.Success) { throw "Missing function $name" }
        $content += $match.Value+"`n`n"
        $script:source=$script:source.Remove($match.Index,$match.Length)
    }
    [IO.File]::WriteAllText((Join-Path $PSScriptRoot "../src/OledUI/$target"),$content,[Text.UTF8Encoding]::new($false))
}
if($Stage -eq 'input') {
    Move-Functions 'UiInput.cpp' @('OledUI::Key_Scan') "#include `"OledUI.h`"`n#include `"Input/InputQueue.h`"`n#include `"Input/InputAdapters.h`"`n#include `"UiInputAdapter.h`"`n#include `"Encoder/HAL_Encoder.h`"`n#ifndef ENABLE_SERIAL_INPUT_DEBUG`n#define ENABLE_SERIAL_INPUT_DEBUG 0`n#endif"
}
[IO.File]::WriteAllText($sourcePath,$script:source,[Text.UTF8Encoding]::new($false))
