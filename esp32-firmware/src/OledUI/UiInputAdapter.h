#pragma once
#include "Input/InputEvent.h"

namespace UiInputAdapter {
// IDs match the existing OLED button enum; verified at the UI integration site.
inline uint8_t ButtonId(UserInput::Action action)
{
    switch (action) {
    case UserInput::Action::Previous: return 0;
    case UserInput::Action::Next: return 1;
    case UserInput::Action::Confirm: return 2;
    default: return 255;
    }
}
}
