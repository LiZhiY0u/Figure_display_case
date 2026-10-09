#include "Controller.h"
#include "Input/BleRemote.h"
#include "Input/GamepadInput.h"
#include "Input/InputQueue.h"
#include "Diagnostics/GamepadLogger.h"

namespace {
const uint8_t TargetAddress[6] = {0xC0, 0xD6, 0xD5, 0xEE, 0xDB, 0x8A};
struct Slot {
    ControllerPtr controller = nullptr;
    bool notified = false;
    GamepadInput::State input;
};
Slot slots[BP32_MAX_GAMEPADS];

bool IsTarget(ControllerPtr controller)
{
    const ControllerProperties properties = controller->getProperties();
    for (uint8_t i = 0; i < 6; ++i)
        if (properties.btaddr[i] != TargetAddress[i]) return false;
    return true;
}

void Submit(UserInput::Action action, uint32_t now_ms)
{
    if (action != UserInput::Action::None)
        InputQueue::Push({action, UserInput::Source::Gamepad, now_ms});
}

void OnConnected(ControllerPtr controller)
{
    if (controller->isGamepad() && !IsTarget(controller)) {
        controller->disconnect();
        return;
    }
    for (uint8_t i = 0; i < BP32_MAX_GAMEPADS; ++i) {
        Slot& slot = slots[i];
        if (slot.controller != nullptr) continue;
        slot.controller = controller;
        slot.input.Reset();
        slot.notified = controller->isGamepad();
        GamepadLogger::Reset(i);
        if (slot.notified) BleRemote::Notify(BleRemote::ConnectionEvent::GamepadConnected);
        Serial.printf("Gamepad connection: slot=%u, model=%s\n", i, controller->getModelName().c_str());
        return;
    }
    Serial.println("Controller connection rejected: no free slot");
    controller->disconnect();
}

void OnDisconnected(ControllerPtr controller)
{
    for (uint8_t i = 0; i < BP32_MAX_GAMEPADS; ++i) {
        Slot& slot = slots[i];
        if (slot.controller != controller) continue;
        if (slot.notified) BleRemote::Notify(BleRemote::ConnectionEvent::GamepadDisconnected);
        slot.input.Reset();
        slot.controller = nullptr;
        slot.notified = false;
        GamepadLogger::Reset(i);
        Serial.printf("Gamepad disconnected: slot=%u\n", i);
        return;
    }
}
}

void Controller_init()
{
    Serial.printf("Bluepad32 firmware: %s\n", BP32.firmwareVersion());
    BP32.setup(OnConnected, OnDisconnected);
    // Preserve Bluetooth pairing keys for reconnect after reboot.
    BP32.enableVirtualDevice(false);
}

void Controller_loop()
{
    BP32.update();
    const uint32_t now_ms = millis();
    for (uint8_t i = 0; i < BP32_MAX_GAMEPADS; ++i) {
        Slot& slot = slots[i];
        ControllerPtr controller = slot.controller;
        if (controller == nullptr || !controller->isConnected() || !controller->isGamepad()) continue;
        // Classification can arrive after the first connection callback.
        if (!slot.notified) {
            if (!IsTarget(controller)) { controller->disconnect(); continue; }
            slot.notified = true;
            BleRemote::Notify(BleRemote::ConnectionEvent::GamepadConnected);
        }
        UserInput::Action action = UserInput::Action::None;
        if (controller->hasData()) {
            const GamepadInput::Report report = {controller->buttons(), controller->dpad(), controller->axisX(), controller->axisY()};
            action = slot.input.Update(report, now_ms);
            GamepadLogger::Log(i, report, now_ms);
        }
        // At most one event per controller per iteration, even on A + direction.
        if (action == UserInput::Action::None) action = slot.input.Tick(now_ms);
        Submit(action, now_ms);
    }
}
