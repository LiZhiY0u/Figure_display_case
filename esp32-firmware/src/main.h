/*
 * @Descripttion:
 * @version:
 * @Author: congsir
 * @Date: 2022-05-15 17:02:59
 * @LastEditors: wenzheng 565402462@qq.com
 * @LastEditTime: 2022-07-24 00:13:30
 */
#pragma once

#include <Arduino.h>
#include "HAL/HAL.h"
#include "Encoder/HAL_Encoder.h"
#include "RGB.h"
#include "TB6612.h"
// #include "OLED/Oled.h"
// #include "Controller/Controller.hpp"

// Shared globals used across modules
// key_en is declared in HAL_Encoder.h (defined in HAL_Encoder.cpp)
extern uint8_t gamepad_axis;

#define ENABLE_BLE_KEY_BOARD
#define ENABLE_WS2812_LED
// #define ESP32C3_ENABLE

#define LVGL_RUNNING_CORE 0  // 内核选择   LVGL用核心0  其他用核心1
#define ESP32_RUNNING_CORE 1 // 内核选择   LVGL用核心0  其他用核心1

// #define RGB_PIN_1 5  // RGB1引脚
#define RGB_PIN_2 18 // RGB2引脚
#define RGB_PIN_3 19 // RGB3引脚

#define RGB_1_NUM 10 // RGB数量
#define RGB_2_NUM 10
#define RGB_3_NUM 10

#define MOTOR_PWM 17
#define MOTOR_PIN_1 16
#define MOTOR_PIN_2 4

#define OLED_SCL 22
#define OLED_SDA 21

// 按键引脚
#define KEY_A 14
#define KEY_B 33
#define KEY_EN 32

#define WS2812_NUM_1 4 // 定义led个数，决定数组长度
#define WS2812_NUM_2 4 // 定义led个数，决定数组长度
#define WS2812_NUM_3 4 // 定义led个数，决定数组长度
