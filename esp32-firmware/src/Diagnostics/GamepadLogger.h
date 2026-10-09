#pragma once
#include "Input/GamepadInput.h"

namespace GamepadLogger {
void Reset(uint8_t slot);
void Log(uint8_t slot, const GamepadInput::Report& report, uint32_t now_ms);
}
