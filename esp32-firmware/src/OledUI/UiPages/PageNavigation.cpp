#include "../UiContext.h"
#include "UiPages.h"
#include "../MainPageState.h"
#include "../UiNavigation.h"
#include "HAL/HAL.h"
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

void UiPages::InitPage(UI_PAGE page)
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
    UiNavigation::OpenRoot(ui, page, UiPages::InitPage);
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
    UiNavigation::CompleteEnter(ui, UiPages::InitPage);
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
    UiNavigation::CompleteBack(ui, UiPages::InitPage);
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

void UiPages::RotateTile()
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

void UiPages::RotateList()
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

