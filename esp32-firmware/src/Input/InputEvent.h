#pragma once
#include <stdint.h>

namespace Input {
enum class Action : uint8_t { None, Previous, Next, Confirm };
enum class Source : uint8_t { Gamepad, MiniApp, Encoder, Serial };
struct Event {
    Action action;
    Source source;
    uint32_t time_ms;
};
}
