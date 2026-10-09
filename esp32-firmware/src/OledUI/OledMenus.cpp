/**
 * @file   OledMenus.cpp
 * @author LZY (original author: RQNG, WouoUI)
 * @brief  OLED menu definitions and icon bitmap data
 */

#include "OledMenus.h"

// ===========================
// Main menu (tile icons)
// ===========================
const M_SELECT main_menu[] = {
    {"Light", MenuItemType::Action},
    {"Editor", MenuItemType::Action},
    {"Volt", MenuItemType::Action},
    {"Setting", MenuItemType::Action},
};
const uint8_t main_menu_count = sizeof(main_menu) / sizeof(M_SELECT);

// ===========================
// Editor menu (list)
// ===========================
const M_SELECT editor_menu[] = {
    {"[ Editor ]", MenuItemType::Back},
    {"- Function 0", MenuItemType::Action},
    {"- Function 1", MenuItemType::Action},
    {"- Function 2", MenuItemType::Action},
    {"- Function 3", MenuItemType::Action},
    {"- Function 4", MenuItemType::Action},
    {"- Function 5", MenuItemType::Action},
    {"- Function 6", MenuItemType::Action},
    {"- Function 7", MenuItemType::Action},
    {"- Function 8", MenuItemType::Action},
    {"- Function 9", MenuItemType::Action},
    {"- Knob", MenuItemType::Action},
};
const uint8_t editor_menu_count = sizeof(editor_menu) / sizeof(M_SELECT);

// ===========================
// Knob menu (list)
// ===========================
const M_SELECT knob_menu[] = {
    {"[ Knob ]", MenuItemType::Back},
    {"# Rotate Func", MenuItemType::RotateBinding, 0},
    {"$ Press Func", MenuItemType::PressBinding, 1},
};
const uint8_t knob_menu_count = sizeof(knob_menu) / sizeof(M_SELECT);

// ===========================
// Knob rotate function menu (list)
// ===========================
const M_SELECT krf_menu[] = {
    {"[ Rotate Function ]", MenuItemType::Back},
    {"--------------------------", MenuItemType::Separator},
    {"= Disable", MenuItemType::Choice, 2},
    {"--------------------------", MenuItemType::Separator},
    {"= Volume", MenuItemType::Choice, 4},
    {"= Brightness", MenuItemType::Choice, 5},
    {"--------------------------", MenuItemType::Separator},
};
const uint8_t krf_menu_count = sizeof(krf_menu) / sizeof(M_SELECT);

