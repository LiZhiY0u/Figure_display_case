/**
 * @file   OledMenus.h
 * @author LZY (original author: RQNG, WouoUI)
 * @brief  OLED menu definitions and icon data
 *
 * All menu structures (M_SELECT arrays) and bitmap icon data.
 * No logic here - pure data.
 */

#ifndef __OLED_MENUS_H
#define __OLED_MENUS_H

#include <Arduino.h>
#include "MenuItem.h"

// ===========================
// Menu arrays (extern in .cpp)
// ===========================

// Main menu - tile icons
extern const M_SELECT main_menu[];
extern const uint8_t main_menu_count;

// Editor menu - list
extern const M_SELECT editor_menu[];
extern const uint8_t editor_menu_count;

// Knob menu - list
extern const M_SELECT knob_menu[];
extern const uint8_t knob_menu_count;

// Knob rotate function - list
extern const M_SELECT krf_menu[];
extern const uint8_t krf_menu_count;

// Knob press function - list
extern const M_SELECT kpf_menu[];
extern const uint8_t kpf_menu_count;

// Voltage monitor - list
extern const M_SELECT volt_menu[];
extern const uint8_t volt_menu_count;

// Settings - list
extern const M_SELECT setting_menu[];
extern const uint8_t setting_menu_count;

// Runtime animation curve selection - list
extern const M_SELECT animation_curve_menu[];
extern const uint8_t animation_curve_menu_count;

// About - list
extern const M_SELECT about_menu[];
extern const uint8_t about_menu_count;

// ---- Light 灯带 ----
// 灯带主菜单
extern const M_SELECT light_main_menu[];
extern const uint8_t light_main_menu_count;

// 亮度调节菜单
extern const M_SELECT light_bri_menu[];
extern const uint8_t light_bri_menu_count;

// 颜色选择菜单
extern const M_SELECT light_color_menu[];
extern const uint8_t light_color_menu_count;

// 预定义效果菜单
extern const M_SELECT light_effect_menu[];
extern const uint8_t light_effect_menu_count;

// ===========================
// Icon bitmap data (PROGMEM)
// ===========================

// Main menu icons - 4 icons, each 120 bytes (30x30 @ 1bpp)
extern const uint8_t main_icon_pic[][120];
extern const uint8_t main_icon_count;

#endif // __OLED_MENUS_H
