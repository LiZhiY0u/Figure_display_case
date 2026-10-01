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
#include "Input/BleRemote.h"
#include "ConnectionToast.h"
#include "Input/SerialDebugInput.h"
#include "main.h"

#ifndef ENABLE_SERIAL_INPUT_DEBUG
#define ENABLE_SERIAL_INPUT_DEBUG 0
#endif

// ===========================
// External references
// ===========================
// u8g2     - declared in OledDriver.h, defined in OledDriver.cpp
// key_en   - declared in HAL_Encoder.h, defined in HAL_Encoder.cpp
// gamepad_axis - declared in main.h, defined in main.cpp
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

void OledUI::Key_Scan()
{
    // Encoder input via HAL_Encoder
    int16_t enc_diff = ENCODER::Encoder_GetDiff();
    if (enc_diff > 0)
    {
        btn.id = BTN_ID_CW;
        btn.pressed = true;
    }
    else if (enc_diff < 0)
    {
        btn.id = BTN_ID_CC;
        btn.pressed = true;
    }

    // Button press from main app (encoder button or external)
    if (key_en)
    {
        key_en = false;
        btn.id = BTN_ID_SP;
        btn.pressed = true;
    }

    // Gamepad axis (for compatibility)
    if (gamepad_axis == 1)
    {
        gamepad_axis = 0;
        btn.id = BTN_ID_CW;
        btn.pressed = true;
    }
    else if (gamepad_axis == 2)
    {
        gamepad_axis = 0;
        btn.id = BTN_ID_CC;
        btn.pressed = true;
    }
    else if (gamepad_axis == 3)
    {
        gamepad_axis = 0;
        btn.id = BTN_ID_SP;
        btn.pressed = true;
    }

    // BLE remote input. Keep hardware input priority and consume at most one
    // command per UI iteration.
    RemoteKeyInput::Action remote_action = RemoteKeyInput::Action::None;
    if (!btn.pressed && BleRemote::Read(remote_action))
    {
        switch (remote_action)
        {
        case RemoteKeyInput::Action::Previous:
            btn.id = BTN_ID_CC;
            btn.pressed = true;
            break;
        case RemoteKeyInput::Action::Next:
            btn.id = BTN_ID_CW;
            btn.pressed = true;
            break;
        case RemoteKeyInput::Action::Confirm:
            btn.id = BTN_ID_SP;
            btn.pressed = true;
            break;
        case RemoteKeyInput::Action::None:
            break;
        }
    }

#if ENABLE_SERIAL_INPUT_DEBUG
    // Hardware inputs keep priority. Ignore line endings and consume at most
    // one valid debug command per UI task iteration.
    while (!btn.pressed && Serial.available() > 0)
    {
        const SerialDebugInput::Command command =
            SerialDebugInput::Decode(static_cast<char>(Serial.read()));

        switch (command)
        {
        case SerialDebugInput::Command::RotateLeft:
            btn.id = BTN_ID_CC;
            btn.pressed = true;
            break;
        case SerialDebugInput::Command::RotateRight:
            btn.id = BTN_ID_CW;
            btn.pressed = true;
            break;
        case SerialDebugInput::Command::Press:
            btn.id = BTN_ID_SP;
            btn.pressed = true;
            break;
        case SerialDebugInput::Command::None:
            break;
        }
    }
#endif
}

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

void OledUI::Animation(
    AnimationTrack& track,
    float& value,
    float target,
    uint8_t speed_param)
{
    UpdateAnimation(
        track,
        value,
        target,
        AnimationDurationMs(ui.param[speed_param]),
        GetAnimationCurve(),
        millis());
}

void OledUI::SetAnimationCurve(AnimationCurve curve)
{
    const uint8_t position = AnimationCurveSelectionPosition(curve);
    const AnimationCurve validated = AnimationCurveFromSelectionPosition(position);
    ui.param[ANIM_CURVE] = static_cast<uint8_t>(validated);
    ui.param[ANIM_CURVE_POS] = position;
}

AnimationCurve OledUI::GetAnimationCurve()
{
    return AnimationCurveFromValue(ui.param[ANIM_CURVE]);
}

