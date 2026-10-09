#ifndef __RGB_H__
#define __RGB_H__

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

extern Adafruit_NeoPixel RGB3;

class RGB : public Adafruit_NeoPixel
{
public:
    explicit RGB(int RGB_pin, int RGB_num);
};

void colorWipe(uint32_t color, int wait);
void theaterChase(uint32_t color, int wait);
void rainbow(int wait);
void theaterChaseRainbow(int wait);

#endif /* __RGB__ */