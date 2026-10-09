#include "OledUI.h"
#include "Input/InputQueue.h"
#include "Input/InputAdapters.h"
#include "UiInputAdapter.h"
#include "Encoder/HAL_Encoder.h"
#ifndef ENABLE_SERIAL_INPUT_DEBUG
#define ENABLE_SERIAL_INPUT_DEBUG 0
#endif
void OledUI::Key_Scan()
{
    static_assert(BTN_ID_CC == 0 && BTN_ID_CW == 1 && BTN_ID_SP == 2, "UI input IDs changed");
    const uint32_t now_ms = millis();
    const UserInput::Action encoder = InputAdapters::Encoder(ENCODER::Encoder_GetDiff());
    if (encoder != UserInput::Action::None)
        InputQueue::Push({encoder, UserInput::Source::Encoder, now_ms});
#if ENABLE_SERIAL_INPUT_DEBUG
    while (Serial.available() > 0) {
        const UserInput::Action action = InputAdapters::Serial(static_cast<char>(Serial.read()));
        if (action != UserInput::Action::None) {
            InputQueue::Push({action, UserInput::Source::Serial, now_ms});
            break; // At most one valid serial command per iteration.
        }
    }
#endif
    // Keep a pending UI event intact during page transitions.
    UserInput::Event event;
    if (!btn.pressed && InputQueue::Read(event)) {
        const uint8_t id = UiInputAdapter::ButtonId(event.action);
        if (id != 255) {
            btn.id = id;
            btn.pressed = true;
        }
    }
}

