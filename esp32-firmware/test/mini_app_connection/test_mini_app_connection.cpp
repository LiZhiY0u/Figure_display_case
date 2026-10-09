#include <cassert>
#include "Bluetooth/MiniAppConnection.h"
int main()
{
    MiniAppConnection::State state;
    assert(!state.Connect(42, false, true));
    assert(!state.Connect(42, true, false));
    assert(!state.Connect(0xffff, true, true));
    assert(!state.Disconnect(0xffff));
    assert(state.Connect(42, true, true));
    assert(!state.Connect(42, true, true));
    assert(!state.Connect(43, true, true));
    assert(!state.Disconnect(99));
    assert(state.Disconnect(42));
    assert(!state.Disconnect(42));
    assert(state.Connect(43, true, true));
    assert(state.Disconnect(43));
}
