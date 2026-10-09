#include <cassert>
#include <deque>
#include "OledUI/OledUI.h"
#include "Input/InputQueue.h"
uint32_t test_now=123;
SerialFake Serial;
volatile Button_Context btn{};
namespace { int16_t diff=0; std::deque<UserInput::Event> events; }
namespace ENCODER { int16_t Encoder_GetDiff() { auto value=diff;diff=0;return value; } }
namespace InputQueue {
bool Push(const UserInput::Event& event) { events.push_back(event);return true; }
bool Read(UserInput::Event& event) { if(events.empty())return false;event=events.front();events.pop_front();return true; }
}
int main() {
    events.push_back({UserInput::Action::Confirm,UserInput::Source::Gamepad,1});
    diff=1; OledUI::Key_Scan();
    assert(btn.pressed && btn.id==BTN_ID_SP && events.size()==1);
    assert(events.front().source==UserInput::Source::Encoder && events.front().time_ms==123);
    OledUI::Key_Scan(); assert(events.size()==1 && btn.id==BTN_ID_SP);
    btn.pressed=false;OledUI::Key_Scan();assert(btn.pressed && events.empty());
    btn.pressed=false;OledUI::Key_Scan();assert(!btn.pressed);
    Serial.bytes={'x','1','2'};
    OledUI::Key_Scan();
#if ENABLE_SERIAL_INPUT_DEBUG
    assert(btn.pressed && btn.id==BTN_ID_CC && Serial.bytes.size()==1);
    btn.pressed=false;OledUI::Key_Scan();assert(btn.pressed && btn.id==BTN_ID_CW);
#else
    assert(!btn.pressed && Serial.bytes.size()==3);
#endif
}
