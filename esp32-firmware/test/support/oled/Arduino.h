#pragma once
#include <cstdint>
#include <cstring>
#include <cmath>
#include <deque>
#include <string>
#define PROGMEM
extern uint32_t test_now;
inline uint32_t millis() { return test_now; }
inline void delay(unsigned) {}
inline long random(long low,long) { return low; }
inline long map(long x,long in_min,long in_max,long out_min,long out_max) {
    return (x-in_min)*(out_max-out_min)/(in_max-in_min)+out_min;
}
struct SerialFake {
    std::deque<char> bytes;
    int available() { return static_cast<int>(bytes.size()); }
    int read() { char value=bytes.front(); bytes.pop_front(); return value; }
};
extern SerialFake Serial;
