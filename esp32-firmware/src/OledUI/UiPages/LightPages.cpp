#include "../UiContext.h"
#include "UiPages.h"
#include "../MainPageState.h"
#include "../UiNavigation.h"
#include "HAL/HAL.h"
namespace {
uint8_t light_brightness = 128;  // 亮度 0-255，待用户实现具体控制逻辑
uint8_t light_color_r = 255;       // RGB颜色，待用户实现
uint8_t light_color_g = 255;
uint8_t light_color_b = 255;
uint8_t light_effect_id = 0;       // 当前效果ID 0-6，待用户实现效果函数
// 效果名称表 (供显示使用)
const char* light_effect_name[] = {
    "Static",   // 0
    "Breath",   // 1
    "Rainbow",  // 2
    "Wave",     // 3
    "Strobe",   // 4
    "Comet",    // 5
    "Sparkle",  // 6
};
static uint8_t light_color_select = 0;
} // namespace
void UiPages::Light_Main_Param_Init()
{
    // The submenu entries are display-only until LED hardware control is
    // connected. They intentionally use the plain '-' menu type.
}

void UiPages::Light_Bri_Param_Init()
{
    OledUI::CheckBox_Value_Init(&light_brightness);
}

void UiPages::Light_Color_Param_Init()
{
    OledUI::CheckBox_Single_Init(&light_color_select, &light_color_select);
}

void UiPages::Light_Effect_Param_Init()
{
    OledUI::CheckBox_Single_Init(&light_effect_id, &light_effect_id);
}

void OledUI::Light_Main_Show()
{
    List_Show(light_main_menu, M_LIGHT_MAIN);
}

void OledUI::Light_Main_Proc()
{
    Light_Main_Show();

    if (btn.pressed)
    {
        btn.pressed = false;
        switch (btn.id)
        {
        case BTN_ID_CW:
        case BTN_ID_CC:
            UiPages::RotateList();
            break;
        case BTN_ID_LP:
            ui.select[ui.layer] = 0;
        case BTN_ID_SP:
            switch (ui.select[ui.layer])
            {
            case 0:  // Return
                Back();
                break;
            case 1: // Brightness (WIP)
            case 2: // Color (WIP)
            case 3: // Effect (WIP)
                // Deliberately disabled until these pages update the LED task.
                break;
            }
            break;
        }
    }
}

void OledUI::Light_Bri_Show()
{
    List_Show(light_bri_menu, M_LIGHT_BRI);
}

void OledUI::Light_Bri_Proc()
{
    Light_Bri_Show();

    if (btn.pressed)
    {
        btn.pressed = false;
        switch (btn.id)
        {
        case BTN_ID_CW:
        case BTN_ID_CC:
            UiPages::RotateList();
            // ========== 用户实现区 ==========
            // 例: 旋转调整 light_brightness
            // if (btn.id == BTN_ID_CW && light_brightness < 255) light_brightness++;
            // if (btn.id == BTN_ID_CC && light_brightness > 0) light_brightness--;
            // ApplyToStrip();
            // ==================================
            break;
        case BTN_ID_LP:
            ui.select[ui.layer] = 0;
        case BTN_ID_SP:
            switch (ui.select[ui.layer])
            {
            case 0:  // Return
                Back();
                break;
            case 1:  // 亮度滑动条弹窗
                // ========== 用户实现区 ==========
                // Window_Value_Init("Bright", ..., &light_brightness, 255, 0, 5, light_bri_menu, M_LIGHT_BRI);
                // ==================================
                break;
            }
            break;
        }
    }
}

void OledUI::Light_Color_Show()
{
    List_Show(light_color_menu, M_LIGHT_COLOR);
}

void OledUI::Light_Color_Proc()
{
    Light_Color_Show();

    if (btn.pressed)
    {
        btn.pressed = false;
        switch (btn.id)
        {
        case BTN_ID_CW:
        case BTN_ID_CC:
            UiPages::RotateList();
            break;
        case BTN_ID_LP:
            ui.select[ui.layer] = 0;
        case BTN_ID_SP:
            switch (ui.select[ui.layer])
            {
            case 0:  // Return
                Back();
                break;
            // ========== 用户实现区 ==========
            // case 1: light_color_r=255; light_color_g=0;   light_color_b=0;   ApplyColor(); break;  // Red
            // case 2: light_color_r=0;   light_color_g=255; light_color_b=0;   ApplyColor(); break;  // Green
            // case 3: light_color_r=0;   light_color_g=0;   light_color_b=255; ApplyColor(); break;  // Blue
            // case 4: light_color_r=255; light_color_g=255; light_color_b=255; ApplyColor(); break;  // White
            // case 5: NavigateTo(M_LIGHT_COLOR_CUSTOM); break; // Custom RGB 预留
            // ==================================
            }
            break;
        }
    }
}

void OledUI::Light_Effect_Show()
{
    List_Show(light_effect_menu, M_LIGHT_EFFECT);
}

void OledUI::Light_Effect_Proc()
{
    Light_Effect_Show();

    if (btn.pressed)
    {
        btn.pressed = false;
        switch (btn.id)
        {
        case BTN_ID_CW:
        case BTN_ID_CC:
            UiPages::RotateList();
            // ========== 用户实现区 ==========
            // 例: 旋转时预览效果名称
            // EffectPreview(light_effect_id);
            // ==================================
            break;
        case BTN_ID_LP:
            ui.select[ui.layer] = 0;
        case BTN_ID_SP:
            switch (ui.select[ui.layer])
            {
            case 0:  // Return
                Back();
                break;
            // ========== 用户实现区 ==========
            // case 1: light_effect_id = 0; StopEffect(); StaticColor(light_color_r, light_color_g, light_color_b); break;  // Static
            // case 2: light_effect_id = 1; StopEffect(); StartEffect(Breath, light_color_r, light_color_g, light_color_b); break;  // Breath
            // case 3: light_effect_id = 2; StopEffect(); StartEffect(Rainbow, ...); break;  // Rainbow
            // case 4: light_effect_id = 3; StopEffect(); StartEffect(Wave, ...); break;   // Wave
            // case 5: light_effect_id = 4; StopEffect(); StartEffect(Strobe, ...); break;  // Strobe
            // case 6: light_effect_id = 5; StopEffect(); StartEffect(Comet, ...); break;  // Comet
            // case 7: light_effect_id = 6; StopEffect(); StartEffect(Sparkle, ...); break; // Sparkle
            // ==================================
            }
            break;
        }
    }
}
