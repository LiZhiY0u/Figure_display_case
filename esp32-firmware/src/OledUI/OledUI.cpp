/**
 * @file   OledUI.cpp
 * @author LZY (original author: RQNG, WouoUI)
 * @brief  OLED UI layer implementation
 *
 * All UI logic: state machines, animations, per-page show/proc functions,
 * button scanning, fade/tile/list/window effects.
 */

#include "OledUI.h"
#include "MainPageState.h"
#include "UiNavigation.h"
#include "ConnectionToast.h"
#include "Input/SerialDebugInput.h"
#include "Input/InputQueue.h"
#include "Input/InputAdapters.h"
#include "UiInputAdapter.h"
#include "HAL/HAL.h"
#include "Encoder/HAL_Encoder.h"

#ifndef ENABLE_SERIAL_INPUT_DEBUG
#define ENABLE_SERIAL_INPUT_DEBUG 0
#endif

// ===========================
// External references
// ===========================
// u8g2     - declared in OledDriver.h, defined in OledDriver.cpp
// Menu data     - in OledMenus.cpp

// (No local definitions here - all globals come from their respective headers)

// Analog channel pins for voltage monitor
const uint8_t analog_pin[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

// ===========================
// Global UI contexts
// ===========================
UI_Context ui;
Tile_Context tile;
List_Context list;
Volt_Context volt;
CheckBox_Context check_box;
Win_Context win;
Knob_Context knob;
volatile Button_Context btn;

// ===========================
// Light 灯带全局变量 (用户实现这些变量)
// ===========================
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

// ===========================
// ===========================
// BUTTON / ENCODER SCANNING
// ===========================
// ===========================



// ===========================
// ===========================
// UI PARAM & PAGE INIT
// ===========================
// ===========================

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

// ---- Checkbox helpers ----

void OledUI::CheckBox_Value_Init(uint8_t* param)
{
    check_box.v = param;
}

void OledUI::CheckBox_Multi_Init(uint8_t* param)
{
    check_box.m = param;
}

void OledUI::CheckBox_Single_Init(uint8_t* param, uint8_t* param_pos)
{
    check_box.s  = param;
    check_box.s_p = param_pos;
}

void OledUI::CheckBox_M_Select(uint8_t idx)
{
    check_box.m[idx] = !check_box.m[idx];
}

void OledUI::CheckBox_S_Select(uint8_t val, uint8_t pos)
{
    *check_box.s  = val;
    *check_box.s_p = pos;
}

// ---- Window helpers ----

void OledUI::Window_Value_Init(
    const char* title, uint8_t sel_param, uint8_t* value,
    uint8_t max, uint8_t min, uint8_t step,
    const M_SELECT* bg, uint8_t bg_index)
{
    strncpy(win.title, title, sizeof(win.title) - 1);
    win.title[sizeof(win.title) - 1] = '\0';
    win.select = sel_param;
    win.value   = value;
    win.max     = max;
    win.min     = min;
    win.step    = step;
    win.bg      = bg;
    win.bg_index = bg_index;
    ui.index    = M_WINDOW;
    ui.state    = S_WINDOW;
}

// ---- Page-specific param init ----

void OledUI::Tile_Param_Init()
{
    MainPageState::Initialize(ui, tile, {
        TILE_ICON_S,
        TILE_ICON_H,
        TILE_INDI_W,
        TILE_INDI_S,
        TILE_INDI_H,
        TILE_B_TITLE_H
    });
    ResetAnimationTrack(tile.icon_x_track);
    ResetAnimationTrack(tile.icon_y_track);
    ResetAnimationTrack(tile.indi_x_track);
    ResetAnimationTrack(tile.title_y_track);
}

void OledUI::Sleep_Param_Init()
{
    u8g2.setDrawColor(0);
    u8g2.drawBox(0, 0, DISP_W, DISP_H);
    u8g2.setPowerSave(1);
    ui.state = S_NONE;
    ui.sleep = true;
}

void OledUI::Knob_Param_Init()
{
    CheckBox_Value_Init(knob.param);
}

void OledUI::Krf_Param_Init()
{
    CheckBox_Single_Init(&knob.param[KNOB_ROT], &knob.param[KNOB_ROT_P]);
}

void OledUI::Kpf_Param_Init()
{
    CheckBox_Single_Init(&knob.param[KNOB_COD], &knob.param[KNOB_COD_P]);
}

void OledUI::Volt_Param_Init()
{
    volt.text_bg_r = 0;
    volt.text_bg_r_trg = VOLT_TEXT_BG_W;
    ResetAnimationTrack(volt.text_bg_r_track);
}

void OledUI::Setting_Param_Init()
{
    CheckBox_Value_Init(ui.param);
    CheckBox_Multi_Init(ui.param);
}

void OledUI::AnimationCurve_Param_Init()
{
    CheckBox_Single_Init(&ui.param[ANIM_CURVE], &ui.param[ANIM_CURVE_POS]);
}

// ---- Layer transition init ----

// Forward declarations for Light 灯带页面
static void Light_Main_Param_Init();
static void Light_Bri_Param_Init();
static void Light_Color_Param_Init();
static void Light_Effect_Param_Init();

static void Page_Param_Init(UI_PAGE page)
{
    ui.init = false;

    switch (page)
    {
    case M_SLEEP:       OledUI::Sleep_Param_Init(); break;
    case M_MAIN:        OledUI::Tile_Param_Init(); break;
    case M_KNOB:        OledUI::Knob_Param_Init(); break;
    case M_KRF:         OledUI::Krf_Param_Init(); break;
    case M_KPF:         OledUI::Kpf_Param_Init(); break;
    case M_VOLT:        OledUI::Volt_Param_Init(); break;
    case M_SETTING:     OledUI::Setting_Param_Init(); break;
    case M_ANIM_CURVE:  OledUI::AnimationCurve_Param_Init(); break;
    case M_LIGHT_MAIN:  Light_Main_Param_Init(); break;
    case M_LIGHT_BRI:   Light_Bri_Param_Init(); break;
    case M_LIGHT_COLOR: Light_Color_Param_Init(); break;
    case M_LIGHT_EFFECT:Light_Effect_Param_Init(); break;
    default: break;
    }
}

bool OledUI::NavigateTo(UI_PAGE page)
{
    return UiNavigation::RequestEnter(ui, page) == UiNavigation::Result::Accepted;
}

bool OledUI::Back()
{
    return UiNavigation::RequestBack(ui) == UiNavigation::Result::Accepted;
}

void OledUI::OpenRoot(UI_PAGE page)
{
    UiNavigation::OpenRoot(ui, page, Page_Param_Init);
}

void OledUI::Layer_Init_In()
{
    ui.init = false;
    list.y = 0;
    list.y_trg = LIST_LINE_H;
    list.box_x = 0;
    list.box_y = 0;
    list.bar_y = 0;
    ResetAnimationTrack(list.y_track);
    ResetAnimationTrack(list.box_x_track);
    ResetAnimationTrack(list.box_y_track);
    ResetAnimationTrack(list.bar_y_track);
    UiNavigation::CompleteEnter(ui, Page_Param_Init);
}

void OledUI::Layer_Init_Out()
{
    list.box_y_trg[ui.layer] = 0;
    ui.init = false;
    list.y = 0;
    list.y_trg = LIST_LINE_H;
    list.bar_y = 0;
    ResetAnimationTrack(list.y_track);
    ResetAnimationTrack(list.box_x_track);
    ResetAnimationTrack(list.box_y_track);
    ResetAnimationTrack(list.bar_y_track);
    UiNavigation::CompleteBack(ui, Page_Param_Init);
}

void OledUI::Window_Param_Init()
{
    win.bar = 0;
    win.y = WIN_Y;
    win.y_trg = win.u;
    ResetAnimationTrack(win.bar_track);
    ResetAnimationTrack(win.y_track);
    ui.state = S_NONE;
}

// ===========================
// ===========================
// ANIMATION
// ===========================
// ===========================









// ===========================
// ===========================
// SHOW / RENDER FUNCTIONS
// ===========================
// ===========================

// ---- Tile show (icon + title) ----



// ---- List tail rendering helpers ----













// ---- List show ----



// ---- Volt show ----



// ---- Window show ----



// ===========================
// ===========================
// PAGE PROCESS FUNCTIONS
// ===========================
// ===========================

// ---- Tile rotate helper ----

static void Tile_Rotate_Switch()
{
    switch (btn.id)
    {
    case BTN_ID_CC:
        if (ui.init)
        {
            if (ui.select[ui.layer] > 0)
            {
                ui.select[ui.layer]--;
                tile.icon_x_trg += TILE_ICON_S;
                tile.select_flag = false;
            }
            else
            {
                if (ui.param[TILE_LOOP])
                {
                    ui.select[ui.layer] = ui.num[ui.index] - 1;
                    tile.icon_x_trg = -TILE_ICON_S * (ui.num[ui.index] - 1);
                }
                else
                    tile.select_flag = true;
            }
        }
        break;

    case BTN_ID_CW:
        if (ui.init)
        {
            if (ui.select[ui.layer] < (ui.num[ui.index] - 1))
            {
                ui.select[ui.layer]++;
                tile.icon_x_trg -= TILE_ICON_S;
                tile.select_flag = false;
            }
            else
            {
                if (ui.param[TILE_LOOP])
                {
                    ui.select[ui.layer] = 0;
                    tile.icon_x_trg = 0;
                }
                else
                    tile.select_flag = true;
            }
        }
        break;
    }
}

// ---- List rotate helper ----

static void List_Rotate_Switch()
{
    if (!list.loop)
    {
        switch (btn.id)
        {
        case BTN_ID_CC:
            if (ui.select[ui.layer] == 0)
            {
                if (ui.param[LIST_LOOP] && ui.init)
                {
                    list.loop = true;
                    ui.select[ui.layer] = ui.num[ui.index] - 1;
                    if (ui.num[ui.index] > list.line_n)
                    {
                        list.box_y_trg[ui.layer] = DISP_H - LIST_LINE_H;
                        list.y_trg = DISP_H - ui.num[ui.index] * LIST_LINE_H;
                    }
                    else
                        list.box_y_trg[ui.layer] = (ui.num[ui.index] - 1) * LIST_LINE_H;
                }
                break;
            }
            if (ui.init)
            {
                ui.select[ui.layer]--;
                if (ui.select[ui.layer] < -(list.y_trg / LIST_LINE_H))
                {
                    if (!(DISP_H % LIST_LINE_H))
                        list.y_trg += LIST_LINE_H;
                    else
                    {
                        if (list.box_y_trg[ui.layer] == DISP_H - LIST_LINE_H * list.line_n)
                        {
                            list.y_trg += (list.line_n + 1) * LIST_LINE_H - DISP_H;
                            list.box_y_trg[ui.layer] = 0;
                        }
                        else if (list.box_y_trg[ui.layer] == LIST_LINE_H)
                            list.box_y_trg[ui.layer] = 0;
                        else
                            list.y_trg += LIST_LINE_H;
                    }
                }
                else
                    list.box_y_trg[ui.layer] -= LIST_LINE_H;
            }
            break;

        case BTN_ID_CW:
            if (ui.select[ui.layer] == (ui.num[ui.index] - 1))
            {
                if (ui.param[LIST_LOOP] && ui.init)
                {
                    list.loop = true;
                    ui.select[ui.layer] = 0;
                    list.y_trg = 0;
                    list.box_y_trg[ui.layer] = 0;
                }
                break;
            }
            if (ui.init)
            {
                ui.select[ui.layer]++;
                if ((ui.select[ui.layer] + 1) > (list.line_n - list.y_trg / LIST_LINE_H))
                {
                    if (!(DISP_H % LIST_LINE_H))
                        list.y_trg -= LIST_LINE_H;
                    else
                    {
                        if (list.box_y_trg[ui.layer] == LIST_LINE_H * (list.line_n - 1))
                        {
                            list.y_trg -= (list.line_n + 1) * LIST_LINE_H - DISP_H;
                            list.box_y_trg[ui.layer] = DISP_H - LIST_LINE_H;
                        }
                        else if (list.box_y_trg[ui.layer] == DISP_H - LIST_LINE_H * 2)
                            list.box_y_trg[ui.layer] = DISP_H - LIST_LINE_H;
                        else
                            list.y_trg -= LIST_LINE_H;
                    }
                }
                else
                    list.box_y_trg[ui.layer] += LIST_LINE_H;
            }
            break;
        }
    }
}

// ---- Window process ----

void OledUI::Window_Proc()
{
    Window_Show();

    if (win.y == WIN_Y_TRG)
        ui.index = win.bg_index;

    if (btn.pressed && win.y == win.y_trg && win.y != WIN_Y_TRG)
    {
        btn.pressed = false;
        switch (btn.id)
        {
        case BTN_ID_CW:
            if (*win.value < win.max) *win.value += win.step;
            break;
        case BTN_ID_CC:
            if (*win.value > win.min) *win.value -= win.step;
            break;
        case BTN_ID_SP:
        case BTN_ID_LP:
            win.y_trg = WIN_Y_TRG;
            break;
        }
    }
}

// ---- Sleep process ----

void OledUI::Sleep_Proc()
{
    if (!ui.sleep || !btn.pressed)
        return;

    // Wake on the next available input event. Keep this handler non-blocking so
    // Key_Scan() continues to run on every UI task iteration.
    btn.pressed = false;
    ui.sleep = false;
    u8g2.setPowerSave(0);

    OpenRoot(M_MAIN);
}

// ---- Main menu process ----

void OledUI::Main_Proc()
{
    Tile_Show(main_menu, main_icon_pic);

    if (btn.pressed)
    {
        btn.pressed = false;
        switch (btn.id)
        {
        case BTN_ID_CW:
        case BTN_ID_CC:
            Tile_Rotate_Switch();
            break;
        case BTN_ID_SP:
            switch (ui.select[ui.layer])
            {
            case 0: NavigateTo(M_LIGHT_MAIN); break;
            case 1: NavigateTo(M_EDITOR); break;
            case 2: NavigateTo(M_VOLT); break;
            case 3: NavigateTo(M_SETTING); break;
            }
            break;
        }
        if (!tile.select_flag && ui.init)
        {
            tile.indi_x = 0;
            tile.title_y = tile.title_y_calc;
        }
    }
}

// ---- Editor process ----

void OledUI::Editor_Proc()
{
    List_Show(editor_menu, M_EDITOR);

    if (btn.pressed)
    {
        btn.pressed = false;
        switch (btn.id)
        {
        case BTN_ID_CW:
        case BTN_ID_CC:
            List_Rotate_Switch();
            break;
        case BTN_ID_LP:
            ui.select[ui.layer] = 0;
        case BTN_ID_SP:
            switch (ui.select[ui.layer])
            {
            case 0:  Back(); break;
            case 11: NavigateTo(M_KNOB); break;
            }
            break;
        }
    }
}

// ---- Knob process ----

void OledUI::Knob_Proc()
{
    List_Show(knob_menu, M_KNOB);

    if (btn.pressed)
    {
        btn.pressed = false;
        switch (btn.id)
        {
        case BTN_ID_CW:
        case BTN_ID_CC:
            List_Rotate_Switch();
            break;
        case BTN_ID_LP:
            ui.select[ui.layer] = 0;
        case BTN_ID_SP:
            switch (ui.select[ui.layer])
            {
            case 0: Back(); break;
            case 1:
                NavigateTo(M_KRF);
                break;
            case 2:
                NavigateTo(M_KPF);
                break;
            }
            break;
        }
    }
}

// ---- KRF process ----

void OledUI::Krf_Proc()
{
    List_Show(krf_menu, M_KRF);

    if (btn.pressed)
    {
        btn.pressed = false;
        switch (btn.id)
        {
        case BTN_ID_CW:
        case BTN_ID_CC:
            List_Rotate_Switch();
            break;
        case BTN_ID_LP:
            ui.select[ui.layer] = 0;
        case BTN_ID_SP:
            switch (ui.select[ui.layer])
            {
            case 0: Back(); break;
            case 2: CheckBox_S_Select(KNOB_DISABLE, ui.select[ui.layer]); break;
            case 4: CheckBox_S_Select(KNOB_ROT_VOL, ui.select[ui.layer]); break;
            case 5: CheckBox_S_Select(KNOB_ROT_BRI, ui.select[ui.layer]); break;
            }
            break;
        }
    }
}

// ---- KPF process ----

void OledUI::Kpf_Proc()
{
    List_Show(kpf_menu, M_KPF);

    if (btn.pressed)
    {
        btn.pressed = false;
        switch (btn.id)
        {
        case BTN_ID_CW:
        case BTN_ID_CC:
            List_Rotate_Switch();
            break;
        case BTN_ID_LP:
            ui.select[ui.layer] = 0;
        case BTN_ID_SP:
            // Key mapping disabled in this build (stub only)
            if (ui.select[ui.layer] == 0)
            {
                Back();
            }
            break;
        }
    }
}

// ---- Volt process ----

void OledUI::Volt_Proc()
{
    Volt_Show();

    if (btn.pressed)
    {
        btn.pressed = false;
        switch (btn.id)
        {
        case BTN_ID_CW:
        case BTN_ID_CC:
            List_Rotate_Switch();
            break;
        case BTN_ID_SP:
        case BTN_ID_LP:
            Back();
            break;
        }
    }
}

// ---- Setting process ----

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
            List_Rotate_Switch();
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

// ---- Runtime animation curve process ----

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
            List_Rotate_Switch();
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

// ---- About process ----

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
            List_Rotate_Switch();
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

// ===========================
// ===========================
// MAIN UI TASK
// ===========================
// ===========================

// Forward declaration (ui_proc defined below)
static void ui_proc();

void OledUI::Task()
{
    Key_Scan();
    ui_proc();
}

// Master UI dispatcher
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
        switch (ui.index)
        {
        case M_WINDOW:  OledUI::Window_Proc();  break;
        case M_SLEEP:   OledUI::Sleep_Proc();   break;
        case M_MAIN:    OledUI::Main_Proc();    break;
        case M_EDITOR:  OledUI::Editor_Proc();  break;
        case M_KNOB:    OledUI::Knob_Proc();    break;
        case M_KRF:     OledUI::Krf_Proc();     break;
        case M_KPF:     OledUI::Kpf_Proc();     break;
        case M_VOLT:    OledUI::Volt_Proc();    break;
        case M_SETTING: OledUI::Setting_Proc(); break;
        case M_ANIM_CURVE: OledUI::AnimationCurve_Proc(); break;
        case M_ABOUT:   OledUI::About_Proc();   break;
        // Light 灯带
        case M_LIGHT_MAIN:  OledUI::Light_Main_Proc();   break;
        case M_LIGHT_BRI:   OledUI::Light_Bri_Proc();    break;
        case M_LIGHT_COLOR: OledUI::Light_Color_Proc();  break;
        case M_LIGHT_EFFECT:OledUI::Light_Effect_Proc(); break;
        }
        ConnectionToast::Draw();
        break;
    }
}

