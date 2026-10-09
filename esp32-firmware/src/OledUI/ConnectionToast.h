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
    if (active && now - started >= 2600u)
        active = false;
    if (!active && BleRemote::ReadConnectionEvent(event))
    {
        active = true;
        started = now;
    }
    if (!active)
        return;

    const uint32_t elapsed = now - started;
    // Right-side trophy-style toast: 300ms ease-out, 2s hold, 300ms ease-in.
    // Coordinates stay non-negative (U8g2 uses unsigned pixel coordinates).
    const int y = 2;
    int x = 18;
    if (elapsed < 300u)
    {
        const uint32_t remaining = 300u - elapsed;
        x += static_cast<int>(110u * remaining * remaining / 90000u);
    }
    else if (elapsed >= 2300u)
    {
        const uint32_t departing = elapsed - 2300u;
        x += static_cast<int>(110u * departing * departing / 90000u);
    }
    if (x >= DISP_W)
        return;

    const bool gamepad = event == BleRemote::ConnectionEvent::GamepadConnected ||
        event == BleRemote::ConnectionEvent::GamepadDisconnected;
    const bool connected = event == BleRemote::ConnectionEvent::GamepadConnected ||
        event == BleRemote::ConnectionEvent::MiniAppConnected;
    const uint8_t *icon = gamepad ? ConnectionIcons::Gamepad : ConnectionIcons::MiniApp;
    const char *title = gamepad ? "Gamepad" : "Mini App";
    const uint8_t color = u8g2.getDrawColor();
    const uint8_t *font = u8g2.getU8g2()->font;
    u8g2.setDrawColor(0);
    u8g2.drawRBox(x, y, 108, 24, 4);
    u8g2.setDrawColor(1);
    u8g2.drawRFrame(x, y, 108, 24, 4);
    u8g2.drawXBMP(x + 5, y + 4, 16, 16, icon);
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(x + 27, y + 10, title);
    u8g2.drawStr(x + 27, y + 19, connected ? "Connected" : "Disconnected");
    u8g2.setFont(font);
    u8g2.setDrawColor(color);
}
} // namespace ConnectionToast

#endif
