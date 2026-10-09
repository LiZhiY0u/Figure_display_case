#include <cassert>

#include "Input/SerialDebugInput.h"

int main()
{
    using SerialDebugInput::Command;

    assert(SerialDebugInput::Decode('1') == Command::RotateLeft);
    assert(SerialDebugInput::Decode('2') == Command::RotateRight);
    assert(SerialDebugInput::Decode('3') == Command::Press);
    assert(SerialDebugInput::Decode('\r') == Command::None);
    assert(SerialDebugInput::Decode('\n') == Command::None);
    assert(SerialDebugInput::Decode('x') == Command::None);

    return 0;
}
