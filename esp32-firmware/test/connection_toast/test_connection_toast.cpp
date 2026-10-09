#include <cassert>
#include <deque>
#include "OledUI/ConnectionToast.h"
#include "OledUI/OledDriver.h"
#include "Bluetooth/ConnectionEvents.h"
uint32_t test_now=100;
SerialFake Serial;
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;
namespace { std::deque<ConnectionEvents::Event> events; }
namespace ConnectionEvents {
bool Read(Event& event) { if(events.empty())return false;event=events.front();events.pop_front();return true; }
}
int main() {
    events={ConnectionEvents::Event::GamepadConnected,ConnectionEvents::Event::MiniAppDisconnected};
    const uint8_t* font=u8g2_font_helvB18_tr;
    u8g2.setFont(font);u8g2.setDrawColor(2);u8g2.calls.clear();
    ConnectionToast::Draw();assert(events.size()==1 && u8g2.calls.empty());
    test_now=400;ConnectionToast::Draw();
    assert(u8g2.data.font==font && u8g2.color==2);
    bool title=false,connected=false;
    for(const auto& call:u8g2.calls) { title|=call.text=="Gamepad";connected|=call.text=="Connected"; }
    assert(title && connected);
    test_now=2699;ConnectionToast::Draw();assert(events.size()==1);
    test_now=2700;u8g2.calls.clear();ConnectionToast::Draw();assert(events.empty() && u8g2.calls.empty());
    test_now=3000;ConnectionToast::Draw();
    bool phone=false,disconnected=false;
    for(const auto& call:u8g2.calls) { phone|=call.text=="Mini App";disconnected|=call.text=="Disconnected"; }
    assert(phone && disconnected && u8g2.data.font==font && u8g2.color==2);
}