void OledUI::Fade()
{
    uint8_t* buf = OledDriver::GetBufferPtr();
    uint16_t len  = OledDriver::GetBufferLen();

    delay(ui.param[FADE_ANI]);

    if (ui.param[DARK_MODE]) // Dark mode
    {
        switch (ui.fade)
        {
        case 1:
            for (uint16_t i = 0; i < len; ++i)
                if (i % 2 != 0) buf[i] = buf[i] & 0xAA;
            break;
        case 2:
            for (uint16_t i = 0; i < len; ++i)
                if (i % 2 != 0) buf[i] = buf[i] & 0x00;
            break;
        case 3:
            for (uint16_t i = 0; i < len; ++i)
                if (i % 2 == 0) buf[i] = buf[i] & 0x55;
            break;
        case 4:
            for (uint16_t i = 0; i < len; ++i)
                if (i % 2 == 0) buf[i] = buf[i] & 0x00;
            break;
        default:
            ui.state = S_NONE;
            ui.fade = 0;
            break;
        }
    }
    else // Light mode
    {
        switch (ui.fade)
        {
        case 1:
            for (uint16_t i = 0; i < len; ++i)
                if (i % 2 != 0) buf[i] = buf[i] | 0xAA;
            break;
        case 2:
            for (uint16_t i = 0; i < len; ++i)
                if (i % 2 != 0) buf[i] = buf[i] | 0x00;
            break;
        case 3:
            for (uint16_t i = 0; i < len; ++i)
                if (i % 2 == 0) buf[i] = buf[i] | 0x55;
            break;
        case 4:
            for (uint16_t i = 0; i < len; ++i)
                if (i % 2 == 0) buf[i] = buf[i] | 0x00;
            break;
        default:
            ui.state = S_NONE;
            ui.fade = 0;
            break;
        }
    }
    ui.fade++;
}

// ===========================
// ===========================
// SHOW / RENDER FUNCTIONS
// ===========================
// ===========================

// ---- Tile show (icon + title) ----

void OledUI::Tile_Show(const M_SELECT arr[], const uint8_t icon_pic[][120])
{
    Animation(tile.icon_x_track,  tile.icon_x,  tile.icon_x_trg,  TILE_ANI);
    Animation(tile.icon_y_track,  tile.icon_y,  tile.icon_y_trg,  TILE_ANI);
    Animation(tile.indi_x_track,  tile.indi_x,  tile.indi_x_trg,  TILE_ANI);
    Animation(tile.title_y_track, tile.title_y, tile.title_y_trg, TILE_ANI);

    u8g2.setDrawColor(1);

    // Draw title
    u8g2.setFont(TILE_B_FONT);
    u8g2.drawStr(
        ((DISP_W - TILE_INDI_W) - u8g2.getStrWidth(arr[ui.select[ui.layer]].m_select)) / 2 + TILE_INDI_W,
        tile.title_y,
        arr[ui.select[ui.layer]].m_select);

    // Draw indicator bar
    u8g2.drawBox(0, TILE_ICON_S, tile.indi_x, TILE_INDI_H);

    // Draw icons
    if (!ui.init)
    {
        for (uint8_t i = 0; i < ui.num[ui.index]; ++i)
        {
            if (ui.param[TILE_UFD])
                tile.temp = (DISP_W - TILE_ICON_W) / 2 + i * tile.icon_x - TILE_ICON_S * ui.select[ui.layer];
            else
                tile.temp = (DISP_W - TILE_ICON_W) / 2 + (i - ui.select[ui.layer]) * tile.icon_x;

            u8g2.drawXBMP(tile.temp, (int16_t)tile.icon_y, TILE_ICON_W, TILE_ICON_H, icon_pic[i]);
        }
        if (tile.icon_x == tile.icon_x_trg)
        {
            ui.init = true;
            tile.icon_x = tile.icon_x_trg = -ui.select[ui.layer] * TILE_ICON_S;
        }
    }
    else
    {
        for (uint8_t i = 0; i < ui.num[ui.index]; ++i)
            u8g2.drawXBMP(
                (DISP_W - TILE_ICON_W) / 2 + (int16_t)tile.icon_x + i * TILE_ICON_S,
                0, TILE_ICON_W, TILE_ICON_H, icon_pic[i]);
    }

    // Invert for dark mode mask
    u8g2.setDrawColor(2);
    if (!ui.param[DARK_MODE])
        u8g2.drawBox(0, 0, DISP_W, DISP_H);
}

// ---- List tail rendering helpers ----

static void List_Draw_Value(uint8_t data_index)
{
    if (check_box.v == nullptr)
    {
        u8g2.print("--");
        return;
    }
    u8g2.print(check_box.v[data_index]);
}

static void List_Draw_CheckBox_Frame()
{
    u8g2.drawRFrame(CHECK_BOX_L_S, list.temp + CHECK_BOX_U_S, CHECK_BOX_F_W, CHECK_BOX_F_H, 1);
}

