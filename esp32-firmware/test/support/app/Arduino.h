#pragma once
struct SerialFake { void println(const char*) {} };
extern SerialFake Serial;
