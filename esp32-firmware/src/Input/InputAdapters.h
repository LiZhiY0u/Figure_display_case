#pragma once
#include "InputEvent.h"
#include "RemoteKeyInput.h"
#include "SerialDebugInput.h"

namespace InputAdapters {
inline UserInput::Action Encoder(int16_t diff)
{
    return diff > 0 ? UserInput::Action::Next : diff < 0 ? UserInput::Action::Previous : UserInput::Action::None;
}
inline UserInput::Action MiniApp(uint8_t command)
{
    switch (RemoteKeyInput::Decode(command)) {
    case RemoteKeyInput::Action::Previous: return UserInput::Action::Previous;
    case RemoteKeyInput::Action::Next: return UserInput::Action::Next;
    case RemoteKeyInput::Action::Confirm: return UserInput::Action::Confirm;
    default: return UserInput::Action::None;
    }
}
inline UserInput::Action Serial(char command)
{
    switch (SerialDebugInput::Decode(command)) {
    case SerialDebugInput::Command::RotateLeft: return UserInput::Action::Previous;
    case SerialDebugInput::Command::RotateRight: return UserInput::Action::Next;
    case SerialDebugInput::Command::Press: return UserInput::Action::Confirm;
    default: return UserInput::Action::None;
    }
}
}