static void List_Draw_CheckBox_Dot()
{
    u8g2.drawBox(
        CHECK_BOX_L_S + CHECK_BOX_D_S + 1,
        list.temp + CHECK_BOX_U_S + CHECK_BOX_D_S + 1,
        CHECK_BOX_F_W - (CHECK_BOX_D_S + 1) * 2,
        CHECK_BOX_F_H - (CHECK_BOX_D_S + 1) * 2);
}

static void List_Draw_Krf(uint8_t data_index)
{
    switch (check_box.v[data_index])
    {
    case 0: u8g2.print("OFF"); break;
    case 1: u8g2.print("VOL"); break;
    case 2: u8g2.print("BRI"); break;
    }
}

static void List_Draw_Kpf(uint8_t data_index)
{
    if (check_box.v[data_index] == 0)
        u8g2.print("OFF");
    else if (check_box.v[data_index] <= 90)
        u8g2.print((char)check_box.v[data_index]);
    else
        u8g2.print("?");
}

static void List_Draw_Text_And_CheckBox(const M_SELECT arr[], int i)
{
    const M_SELECT &item = arr[i];

    u8g2.drawStr(LIST_TEXT_S, list.temp + LIST_TEXT_H + LIST_TEXT_S, item.m_select);
    u8g2.setCursor(CHECK_BOX_L_S, list.temp + LIST_TEXT_H + LIST_TEXT_S);

    switch (RenderModeFor(item))
    {
    case MenuItemRenderMode::Value:
        List_Draw_Value(item.data_index);
        break;
    case MenuItemRenderMode::Toggle:
        List_Draw_CheckBox_Frame();
        if (check_box.m != nullptr && check_box.m[item.data_index] == 1)
            List_Draw_CheckBox_Dot();
        break;
    case MenuItemRenderMode::Choice:
        List_Draw_CheckBox_Frame();
        if (check_box.s_p != nullptr && *check_box.s_p == item.data_index)
            List_Draw_CheckBox_Dot();
        break;
    case MenuItemRenderMode::RotateBinding:
        List_Draw_Krf(item.data_index);
        break;
    case MenuItemRenderMode::PressBinding:
        List_Draw_Kpf(item.data_index);
        break;
    case MenuItemRenderMode::None:
        break;
    }
}

// ---- List show ----

void OledUI::List_Show(const M_SELECT arr[], uint8_t ui_index)
{
    u8g2.setFont(LIST_FONT);

    list.box_x_trg = u8g2.getStrWidth(arr[ui.select[ui.layer]].m_select) + LIST_TEXT_S * 2;
    list.bar_y_trg = ceil((ui.select[ui.layer]) * ((float)DISP_H / (ui.num[ui_index] - 1)));

    Animation(list.y_track,     list.y,     list.y_trg,                 LIST_ANI);
    Animation(list.box_x_track, list.box_x, list.box_x_trg,             LIST_ANI);
    Animation(list.box_y_track, list.box_y, list.box_y_trg[ui.layer],   LIST_ANI);
    Animation(list.bar_y_track, list.bar_y, list.bar_y_trg,             LIST_ANI);

    if (list.loop && list.box_y == list.box_y_trg[ui.layer])
        list.loop = false;

    u8g2.setDrawColor(1);

    // Progress bar
    u8g2.drawHLine(DISP_W - LIST_BAR_W, 0, LIST_BAR_W);
    u8g2.drawHLine(DISP_W - LIST_BAR_W, DISP_H - 1, LIST_BAR_W);
    u8g2.drawVLine(DISP_W - ceil((float)LIST_BAR_W / 2), 0, DISP_H);
    u8g2.drawBox(DISP_W - LIST_BAR_W, 0, LIST_BAR_W, list.bar_y);

    // List text
    if (!ui.init)
    {
        for (int i = 0; i < ui.num[ui_index]; ++i)
        {
            if (ui.param[LIST_UFD])
                list.temp = i * list.y - LIST_LINE_H * ui.select[ui.layer] + list.box_y_trg[ui.layer];
            else
                list.temp = (i - ui.select[ui.layer]) * list.y + list.box_y_trg[ui.layer];

            List_Draw_Text_And_CheckBox(arr, i);
        }
        if (list.y == list.y_trg)
        {
            ui.init = true;
            list.y = list.y_trg = -LIST_LINE_H * ui.select[ui.layer] + list.box_y_trg[ui.layer];
        }
    }
    else
    {
        for (int i = 0; i < ui.num[ui_index]; ++i)
        {
            list.temp = LIST_LINE_H * i + list.y;
            List_Draw_Text_And_CheckBox(arr, i);
        }
    }

    // Selection box
    u8g2.setDrawColor(2);
    u8g2.drawRBox(0, list.box_y, list.box_x, LIST_LINE_H, LIST_BOX_R);

    // Dark mode mask
    if (!ui.param[DARK_MODE])
    {
        u8g2.drawBox(0, 0, DISP_W, DISP_H);
        if (ui.index == M_WINDOW || ui.index == M_VOLT)
            u8g2.drawBox(0, 0, DISP_W, DISP_H);
    }
}

