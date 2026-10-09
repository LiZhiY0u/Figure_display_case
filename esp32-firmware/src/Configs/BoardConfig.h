#pragma once
#define KEY_A 14
#define KEY_B 33
#define KEY_EN 32
#define OLED_SCL 22
#define OLED_SDA 21
#define CONFIG_BAT_DET_PIN 27
#define CONFIG_BAT_CHG_DET_PIN 25
#define CONFIG_POWER_EN_PIN 26
namespace BoardConfig {
constexpr int RgbPin = 19, RgbCount = 10;
constexpr int MotorIn1 = 16, MotorIn2 = 4, MotorPwm = 17;
constexpr int MotorFrequency = 1000, MotorChannel = 1;
}