// ===========================
// Knob press function menu (list)
// ===========================
const M_SELECT kpf_menu[] = {
    {"[ Press Function ]", MenuItemType::Back},
    {"--------------------------", MenuItemType::Separator},
    {"= Disable", MenuItemType::Choice, 2},
    {"--------------------------", MenuItemType::Separator},
    {"= A", MenuItemType::Choice, 4}, {"= B", MenuItemType::Choice, 5}, {"= C", MenuItemType::Choice, 6}, {"= D", MenuItemType::Choice, 7}, {"= E", MenuItemType::Choice, 8}, {"= F", MenuItemType::Choice, 9},
    {"= G", MenuItemType::Choice, 10}, {"= H", MenuItemType::Choice, 11}, {"= I", MenuItemType::Choice, 12}, {"= J", MenuItemType::Choice, 13}, {"= K", MenuItemType::Choice, 14}, {"= L", MenuItemType::Choice, 15},
    {"= M", MenuItemType::Choice, 16}, {"= N", MenuItemType::Choice, 17}, {"= O", MenuItemType::Choice, 18}, {"= P", MenuItemType::Choice, 19}, {"= Q", MenuItemType::Choice, 20}, {"= R", MenuItemType::Choice, 21},
    {"= S", MenuItemType::Choice, 22}, {"= T", MenuItemType::Choice, 23}, {"= U", MenuItemType::Choice, 24}, {"= V", MenuItemType::Choice, 25}, {"= W", MenuItemType::Choice, 26}, {"= X", MenuItemType::Choice, 27},
    {"= Y", MenuItemType::Choice, 28}, {"= Z", MenuItemType::Choice, 29},
    {"--------------------------", MenuItemType::Separator},
    {"= 0", MenuItemType::Choice, 31}, {"= 1", MenuItemType::Choice, 32}, {"= 2", MenuItemType::Choice, 33}, {"= 3", MenuItemType::Choice, 34}, {"= 4", MenuItemType::Choice, 35},
    {"= 5", MenuItemType::Choice, 36}, {"= 6", MenuItemType::Choice, 37}, {"= 7", MenuItemType::Choice, 38}, {"= 8", MenuItemType::Choice, 39}, {"= 9", MenuItemType::Choice, 40},
    {"--------------------------", MenuItemType::Separator},
    {"= Esc", MenuItemType::Choice, 42},
    {"= F1", MenuItemType::Choice, 43}, {"= F2", MenuItemType::Choice, 44}, {"= F3", MenuItemType::Choice, 45}, {"= F4", MenuItemType::Choice, 46}, {"= F5", MenuItemType::Choice, 47}, {"= F6", MenuItemType::Choice, 48},
    {"= F7", MenuItemType::Choice, 49}, {"= F8", MenuItemType::Choice, 50}, {"= F9", MenuItemType::Choice, 51}, {"= F10", MenuItemType::Choice, 52}, {"= F11", MenuItemType::Choice, 53}, {"= F12", MenuItemType::Choice, 54},
    {"--------------------------", MenuItemType::Separator},
    {"= Left Ctrl", MenuItemType::Choice, 56}, {"= Left Shift", MenuItemType::Choice, 57}, {"= Left Alt", MenuItemType::Choice, 58}, {"= Left Win", MenuItemType::Choice, 59},
    {"= Right Ctrl", MenuItemType::Choice, 60}, {"= Right Shift", MenuItemType::Choice, 61}, {"= Right Alt", MenuItemType::Choice, 62}, {"= Right Win", MenuItemType::Choice, 63},
    {"--------------------------", MenuItemType::Separator},
    {"= Caps Lock", MenuItemType::Choice, 65}, {"= Backspace", MenuItemType::Choice, 66}, {"= Return", MenuItemType::Choice, 67},
    {"= Insert", MenuItemType::Choice, 68}, {"= Delete", MenuItemType::Choice, 69}, {"= Tab", MenuItemType::Choice, 70},
    {"--------------------------", MenuItemType::Separator},
    {"= Home", MenuItemType::Choice, 72}, {"= End", MenuItemType::Choice, 73}, {"= Page Up", MenuItemType::Choice, 74}, {"= Page Down", MenuItemType::Choice, 75},
    {"--------------------------", MenuItemType::Separator},
    {"= Up Arrow", MenuItemType::Choice, 77}, {"= Down Arrow", MenuItemType::Choice, 78}, {"= Left Arrow", MenuItemType::Choice, 79}, {"= Right Arrow", MenuItemType::Choice, 80},
    {"--------------------------", MenuItemType::Separator},
};
const uint8_t kpf_menu_count = sizeof(kpf_menu) / sizeof(M_SELECT);

// ===========================
// Voltage monitor menu (list)
// ===========================
const M_SELECT volt_menu[] = {
    {"A0", MenuItemType::Action}, {"A1", MenuItemType::Action}, {"A2", MenuItemType::Action}, {"A3", MenuItemType::Action}, {"A4", MenuItemType::Action},
    {"A5", MenuItemType::Action}, {"A6", MenuItemType::Action}, {"A7", MenuItemType::Action},
    {"B0", MenuItemType::Action}, {"B1", MenuItemType::Action},
};
const uint8_t volt_menu_count = sizeof(volt_menu) / sizeof(M_SELECT);

