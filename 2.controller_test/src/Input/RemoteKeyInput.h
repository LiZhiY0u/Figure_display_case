#ifndef INPUT_REMOTE_KEY_INPUT_H
#define INPUT_REMOTE_KEY_INPUT_H

#include <stdint.h>

namespace RemoteKeyInput
{
enum class Action : uint8_t
{
    None,
    Previous,
    Next,
    Confirm,
};

inline Action Decode(uint8_t command)
{
    switch (command)
    {
    case 0x01: // Up
    case 0x03: // Left
        return Action::Previous;

    case 0x02: // Down
    case 0x04: // Right
        return Action::Next;

    case 0x05: // Confirm
        return Action::Confirm;

    default:
        return Action::None;
    }
}
} // namespace RemoteKeyInput

#endif
