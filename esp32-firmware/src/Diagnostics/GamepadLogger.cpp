#include "GamepadLogger.h"
#include <Arduino.h>
#include <Bluepad32.h>

#ifndef ENABLE_GAMEPAD_INPUT_LOG
#define ENABLE_GAMEPAD_INPUT_LOG 0
#endif

#if ENABLE_GAMEPAD_INPUT_LOG
namespace {
struct Snapshot { bool valid; GamepadInput::Report report; uint32_t time; };
Snapshot snapshots[BP32_MAX_GAMEPADS] = {};
}
#endif

void GamepadLogger::Reset(uint8_t slot)
{
#if ENABLE_GAMEPAD_INPUT_LOG
    if (slot < BP32_MAX_GAMEPADS) snapshots[slot].valid = false;
#else
    (void)slot;
#endif
}

void GamepadLogger::Log(uint8_t slot, const GamepadInput::Report& report, uint32_t now_ms)
{
#if ENABLE_GAMEPAD_INPUT_LOG
    if (slot >= BP32_MAX_GAMEPADS) return;
    Snapshot& last = snapshots[slot];
    const bool digital = !last.valid || last.report.buttons != report.buttons || last.report.dpad != report.dpad;
    const bool analog = !last.valid || abs(last.report.x - report.x) >= 16 || abs(last.report.y - report.y) >= 16 ||
        (last.report.x != 0 && report.x == 0) || (last.report.y != 0 && report.y == 0);
    if (!digital && !(analog && now_ms - last.time >= 100u)) return;
    const uint16_t previous = last.valid ? last.report.buttons : 0;
    Serial.printf("[PAD %lu] slot=%u btn=%04X press=%04X release=%04X dpad=%02X L=(%ld,%ld)\n",
        static_cast<unsigned long>(now_ms), slot, report.buttons,
        report.buttons & ~previous, previous & ~report.buttons, report.dpad,
        static_cast<long>(report.x), static_cast<long>(report.y));
    last = {true, report, now_ms};
#else
    (void)slot; (void)report; (void)now_ms;
#endif
}
