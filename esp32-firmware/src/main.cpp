/** Arduino entry points; application ownership lives in App. */
#include <Arduino.h>
#include "App/App.h"
#include "Configs/TaskConfig.h"
void setup() { (void)App::Init(); }
void loop() {
    App::Update();
    vTaskDelay(pdMS_TO_TICKS(TaskConfig::MainLoopMs));
}
