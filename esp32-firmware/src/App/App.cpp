#include "App.h"
#include "AppTasks.h"
#include <Arduino.h>
#include "HAL/HAL.h"
#include "Input/InputQueue.h"
#include "Bluetooth/BluetoothService.h"

namespace {
enum class State { NotStarted, Failed, Ready };
State state = State::NotStarted;
}
bool App::Init()
{
    if (state != State::NotStarted) return state == State::Ready;
    state = State::Failed; // Partial initialization must not be retried implicitly.
    HAL::Init();
    if (!InputQueue::Init()) {
        Serial.println("Input queue initialization failed; application not started");
        return false;
    }
    if (!BluetoothService::Init()) {
        Serial.println("Bluetooth initialization failed; application not started");
        return false;
    }
    if (!AppTasks::Start()) {
        Serial.println("Task initialization failed; application not started");
        return false;
    }
    state = State::Ready;
    return true;
}
void App::Update()
{
    if (state == State::Ready) BluetoothService::Update();
}