// ---- Volt show ----

void OledUI::Volt_Show()
{
    List_Show(volt_menu, M_VOLT);

    Animation(volt.text_bg_r_track, volt.text_bg_r, volt.text_bg_r_trg, TAG_ANI);

    u8g2.setDrawColor(1);

    // Waveform frame
    u8g2.drawFrame(WAVE_BOX_L_S, 0, WAVE_BOX_W, WAVE_BOX_H);
    u8g2.drawFrame(WAVE_BOX_L_S + 1, 1, WAVE_BOX_W - 2, WAVE_BOX_H - 2);

    if (list.box_y == list.box_y_trg[ui.layer] && list.y == list.y_trg)
    {
        for (int i = 0; i < WAVE_SAMPLE * WAVE_W; i++)
            // volt.ch0_adc[i] = volt.val = analogRead(analog_pin[ui.select[ui.layer]]);
            volt.ch0_adc[i] = volt.val = random(500, 1023);

        for (int i = 1; i < WAVE_W - 1; i++)
        {
            volt.ch0_wave[i] = map(volt.ch0_adc[int(5 * i)], 0, 4095, WAVE_MAX, WAVE_MIN);
            u8g2.drawLine(WAVE_L + i - 1, WAVE_U + volt.ch0_wave[i - 1],
                          WAVE_L + i,       WAVE_U + volt.ch0_wave[i]);
        }
    }

    // Voltage value
    u8g2.setFontDirection(0);
    u8g2.setFont(VOLT_FONT);
    u8g2.setCursor(39, DISP_H - 6);
    u8g2.print(volt.val / 4096.0f * 3.3f);
    u8g2.print("V");

    // Text background + dark mode
    u8g2.setDrawColor(2);
    u8g2.drawBox(VOLT_TEXT_BG_L_S, DISP_H - VOLT_TEXT_BG_H, volt.text_bg_r, VOLT_TEXT_BG_H);

    if (!ui.param[DARK_MODE])
        u8g2.drawBox(0, 0, DISP_W, DISP_H);
}

// ---- Window show ----

void OledUI::Window_Show()
{
    List_Show(win.bg, win.bg_index);

    if (ui.param[WIN_BOK])
    {
        uint8_t* buf = OledDriver::GetBufferPtr();
        uint16_t len  = OledDriver::GetBufferLen();
        for (uint16_t i = 0; i < len; ++i)
            buf[i] = buf[i] & (i % 2 == 0 ? 0x55 : 0xAA);
    }

    u8g2.setFont(WIN_FONT);
    win.bar_trg = (float)(*win.value - win.min) / (float)(win.max - win.min) * (WIN_BAR_W - 4);

    Animation(win.bar_track, win.bar, win.bar_trg, WIN_ANI);
    Animation(win.y_track,   win.y,   win.y_trg,   WIN_ANI);

    win.l = (DISP_W - WIN_W) / 2;
    win.u = (DISP_H - WIN_H) / 2;

    u8g2.setDrawColor(0);
    u8g2.drawRBox(win.l, (int16_t)win.y, WIN_W, WIN_H, 2);
    u8g2.setDrawColor(1);
    u8g2.drawRFrame(win.l, (int16_t)win.y, WIN_W, WIN_H, 2);
    u8g2.drawRFrame(win.l + 5, (int16_t)win.y + 20, WIN_BAR_W, WIN_BAR_H, 1);
    u8g2.drawBox(win.l + 7, (int16_t)win.y + 22, win.bar, WIN_BAR_H - 4);
    u8g2.setCursor(win.l + 5, (int16_t)win.y + 14);
    u8g2.print(win.title);
    u8g2.setCursor(win.l + 78, (int16_t)win.y + 14);
    u8g2.print(*win.value);

    // Apply brightness immediately on "Disp Bri"
    if (!strcmp(win.title, "Disp Bri"))
        u8g2.setContrast(ui.param[DISP_BRI]);

    u8g2.setDrawColor(2);
    if (!ui.param[DARK_MODE])
        u8g2.drawBox(0, 0, DISP_W, DISP_H);
}

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
