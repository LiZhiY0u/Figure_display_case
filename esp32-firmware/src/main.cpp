/**
 * @file   main.cpp
 * @author LZY (original: congsir)
 * @brief  Controller firmware - main entry point (refactored OLED version)
 *
 * Uses the modularized OledUI library (OledDriver + OledUI + OledMenus).
 */

#include <Arduino.h>
#include "HAL/HAL.h"
#include "Encoder/HAL_Encoder.h"
#include "Input/BleRemote.h"
#include "RGB.h"
#include "TB6612.h"
#include "OledUI/OledDriver.h"
#include "OledUI/OledUI.h"
#include "main.h"

// ===========================
// Task handles
// ===========================
TaskHandle_t Task_controller_Handle;
TaskHandle_t Task_start_Handle;
TaskHandle_t Task_ws2812_Handle;
TaskHandle_t Task_motor_Handle;
TaskHandle_t Task_key_Handle;
TaskHandle_t Task_oled_Handle;

// ===========================
// Queues
// ===========================
QueueHandle_t start_Queue;
QueueHandle_t iot_control_Queue;
QueueHandle_t key_Queue;
QueueHandle_t oled_Queue;

// ===========================
// Global objects
// ===========================
uint16_t adc = 0;

Adafruit_NeoPixel RGB3 = Adafruit_NeoPixel(RGB_3_NUM, RGB_PIN_3, NEO_GRB + NEO_KHZ800);
Motor motor(MOTOR_PIN_1, MOTOR_PIN_2, MOTOR_PWM, 1000, 1);

// key_en is defined in HAL_Encoder.cpp (declared in HAL_Encoder.h)
// gamepad_axis is defined in Controller.cpp (declared in Controller.h)

// ===========================
// Task: Start / monitoring
// ===========================
void Task_start(void* pvParameters)
{
    while (1)
    {
        adc = analogRead(CONFIG_BAT_DET_PIN);
        vTaskDelay(1000);
    }
}

// ===========================
// Task: Encoder input
// ===========================
void Task_key(void* pvParameters)
{
    ENCODER::Encoder_Init();
    while (1)
    {
        ENCODER::Encoder_Update();
        vTaskDelay(10);
    }
}

// ===========================
// Task: OLED UI
// ===========================
void Task_oled(void* pvParameters)
{
    // Initialize the entire UI subsystem
    OledUI::Init();

    while (1)
    {
        // Key scan + UI render + sendBuffer
        OledUI::Task();
        vTaskDelay(1);
    }
}

// ===========================
// Task: WS2812 RGB strip
// ===========================
void Task_ws2812(void* pvParameters)
{
    RGB3.begin();
    RGB3.show();
    RGB3.setBrightness(255);

    while (1)
    {
        rainbow(5);
    }
}

// ===========================
// Task: Motor driver
// ===========================
void Task_motor(void* pvParameters)
{
    vTaskDelay(1000);
    while (1)
    {
        vTaskDelay(20);
    }
}

// ===========================
// Task: Periodic HAL update
// ===========================
void Task_update(void* pvParameters)
{
    while (1)
    {
        HAL::Update();
        vTaskDelay(20);
    }
}

// ===========================
// Task: Gamepad controller
// ===========================
// void Controller_start(void* pvParameters)
// {
//     Controller_init();
//     while (1)
//     {
//         Controller_loop();
//         vTaskDelay(100);
//     }
// }

// ===========================
// Arduino setup
// ===========================
void setup()
{
    HAL::Init();
    BleRemote::Init();

    xTaskCreatePinnedToCore(
        Task_start, "start_task", 4096, NULL, 2, &Task_start_Handle, ESP32_RUNNING_CORE);

    xTaskCreatePinnedToCore(
        Task_ws2812, "Task_ws2812", 2048, NULL, 4, &Task_ws2812_Handle, ESP32_RUNNING_CORE);

    xTaskCreatePinnedToCore(
        Task_key, "Task_key", 2048, NULL, 4, &Task_key_Handle, ESP32_RUNNING_CORE);

    xTaskCreatePinnedToCore(
        Task_oled, "Task_oled", 1024 * 10, NULL, 4, &Task_oled_Handle, ESP32_RUNNING_CORE);

    // xTaskCreatePinnedToCore(
    //     Controller_start, "Controller_start", 1024 * 10, NULL, 2, &Task_controller_Handle, 1);
}

// ===========================
// Arduino main loop
// ===========================
void loop()
{
    BleRemote::Update();
    vTaskDelay(10);
}

