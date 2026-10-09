#include "ConnectionToast.h"
#include "ToastTimeline.h"
#include "Bluetooth/ConnectionEvents.h"
#include "ConnectionIcons.h"
#include "OledDriver.h"
namespace ConnectionToast {
void Draw() {
    static ToastTimeline::State timeline;
    static ConnectionEvents::Event event;
    const uint32_t now=millis();
    if(!timeline.Active(now) && ConnectionEvents::Read(event)) timeline.Start(now);
    if(!timeline.Active(now)) return;
    const int y=2, x=timeline.X(now);
    if(x>=DISP_W) return;
    const bool gamepad=event==ConnectionEvents::Event::GamepadConnected || event==ConnectionEvents::Event::GamepadDisconnected;
    const bool connected=event==ConnectionEvents::Event::GamepadConnected || event==ConnectionEvents::Event::MiniAppConnected;
    const uint8_t* icon=gamepad ? ConnectionIcons::Gamepad : ConnectionIcons::MiniApp;
    const char* title=gamepad ? "Gamepad" : "Mini App";
    const uint8_t color=u8g2.getDrawColor();
    const uint8_t* font=u8g2.getU8g2()->font;
    u8g2.setDrawColor(0);u8g2.drawRBox(x,y,108,24,4);
    u8g2.setDrawColor(1);u8g2.drawRFrame(x,y,108,24,4);
    u8g2.drawXBMP(x+5,y+4,16,16,icon);
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(x+27,y+10,title);
    u8g2.drawStr(x+27,y+19,connected ? "Connected" : "Disconnected");
    u8g2.setFont(font);u8g2.setDrawColor(color);
}
}
