#pragma once
#include <stdint.h>

// Task-context, nonblocking notification channel. Init before Bluetooth starts.
namespace ConnectionEvents {
enum class Event : uint8_t {
    GamepadConnected, GamepadDisconnected, MiniAppConnected, MiniAppDisconnected
};
bool Init();
bool Push(Event event);
bool Read(Event& event);
uint32_t DroppedCount();
}