// ===========================
// Settings menu (list with checkboxes)
// ===========================
const M_SELECT setting_menu[] = {
    {"[ Setting ]", MenuItemType::Back},
    {"~ Disp Bri", MenuItemType::Value, 0},
    {"~ Tile Ani", MenuItemType::Value, 1},
    {"~ List Ani", MenuItemType::Value, 2},
    {"~ Win Ani", MenuItemType::Value, 3},
    {"~ Spot Ani", MenuItemType::Value, 4},
    {"~ Tag Ani", MenuItemType::Value, 5},
    {"~ Fade Ani", MenuItemType::Value, 6},
    {"~ Btn SPT", MenuItemType::Value, 7},
    {"~ Btn LPT", MenuItemType::Value, 8},
    {"+ T Ufd Fm Scr", MenuItemType::Toggle, 9},
    {"+ L Ufd Fm Scr", MenuItemType::Toggle, 10},
    {"+ T Loop Mode", MenuItemType::Toggle, 11},
    {"+ L Loop Mode", MenuItemType::Toggle, 12},
    {"+ Win Bokeh Bg", MenuItemType::Toggle, 13},
    {"+ Knob Rot Dir", MenuItemType::Toggle, 14},
    {"+ Dark Mode", MenuItemType::Toggle, 15},
    {"- Anim Curve", MenuItemType::Action},
    {"- [ About ]", MenuItemType::Action},
};
const uint8_t setting_menu_count = sizeof(setting_menu) / sizeof(M_SELECT);

// ===========================
// Runtime animation curve menu (list)
// ===========================
const M_SELECT animation_curve_menu[] = {
    {"[ Anim Curve ]", MenuItemType::Back},
    {"= Linear", MenuItemType::Choice, 1},
    {"= Out Cubic", MenuItemType::Choice, 2},
    {"= InOut Cubic", MenuItemType::Choice, 3},
    {"= Out Back", MenuItemType::Choice, 4},
};
const uint8_t animation_curve_menu_count =
    sizeof(animation_curve_menu) / sizeof(M_SELECT);

// ===========================
// About menu (list)
// ===========================
const M_SELECT about_menu[] = {
    {"[ WouoUI ]", MenuItemType::Back},
    {"- Version: v2.3", MenuItemType::Info},
    {"- Board: STM32F103", MenuItemType::Info},
    {"- Ram: 20k", MenuItemType::Info},
    {"- Flash: 64k", MenuItemType::Info},
    {"- Freq: 72Mhz", MenuItemType::Info},
    {"- Creator: RQNG", MenuItemType::Info},
    {"- Bili UID: 9182439", MenuItemType::Info},
};
const uint8_t about_menu_count = sizeof(about_menu) / sizeof(M_SELECT);

