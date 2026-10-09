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
if($Stage -eq 'renderer') {
    Move-Functions 'UiRenderer.cpp' @('OledUI::Animation','OledUI::SetAnimationCurve','OledUI::GetAnimationCurve','OledUI::Fade','OledUI::Tile_Show','List_Draw_Value','List_Draw_CheckBox_Frame','List_Draw_CheckBox_Dot','List_Draw_Krf','List_Draw_Kpf','List_Draw_Text_And_CheckBox','OledUI::List_Show','OledUI::Volt_Show','OledUI::Window_Show') '#include "OledUI.h"'
}
if($Stage -eq 'pages') {
    $root=Join-Path $PSScriptRoot '../src/OledUI'
    $header=[IO.File]::ReadAllText((Join-Path $root 'OledUI.h'))
    if(!$header.Contains('#ifndef __OLED_UI_H') -or !$script:source.Contains('UI_Context ui;')) {
        throw 'Page migration requires the original unsplit header and context definitions; no files changed.'
    }
    $header=$header.Replace('__OLED_UI_H','OLED_UI_CONTEXT_H')
    [IO.File]::WriteAllText((Join-Path $root 'UiContext.h'),$header,[Text.UTF8Encoding]::new($false))
    [IO.File]::WriteAllText((Join-Path $root 'OledUI.h'),"#pragma once`nnamespace OledUI { void Init(); void Task(); }`n",[Text.UTF8Encoding]::new($false))
    $contexts=[regex]::Match($script:source,'(?m)^UI_Context ui;.*?^volatile Button_Context btn;',[Text.RegularExpressions.RegexOptions]::Singleline)
    if(!$contexts.Success){throw 'Missing contexts'}
    [IO.File]::WriteAllText((Join-Path $root 'UiContext.cpp'),"#include `"UiContext.h`"`n"+$contexts.Value+"`n",[Text.UTF8Encoding]::new($false))
    $script:source=$script:source.Remove($contexts.Index,$contexts.Length)
    $light=[regex]::Match($script:source,'(?m)^uint8_t light_brightness.*?^\};',[Text.RegularExpressions.RegexOptions]::Singleline)
    if(!$light.Success){throw 'Missing light state'}
    $lightState=$light.Value
    $script:source=$script:source.Remove($light.Index,$light.Length)
    $script:source=[regex]::Replace($script:source,'(?m)^static void Light_\w+_Param_Init\(\);\r?\n','')
    $script:source=$script:source.Replace('static void Tile_Rotate_Switch()','void UiPages::RotateTile()').Replace('static void List_Rotate_Switch()','void UiPages::RotateList()')
    $script:source=$script:source.Replace('Tile_Rotate_Switch();','UiPages::RotateTile();').Replace('List_Rotate_Switch();','UiPages::RotateList();')
    $script:source=$script:source.Replace('static void Page_Param_Init(UI_PAGE page)','void UiPages::InitPage(UI_PAGE page)').Replace('Page_Param_Init);','UiPages::InitPage);')
    foreach($part in @('Main','Bri','Color','Effect')) {
        $script:source=$script:source.Replace("static void Light_${part}_Param_Init()","void UiPages::Light_${part}_Param_Init()")
    }
    $includes="#include `"../UiContext.h`"`n#include `"UiPages.h`"`n#include `"../MainPageState.h`"`n#include `"../UiNavigation.h`"`n#include `"HAL/HAL.h`""
    Move-Functions 'UiPages/HomePages.cpp' @('OledUI::Tile_Param_Init','OledUI::Sleep_Param_Init','OledUI::Sleep_Proc','OledUI::Main_Proc','OledUI::Editor_Proc') $includes
    Move-Functions 'UiPages/ToolPages.cpp' @('OledUI::Knob_Param_Init','OledUI::Krf_Param_Init','OledUI::Kpf_Param_Init','OledUI::Volt_Param_Init','OledUI::Knob_Proc','OledUI::Krf_Proc','OledUI::Kpf_Proc','OledUI::Volt_Proc') $includes
    Move-Functions 'UiPages/SettingPages.cpp' @('OledUI::Setting_Param_Init','OledUI::AnimationCurve_Param_Init','OledUI::Setting_Proc','OledUI::AnimationCurve_Proc','OledUI::About_Proc') $includes
    $color=[regex]::Match($script:source,'(?m)^static uint8_t light_color_select = 0;')
    if(!$color.Success){throw 'Missing color selection'}
    $script:source=$script:source.Remove($color.Index,$color.Length)
    Move-Functions 'UiPages/LightPages.cpp' @('UiPages::Light_Main_Param_Init','UiPages::Light_Bri_Param_Init','UiPages::Light_Color_Param_Init','UiPages::Light_Effect_Param_Init','OledUI::Light_Main_Show','OledUI::Light_Main_Proc','OledUI::Light_Bri_Show','OledUI::Light_Bri_Proc','OledUI::Light_Color_Show','OledUI::Light_Color_Proc','OledUI::Light_Effect_Show','OledUI::Light_Effect_Proc') ($includes+"`n"+$lightState+"`n"+$color.Value)
    Move-Functions 'UiPages/PageNavigation.cpp' @('OledUI::CheckBox_Value_Init','OledUI::CheckBox_Multi_Init','OledUI::CheckBox_Single_Init','OledUI::CheckBox_M_Select','OledUI::CheckBox_S_Select','OledUI::Window_Value_Init','UiPages::InitPage','OledUI::NavigateTo','OledUI::Back','OledUI::OpenRoot','OledUI::Layer_Init_In','OledUI::Layer_Init_Out','OledUI::Window_Param_Init','UiPages::RotateTile','UiPages::RotateList','OledUI::Window_Proc') $includes
    $switch=[regex]::Match($script:source,'switch \(ui.index\)\s*\{.*?\r?\n        \}',[Text.RegularExpressions.RegexOptions]::Singleline)
    if(!$switch.Success){throw 'Missing page dispatcher'}
    $script:source=$script:source.Remove($switch.Index,$switch.Length).Insert($switch.Index,'UiPages::Process(static_cast<UI_PAGE>(ui.index));')
    $script:source=$script:source.Replace('#include "OledUI.h"',"#include `"OledUI.h`"`n#include `"UiContext.h`"`n#include `"UiPages/UiPages.h`"")
    $script:source=[regex]::Replace($script:source,'(?m)^const uint8_t analog_pin\[10\].*\r?\n','')
    $script:source=[regex]::Replace($script:source,'(?m)^//.*\r?\n','')
    $script:source=[regex]::Replace($script:source,'(\r?\n\s*){3,}',"`n`n")
    foreach($file in @('UiInput.cpp','UiRenderer.cpp')) {
        $path=Join-Path $root $file
        $text=[IO.File]::ReadAllText($path).Replace('#include "OledUI.h"','#include "UiContext.h"')
        [IO.File]::WriteAllText($path,$text,[Text.UTF8Encoding]::new($false))
    }
}
[IO.File]::WriteAllText($sourcePath,$script:source,[Text.UTF8Encoding]::new($false))
