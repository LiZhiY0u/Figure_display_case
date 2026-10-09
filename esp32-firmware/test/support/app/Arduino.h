#pragma once
#include <cstdint>
using TaskHandle_t = void*;
using TaskFunction_t = void (*)(void*);
using BaseType_t = int;
constexpr int pdPASS = 1;
#define configTICK_RATE_HZ 1000
#define pdMS_TO_TICKS(ms) (ms)
struct SerialFake { void println(const char*) {} void print(const char*) {} };
extern SerialFake Serial;
int analogRead(int);
void vTaskDelay(unsigned);
void vTaskDelete(TaskHandle_t);
BaseType_t xTaskCreatePinnedToCore(TaskFunction_t, const char*, uint32_t, void*, unsigned, TaskHandle_t*, int);