// ===========================
// Main menu icon bitmaps (PROGMEM)
// Each icon: 30 wide x 30 tall @ 1bpp = 120 bytes
// ===========================
PROGMEM const uint8_t main_icon_pic[][120] = {

    // Icon 0: Light/bulb
    {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF9, 0xFF, 0xFF, 0xFB, 0xFC,
    0xFF, 0xFF, 0xF3, 0x3C, 0xFF, 0xFF, 0x07, 0x98, 0xFF, 0xFF, 0xC7, 0xC3, 0xFF, 0x9F, 0xF3,
    0xCF, 0xFF, 0x3F, 0xF9, 0xDF, 0xFF, 0x7F, 0xCC, 0xDF, 0xFE, 0xFF, 0x9E, 0x11, 0xFE, 0xFF,
    0xBE, 0x9C, 0xFF, 0x0F, 0xBE, 0xDE, 0xFF, 0xEF, 0xBE, 0xCE, 0xFF, 0xFF, 0xBE, 0xCE, 0xFF,
    0xFF, 0xBC, 0x0E, 0xFC, 0x7F, 0xF9, 0x63, 0xFF, 0x1F, 0xE3, 0xE3, 0xFF, 0xFF, 0x01, 0xE8,
    0xFF, 0xFF, 0xCC, 0xCD, 0xFF, 0xFF, 0x8E, 0xFD, 0xFF, 0x7F, 0x1E, 0xFC, 0xFF, 0xFF, 0xDF,
    0xFD, 0xFF, 0xFF, 0x9F, 0xFD, 0xFF, 0xFF, 0x1F, 0xFC, 0xFF, 0xFF, 0x3F, 0xFE, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    },

    // Icon 1: Editor (document)
    {
    0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xF9, 0xE7,
    0x3F, 0xFF, 0xF9, 0xE7, 0x3F, 0xFF, 0xF9, 0xE7, 0x3F, 0xFF, 0xF0, 0xE7, 0x3F, 0x7F, 0xE0,
    0xE7, 0x3F, 0x7F, 0xE0, 0xC3, 0x3F, 0x7F, 0xE0, 0xC3, 0x3F, 0x7F, 0xE0, 0xC3, 0x3F, 0x7F,
    0xE0, 0xE7, 0x3F, 0xFF, 0xF0, 0xE7, 0x3F, 0xFF, 0xF9, 0xE7, 0x3F, 0xFF, 0xF9, 0xE7, 0x3F,
    0xFF, 0xF9, 0xE7, 0x3F, 0xFF, 0xF9, 0xE7, 0x3F, 0xFF, 0xF9, 0xC3, 0x3F, 0xFF, 0xF9, 0x81,
    0x3F, 0xFF, 0xF0, 0x81, 0x3F, 0xFF, 0xF0, 0x81, 0x3F, 0xFF, 0xF0, 0x81, 0x3F, 0xFF, 0xF9,
    0x81, 0x3F, 0xFF, 0xF9, 0xC3, 0x3F, 0xFF, 0xF9, 0xE7, 0x3F, 0xFF, 0xF9, 0xE7, 0x3F, 0xFF,
    0xF9, 0xE7, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F,
    },

    // Icon 2: Volt/meter
    {
    0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F, 0xEF, 0xFF, 0xFF, 0x3F, 0xC7, 0xFF, 0xFF,
    0x3F, 0xC7, 0xF3, 0xFF, 0x3F, 0x83, 0xC0, 0xFF, 0x3F, 0xEF, 0xCC, 0xFF, 0x3F, 0x6F, 0x9E,
    0xFF, 0x3F, 0x6F, 0x9E, 0xFF, 0x3F, 0x2F, 0x3F, 0xFF, 0x3F, 0x2F, 0x3F, 0xFF, 0x3F, 0x8F,
    0x7F, 0xFE, 0x3F, 0x8F, 0x7F, 0xFE, 0x39, 0x8F, 0x7F, 0xFE, 0x39, 0xCF, 0xFF, 0xFC, 0x3C,
    0xCF, 0xFF, 0xFC, 0x3C, 0xEF, 0xFF, 0xFC, 0x3C, 0xEF, 0xFF, 0x79, 0x3E, 0xEF, 0xFF, 0x79,
    0x3E, 0xEF, 0xFF, 0x33, 0x3F, 0xEF, 0xFF, 0x33, 0x3F, 0xEF, 0xFF, 0x87, 0x3F, 0xEF, 0xFF,
    0xCF, 0x3F, 0xEF, 0xFF, 0x7F, 0x3E, 0xEF, 0xFF, 0x7F, 0x38, 0x0F, 0x00, 0x00, 0x30, 0xFF,
    0xFF, 0x7F, 0x38, 0xFF, 0xFF, 0x7F, 0x3E, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F,
    },

    // Icon 3: Settings (gear)
    {
    0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF,
    0x3F, 0xFF, 0x1F, 0xFE, 0x3F, 0xFF, 0x1F, 0xFE, 0x3F, 0xFF, 0x0C, 0xCC, 0x3F, 0x7F, 0x00,
    0x80, 0x3F, 0x3F, 0x00, 0x00, 0x3F, 0x3F, 0xE0, 0x01, 0x3F, 0x7F, 0xF8, 0x87, 0x3F, 0x7F,
    0xFC, 0x8F, 0x3F, 0x3F, 0xFC, 0x0F, 0x3F, 0x0F, 0x3E, 0x1F, 0x3C, 0x0F, 0x1E, 0x1E, 0x3C,
    0x0F, 0x1E, 0x1E, 0x3C, 0x0F, 0x3E, 0x1F, 0x3C, 0x3F, 0xFC, 0x0F, 0x3F, 0x7F, 0xFC, 0x8F,
    0x3F, 0x7F, 0xF8, 0x87, 0x3F, 0x3F, 0xE0, 0x01, 0x3F, 0x3F, 0x00, 0x00, 0x3F, 0x7F, 0x00,
    0x80, 0x3F, 0xFF, 0x0C, 0xCC, 0x3F, 0xFF, 0x1F, 0xFE, 0x3F, 0xFF, 0x1F, 0xFE, 0x3F, 0xFF,
    0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F,
    },
};
const uint8_t main_icon_count = sizeof(main_icon_pic) / 120;

