#include <cassert>
#include <cstdint>

#include "Input/RemoteKeyInput.h"

int main()
{
    using RemoteKeyInput::Action;

    assert(RemoteKeyInput::Decode(0x01) == Action::Previous);
    assert(RemoteKeyInput::Decode(0x02) == Action::Next);
    assert(RemoteKeyInput::Decode(0x03) == Action::Previous);
    assert(RemoteKeyInput::Decode(0x04) == Action::Next);
    assert(RemoteKeyInput::Decode(0x05) == Action::Confirm);
    assert(RemoteKeyInput::Decode(0x00) == Action::None);
    assert(RemoteKeyInput::Decode(0xFF) == Action::None);

    return 0;
}