// ===========================
// Top-level init
// ===========================

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

// ===========================
// ===========================
// LIGHT 灯带页面实现
// ===========================
// ===========================

// ---- Light 主菜单 参数初始化 ----

static void Light_Main_Param_Init()
{
    // The submenu entries are display-only until LED hardware control is
    // connected. They intentionally use the plain '-' menu type.
}

static void Light_Bri_Param_Init()
{
    OledUI::CheckBox_Value_Init(&light_brightness);
}

// light_color_select: 追踪当前选中的颜色项 (0-4)
static uint8_t light_color_select = 0;

static void Light_Color_Param_Init()
{
    OledUI::CheckBox_Single_Init(&light_color_select, &light_color_select);
}

static void Light_Effect_Param_Init()
{
    OledUI::CheckBox_Single_Init(&light_effect_id, &light_effect_id);
}

// ---- Light 主菜单显示 ----

void OledUI::Light_Main_Show()
{
    List_Show(light_main_menu, M_LIGHT_MAIN);
}

// ---- Light 主菜单处理 ----

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
            List_Rotate_Switch();
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

// ---- Light 亮度调节 显示 ----

void OledUI::Light_Bri_Show()
{
    List_Show(light_bri_menu, M_LIGHT_BRI);
}

// ---- Light 亮度调节 处理 ----
// 用户需实现: 将 light_brightness 应用到灯带硬件
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
            List_Rotate_Switch();
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

// ---- Light 颜色选择 显示 ----

void OledUI::Light_Color_Show()
{
    List_Show(light_color_menu, M_LIGHT_COLOR);
}

// ---- Light 颜色选择 处理 ----
// 用户需实现: 将 light_color_r/g/b 应用到灯带硬件
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
            List_Rotate_Switch();
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

// ---- Light 效果选择 显示 ----

void OledUI::Light_Effect_Show()
{
    List_Show(light_effect_menu, M_LIGHT_EFFECT);
}

// ---- Light 效果选择 处理 ----
// 用户需实现: 根据 light_effect_id 执行对应效果函数
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
            List_Rotate_Switch();
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
