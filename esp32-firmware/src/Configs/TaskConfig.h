#pragma once
#include <cstdint>
namespace TaskConfig {
struct Specification { const char* name; uint32_t stack; unsigned priority; int core; };
constexpr Specification Sampling = {"start_task",4096,2,1};
constexpr Specification Lighting = {"Task_ws2812",2048,4,1};
constexpr Specification Encoder = {"Task_key",2048,4,1};
constexpr Specification Display = {"Task_oled",10240,4,1};
constexpr unsigned SamplingMs = 1000, EncoderMs = 10, DisplayMs = 1, MainLoopMs = 10;
}
