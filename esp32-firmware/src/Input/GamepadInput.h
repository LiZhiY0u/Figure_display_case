#pragma once
#include "InputEvent.h"

namespace GamepadInput {
struct Report {
    uint16_t buttons;
    uint8_t dpad;
    int32_t x, y;
};

// One instance per connected controller; all calls run in the input task.
class State {
public:
    UserInput::Action Update(const Report& report, uint32_t now_ms);
    UserInput::Action Tick(uint32_t now_ms);
    void Reset();
private:
    bool a_down_ = false;
    UserInput::Action stick_ = UserInput::Action::None;
    UserInput::Action direction_ = UserInput::Action::None;
    uint32_t repeat_started_ = 0;
    uint32_t repeat_wait_ = 0;
};
}
