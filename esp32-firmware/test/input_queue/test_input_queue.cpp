#include <cassert>
#include "freertos/queue.h"
#include "Input/InputQueue.h"
namespace QueueFake {
bool fail_create = false;
Queue queue;
}

int main()
{
    UserInput::Event event = {UserInput::Action::Confirm, UserInput::Source::Gamepad, 0};
    assert(!InputQueue::Read(event));
    assert(!InputQueue::Push(event));
    QueueFake::fail_create = true;
    assert(!InputQueue::Init());
    QueueFake::fail_create = false;
    assert(InputQueue::Init());
    assert(!InputQueue::Push({UserInput::Action::None, UserInput::Source::Gamepad, 0}));
    for (uint32_t i = 0; i < 16; ++i)
        assert(InputQueue::Push({UserInput::Action::Confirm, UserInput::Source::MiniApp, i}));
    assert(!InputQueue::Push({UserInput::Action::Next, UserInput::Source::Serial, 17}));
    assert(InputQueue::DroppedCount() == 1);
    assert(InputQueue::Init()); // Must not reset an existing queue.
    for (uint32_t i = 0; i < 16; ++i) {
        assert(InputQueue::Read(event));
        assert(event.action == UserInput::Action::Confirm);
        assert(event.source == UserInput::Source::MiniApp);
        assert(event.time_ms == i);
    }
    assert(!InputQueue::Read(event));
    assert(InputQueue::Push({UserInput::Action::Next, UserInput::Source::Serial, 42}));
    assert(InputQueue::Read(event) && event.time_ms == 42);
}
