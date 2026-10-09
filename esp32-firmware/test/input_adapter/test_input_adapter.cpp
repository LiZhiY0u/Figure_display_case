#include <cassert>
#include "Input/InputAdapters.h"
#include "OledUI/UiInputAdapter.h"

int main()
{
    using UserInput::Action;
    assert(UiInputAdapter::ButtonId(Action::Previous) == 0);
    assert(UiInputAdapter::ButtonId(Action::Next) == 1);
    assert(UiInputAdapter::ButtonId(Action::Confirm) == 2);
    assert(UiInputAdapter::ButtonId(Action::None) == 255);
    assert(InputAdapters::Encoder(1) == Action::Next);
    assert(InputAdapters::Encoder(-1) == Action::Previous);
    assert(InputAdapters::Encoder(0) == Action::None);
    assert(InputAdapters::Serial('1') == Action::Previous);
    assert(InputAdapters::Serial('2') == Action::Next);
    assert(InputAdapters::Serial('3') == Action::Confirm);
    assert(InputAdapters::Serial('\n') == Action::None);
    assert(InputAdapters::MiniApp(1) == Action::Previous);
    assert(InputAdapters::MiniApp(3) == Action::Previous);
    assert(InputAdapters::MiniApp(2) == Action::Next);
    assert(InputAdapters::MiniApp(4) == Action::Next);
    assert(InputAdapters::MiniApp(5) == Action::Confirm);
    assert(InputAdapters::MiniApp(255) == Action::None);
}
