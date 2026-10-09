#include <cassert>
#include "freertos/queue.h"
#include "Input/InputQueue.h"
namespace QueueFake {
bool fail_create = false;
Queue queue;
}

int main()
{
    Input::Event event = {Input::Action::Confirm, Input::Source::Gamepad, 0};
    assert(!InputQueue::Read(event));
    assert(!InputQueue::Push(event));
    QueueFake::fail_create = true;
    assert(!InputQueue::Init());
    QueueFake::fail_create = false;
    assert(InputQueue::Init());
    assert(!InputQueue::Push({Input::Action::None, Input::Source::Gamepad, 0}));
    for (uint32_t i = 0; i < 16; ++i)
        assert(InputQueue::Push({Input::Action::Confirm, Input::Source::MiniApp, i}));
    assert(!InputQueue::Push({Input::Action::Next, Input::Source::Serial, 17}));
    assert(InputQueue::DroppedCount() == 1);
    assert(InputQueue::Init()); // Must not reset an existing queue.
    for (uint32_t i = 0; i < 16; ++i) {
        assert(InputQueue::Read(event));
        assert(event.action == Input::Action::Confirm);
        assert(event.source == Input::Source::MiniApp);
        assert(event.time_ms == i);
    }
    assert(!InputQueue::Read(event));
    assert(InputQueue::Push({Input::Action::Next, Input::Source::Serial, 42}));
    assert(InputQueue::Read(event) && event.time_ms == 42);
}
