#pragma once

#include <stdint.h>

namespace SerialDebugInput
{

enum class Command : uint8_t
{
    None,
    RotateLeft,
    RotateRight,
    Press,
};

inline Command Decode(char input)
{
    switch (input)
    {
    case '1': return Command::RotateLeft;
    case '2': return Command::RotateRight;
    case '3': return Command::Press;
    default:  return Command::None;
    }
}

} // namespace SerialDebugInput