// ===========================
// Light 灯带菜单
// ===========================

// 灯带主菜单
const M_SELECT light_main_menu[] = {
    {"[ Light - WIP ]", MenuItemType::Back}, // 0
    {"- Brightness", MenuItemType::Action},    // 1 (入口将在硬件控制接通后启用)
    {"- Color", MenuItemType::Action},         // 2 (入口将在硬件控制接通后启用)
    {"- Effect", MenuItemType::Action},        // 3 (入口将在硬件控制接通后启用)
};
const uint8_t light_main_menu_count = sizeof(light_main_menu) / sizeof(M_SELECT);

// 亮度调节菜单
extern uint8_t light_brightness;  // ← 由用户实现: 全局亮度值 (0-255)
const M_SELECT light_bri_menu[] = {
    {"[ Brightness ]", MenuItemType::Back},
    {"~ Bri:", MenuItemType::Value, 0},         // 亮度值；处理器负责打开滑动条弹窗
};
const uint8_t light_bri_menu_count = sizeof(light_bri_menu) / sizeof(M_SELECT);

// 颜色选择菜单 (预留RGB独立滑条，或预定义色板)
extern uint8_t light_color_r;  // ← 由用户实现
extern uint8_t light_color_g;  // ← 由用户实现
extern uint8_t light_color_b;  // ← 由用户实现
const M_SELECT light_color_menu[] = {
    {"[ Color ]", MenuItemType::Back},
    {"= Red", MenuItemType::Choice, 1},           // 0
    {"= Green", MenuItemType::Choice, 2},         // 1
    {"= Blue", MenuItemType::Choice, 3},          // 2
    {"= White", MenuItemType::Choice, 4},         // 3
    {"= Custom", MenuItemType::Choice, 5},        // 4  预留: RGB滑条或取色器
};
const uint8_t light_color_menu_count = sizeof(light_color_menu) / sizeof(M_SELECT);

// 预定义效果菜单 (预留，用户实现效果函数)
// light_effect_id: 0=Static 1=Breath 2=Rainbow 3=Wave 4=Strobe 5=Comet 6=Sparkle
extern const char* light_effect_name[];  // 效果名称数组指针，由用户实现
const M_SELECT light_effect_menu[] = {
    {"[ Effect ]", MenuItemType::Back},
    {"= Static", MenuItemType::Choice, 1},        // 0  静态单色
    {"= Breath", MenuItemType::Choice, 2},        // 1  呼吸效果
    {"= Rainbow", MenuItemType::Choice, 3},       // 2  彩虹流水
    {"= Wave", MenuItemType::Choice, 4},          // 3  波浪效果
    {"= Strobe", MenuItemType::Choice, 5},        // 4  频闪效果
    {"= Comet", MenuItemType::Choice, 6},         // 5  彗星拖尾
    {"= Sparkle", MenuItemType::Choice, 7},        // 6  闪烁星点
};
const uint8_t light_effect_menu_count = sizeof(light_effect_menu) / sizeof(M_SELECT);
