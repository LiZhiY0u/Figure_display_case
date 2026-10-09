#include "../UiContext.h"
#include "UiPages.h"
#include "../MainPageState.h"
#include "../UiNavigation.h"
#include "HAL/HAL.h"
void OledUI::Setting_Param_Init()
{
    CheckBox_Value_Init(ui.param);
    CheckBox_Multi_Init(ui.param);
}

void OledUI::AnimationCurve_Param_Init()
{
    CheckBox_Single_Init(&ui.param[ANIM_CURVE], &ui.param[ANIM_CURVE_POS]);
}

void OledUI::Setting_Proc()
{
    List_Show(setting_menu, M_SETTING);

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
            case 0:
                Back();
                break;
            case 1: Window_Value_Init("Disp Bri", DISP_BRI, &ui.param[DISP_BRI], 255, 0, 5, setting_menu, M_SETTING); break;
            case 2: Window_Value_Init("Tile Ani", TILE_ANI, &ui.param[TILE_ANI], 100, 10, 1, setting_menu, M_SETTING); break;
            case 3: Window_Value_Init("List Ani", LIST_ANI, &ui.param[LIST_ANI], 100, 10, 1, setting_menu, M_SETTING); break;
            case 4: Window_Value_Init("Win Ani",  WIN_ANI,  &ui.param[WIN_ANI],  100, 10, 1, setting_menu, M_SETTING); break;
            case 5: Window_Value_Init("Spot Ani", SPOT_ANI, &ui.param[SPOT_ANI], 100, 10, 1, setting_menu, M_SETTING); break;
            case 6: Window_Value_Init("Tag Ani",  TAG_ANI,  &ui.param[TAG_ANI],  100, 10, 1, setting_menu, M_SETTING); break;
            case 7: Window_Value_Init("Fade Ani", FADE_ANI, &ui.param[FADE_ANI], 255, 0, 1, setting_menu, M_SETTING); break;
            case 8: Window_Value_Init("Btn SPT",  BTN_SPT,  &ui.param[BTN_SPT],  255, 0, 1, setting_menu, M_SETTING); break;
            case 9: Window_Value_Init("Btn LPT",  BTN_LPT,  &ui.param[BTN_LPT],  255, 0, 1, setting_menu, M_SETTING); break;
            case 10: CheckBox_M_Select(TILE_UFD);  break;
            case 11: CheckBox_M_Select(LIST_UFD);  break;
            case 12: CheckBox_M_Select(TILE_LOOP); break;
            case 13: CheckBox_M_Select(LIST_LOOP); break;
            case 14: CheckBox_M_Select(WIN_BOK);   break;
            case 15: CheckBox_M_Select(KNOB_DIR);  break;
            case 16: CheckBox_M_Select(DARK_MODE); break;
            case 17:
                NavigateTo(M_ANIM_CURVE);
                break;
            case 18:
                NavigateTo(M_ABOUT);
                break;
            }
            break;
        }
    }
}

void OledUI::AnimationCurve_Proc()
{
    List_Show(animation_curve_menu, M_ANIM_CURVE);

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
            if (ui.select[ui.layer] == 0)
                Back();
            else
                SetAnimationCurve(
                    AnimationCurveFromSelectionPosition(ui.select[ui.layer]));
            break;
        }
    }
}

void OledUI::About_Proc()
{
    List_Show(about_menu, M_ABOUT);

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
            if (ui.select[ui.layer] == 0)
            {
                Back();
            }
            break;
        }
    }
}

