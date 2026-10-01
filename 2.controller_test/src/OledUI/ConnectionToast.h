#ifndef OLED_CONNECTION_TOAST_H
#define OLED_CONNECTION_TOAST_H

#include "Input/BleRemote.h"
#include "ConnectionIcons.h"
#include "OledDriver.h"

namespace ConnectionToast
{
inline void Draw()
{
    static bool active = false;
    static uint32_t started = 0;
    static BleRemote::ConnectionEvent event;
    const uint32_t now = millis();
    if (active && now - started >= 2400u)
        active = false;
    if (!active && BleRemote::ReadConnectionEvent(event))
    {
        active = true;
        started = now;
    }
    if (!active)
        return;

    const uint32_t elapsed = now - started;
    int y = 2;
    if (elapsed < 200u)
        y = -24 + static_cast<int>(elapsed * 26u / 200u);
    else if (elapsed >= 2200u)
        y = 2 - static_cast<int>((elapsed - 2200u) * 26u / 200u);

    const bool gamepad = event == BleRemote::ConnectionEvent::GamepadConnected ||
        event == BleRemote::ConnectionEvent::GamepadDisconnected;
    const bool connected = event == BleRemote::ConnectionEvent::GamepadConnected ||
        event == BleRemote::ConnectionEvent::MiniAppConnected;
    const uint8_t *icon = gamepad ? ConnectionIcons::Gamepad : ConnectionIcons::MiniApp;
    const char *title = gamepad ? "Gamepad" : "Mini App";
    const uint8_t color = u8g2.getDrawColor();
    const uint8_t *font = u8g2.getU8g2()->font;
    u8g2.setDrawColor(0);
    u8g2.drawRBox(2, y, 124, 24, 3);
    u8g2.setDrawColor(1);
    u8g2.drawRFrame(2, y, 124, 24, 3);
    u8g2.drawXBMP(7, y + 4, 16, 16, icon);
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(29, y + 10, title);
    u8g2.drawStr(29, y + 19, connected ? "Connected" : "Disconnected");
    u8g2.setFont(font);
    u8g2.setDrawColor(color);
}
} // namespace ConnectionToast

#endif
