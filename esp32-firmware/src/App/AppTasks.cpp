#include "AppTasks.h"
#include <Arduino.h>
#include "Configs/BoardConfig.h"
#include "Configs/TaskConfig.h"
#include "Encoder/HAL_Encoder.h"
#include "OledUI/OledUI.h"
#include "RGB.h"
#include "TB6612.h"
static_assert(configTICK_RATE_HZ == 1000, "Task migration requires original 1ms RTOS tick");
// Existing RGB implementation references this statically owned object.
Adafruit_NeoPixel RGB3(BoardConfig::RgbCount, BoardConfig::RgbPin, NEO_GRB + NEO_KHZ800);
// Preserve the original constructor's GPIO/PWM setup; no motor task is enabled.
Motor motor(BoardConfig::MotorIn1, BoardConfig::MotorIn2, BoardConfig::MotorPwm,
            BoardConfig::MotorFrequency, BoardConfig::MotorChannel);
namespace {
TaskHandle_t handles[4] = {};
enum class State { NotStarted, Failed, Ready };
State state = State::NotStarted;
void Sampling(void*) {
    for (;;) { (void)analogRead(CONFIG_BAT_DET_PIN); vTaskDelay(pdMS_TO_TICKS(TaskConfig::SamplingMs)); }
}
void Lighting(void*) {
    RGB3.begin(); RGB3.show(); RGB3.setBrightness(255);
    for (;;) rainbow(5);
}
void Encoder(void*) {
    ENCODER::Encoder_Init();
    for (;;) { ENCODER::Encoder_Update(); vTaskDelay(pdMS_TO_TICKS(TaskConfig::EncoderMs)); }
}
void Display(void*) {
    OledUI::Init();
    for (;;) { OledUI::Task(); vTaskDelay(pdMS_TO_TICKS(TaskConfig::DisplayMs)); }
}
}
bool AppTasks::Start()
{
    if (state != State::NotStarted) return state == State::Ready;
    state = State::Failed;
    const TaskFunction_t functions[] = {Sampling, Lighting, Encoder, Display};
    const TaskConfig::Specification specifications[] = {
        TaskConfig::Sampling, TaskConfig::Lighting, TaskConfig::Encoder, TaskConfig::Display};
    for (unsigned i = 0; i < 4; ++i) {
        const auto& spec = specifications[i];
        if (xTaskCreatePinnedToCore(functions[i], spec.name, spec.stack, nullptr,
                                   spec.priority, &handles[i], spec.core) != pdPASS) {
            Serial.print("Task creation failed: "); Serial.println(spec.name);
            for (unsigned j = 0; j < i; ++j) { vTaskDelete(handles[j]); handles[j] = nullptr; }
            return false;
        }
    }
    state = State::Ready;
    return true;
}
