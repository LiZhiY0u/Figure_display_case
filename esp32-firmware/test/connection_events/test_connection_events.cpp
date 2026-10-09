#include <cassert>
#include "freertos/queue.h"
#include "Bluetooth/ConnectionEvents.h"
namespace QueueFake { bool fail_create = false; Queue queue; }
int main()
{
    using ConnectionEvents::Event;
    Event event;
    assert(!ConnectionEvents::Read(event));
    assert(!ConnectionEvents::Push(Event::GamepadConnected));
    QueueFake::fail_create = true;
    assert(!ConnectionEvents::Init());
    QueueFake::fail_create = false;
    assert(ConnectionEvents::Init());
    const Event sequence[] = {Event::GamepadConnected, Event::MiniAppConnected,
        Event::GamepadDisconnected, Event::MiniAppDisconnected};
    for (unsigned i = 0; i < 8; ++i) assert(ConnectionEvents::Push(sequence[i % 4]));
    assert(!ConnectionEvents::Push(Event::MiniAppConnected));
    assert(ConnectionEvents::DroppedCount() == 1);
    assert(ConnectionEvents::Init());
    for (unsigned i = 0; i < 8; ++i) {
        assert(ConnectionEvents::Read(event));
        assert(event == sequence[i % 4]);
    }
    assert(!ConnectionEvents::Read(event));
}
