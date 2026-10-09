/**
 * @file   OledUI.cpp
 * @author LZY (original author: RQNG, WouoUI)
 * @brief  OLED UI layer implementation
 *
 * System defaults, initialization and UI state dispatch.
 */

#include "OledUI.h"
#include "UiContext.h"
#include "UiPages/UiPages.h"
#include "ConnectionToast.h"

void OledUI::UI_Param_Init()
{
    ui.param[DISP_BRI] = 255;
    ui.param[TILE_ANI] = 30;
    ui.param[LIST_ANI] = 60;
    ui.param[WIN_ANI] = 25;
    ui.param[SPOT_ANI] = 50;
    ui.param[TAG_ANI] = 60;
    ui.param[FADE_ANI] = 30;
    ui.param[BTN_SPT] = 25;
    ui.param[BTN_LPT] = 150;
    ui.param[TILE_UFD] = 1;
    ui.param[LIST_UFD] = 1;
    ui.param[TILE_LOOP] = 0;
    ui.param[LIST_LOOP] = 0;
    ui.param[WIN_BOK] = 0;
    ui.param[KNOB_DIR] = 0;
    ui.param[DARK_MODE] = 1;
    OledUI::SetAnimationCurve(AnimationCurve::OutCubic);
}

void OledUI::UI_Init()
{
    ui.num[M_MAIN]   = main_menu_count;
    ui.num[M_EDITOR] = editor_menu_count;
    ui.num[M_KNOB]   = knob_menu_count;
    ui.num[M_KRF]    = krf_menu_count;
    ui.num[M_KPF]    = kpf_menu_count;
    ui.num[M_VOLT]   = volt_menu_count;
    ui.num[M_SETTING]= setting_menu_count;
    ui.num[M_ANIM_CURVE] = animation_curve_menu_count;
    ui.num[M_ABOUT]  = about_menu_count;
    // Light 灯带
    ui.num[M_LIGHT_MAIN] = light_main_menu_count;
    ui.num[M_LIGHT_BRI]  = light_bri_menu_count;
    ui.num[M_LIGHT_COLOR] = light_color_menu_count;
    ui.num[M_LIGHT_EFFECT]= light_effect_menu_count;
}

static void ui_proc();

void OledUI::Task()
{
    Key_Scan();
    ui_proc();
}

static void ui_proc()
{
    OledDriver::SendBuffer();

    switch (ui.state)
    {
    case S_FADE:
        OledUI::Fade();
        break;
    case S_WINDOW:
        OledUI::Window_Param_Init();
        break;
    case S_LAYER_IN:
        OledUI::Layer_Init_In();
        break;
    case S_LAYER_OUT:
        OledUI::Layer_Init_Out();
        break;
    case S_NONE:
        OledDriver::ClearBuffer();
        UiPages::Process(static_cast<UI_PAGE>(ui.index));
        ConnectionToast::Draw();
        break;
    }
}

void OledUI::Init()
{
    // Init list context
    list.line_n = DISP_H / LIST_LINE_H;

    // Init knob defaults
    knob.param[KNOB_ROT]  = KNOB_DISABLE;
    knob.param[KNOB_COD]  = KNOB_DISABLE;
    knob.param[KNOB_ROT_P] = 2;
    knob.param[KNOB_COD_P] = 2;

    // UI subsystem
    UI_Param_Init();
    UI_Init();

    // Hardware
    OledDriver::Init();
    OledDriver::SetContrast(ui.param[DISP_BRI]);

    // Boot to main menu through the same lifecycle used by wake-up.
    OpenRoot(M_MAIN);
}
