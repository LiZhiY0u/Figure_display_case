#include "HAL/HAL.h"
#include "Configs/Version.h"

#define DISP_HOR_RES CONFIG_SCREEN_HOR_RES
#define DISP_VER_RES CONFIG_SCREEN_VER_RES
#define DISP_BUF_SIZE CONFIG_SCREEN_BUFFER_SIZE
// extern lv_color_t *lv_disp_buf_p;

void HAL::Init()
{
    Serial.begin(115200);
    Serial.println(VERSION_FIRMWARE_NAME);
    Serial.println("Version: " VERSION_SOFTWARE);
    Serial.println("Author: " VERSION_AUTHOR_NAME);
    Serial.println("DATA: " VERSION_DATA);

    pinMode(CONFIG_BAT_CHG_DET_PIN, INPUT);

    /*电源使能保持*/
    Serial.println("Power: Waiting...");
    pinMode(CONFIG_POWER_EN_PIN, OUTPUT);
    digitalWrite(CONFIG_POWER_EN_PIN, LOW);
    delay(100);
    digitalWrite(CONFIG_POWER_EN_PIN, HIGH);
    Serial.println("Power: ON");

    /*电池检测*/
    pinMode(CONFIG_BAT_DET_PIN, INPUT);
    pinMode(CONFIG_BAT_CHG_DET_PIN, INPUT_PULLUP);

    Power_SetAutoLowPowerTimeout(60);
    Power_HandleTimeUpdate();
    Power_SetAutoLowPowerEnable(false);
}

void HAL::Update()
{
    // HAL::Power_Update();
    // HAL::Encoder_Update();
    // HAL::Audio_Update();
    // HAL::IMU_Update();
    // HAL::BT_Update();
    // __IntervalExecute(HAL::SD_Update(), 500);
}
