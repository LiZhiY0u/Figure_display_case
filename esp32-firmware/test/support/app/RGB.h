#pragma once
#define NEO_GRB 1
#define NEO_KHZ800 2
struct Adafruit_NeoPixel {
    Adafruit_NeoPixel(int,int,int) {}
    void begin() {} void show() {} void setBrightness(int) {}
};
void rainbow(int);
