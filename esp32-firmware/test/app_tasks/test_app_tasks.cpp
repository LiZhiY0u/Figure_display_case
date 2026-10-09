#include <Arduino.h>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "App/AppTasks.h"
SerialFake Serial;
namespace { int fail = 0, created = 0; std::vector<TaskHandle_t> deleted; }
int analogRead(int pin) { assert(pin == 27); return 123; }
void vTaskDelay(unsigned) {}
void vTaskDelete(TaskHandle_t handle) { assert(handle); deleted.push_back(handle); }
namespace ENCODER { void Encoder_Init() {} void Encoder_Update() {} }
namespace OledUI { void Init() {} void Task() {} }
void rainbow(int) {}
BaseType_t xTaskCreatePinnedToCore(TaskFunction_t fn, const char* name, uint32_t stack,
    void* parameter, unsigned priority, TaskHandle_t* handle, int core)
{
    const char* names[] = {"start_task", "Task_ws2812", "Task_key", "Task_oled"};
    const uint32_t stacks[] = {4096,2048,2048,10240};
    assert(created < 4 && fn && !parameter && core == 1);
    assert(std::strcmp(name,names[created]) == 0 && stack == stacks[created]);
    assert(priority == (created == 0 ? 2u : 4u));
    ++created;
    if (created == fail) return 0;
    *handle = reinterpret_cast<void*>(static_cast<uintptr_t>(created));
    return pdPASS;
}
int main(int argc, char** argv)
{
    fail = argc > 1 ? std::atoi(argv[1]) : 0;
    assert(AppTasks::Start() == (fail == 0));
    assert(created == (fail ? fail : 4));
    assert(deleted.size() == (fail ? static_cast<unsigned>(fail-1) : 0u));
    for (int i=1; i<fail; ++i)
        assert(deleted[static_cast<unsigned>(i-1)] == reinterpret_cast<void*>(static_cast<uintptr_t>(i)));
    assert(AppTasks::Start() == (fail == 0));
    assert(created == (fail ? fail : 4));
}
