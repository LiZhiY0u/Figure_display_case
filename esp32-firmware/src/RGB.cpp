#include "RGB.h"

unsigned long _millis = 0;
int RGB_time = 0;
// RGB随机数
int R;
int G;
int B;

// void RGB()
// {
//     // RGB随机数
//     R = random(255);
//     G = random(255);
//     B = random(255);
//     if (num2 == 80)
//     {
//         RGB3.setPixelColor(0, R, G, B);
//         RGB3.setPixelColor(1, B, R, G);
//         RGB3.setPixelColor(2, G, B, R);
//         RGB3.show();
//     }
//     if (num2 == 120)
//     {
//         RGB3.clear(); // 关闭所有灯
//         RGB3.show();
//         num2 = 0;
//     }
//     num2++;
// }

// RGB功能实现
//  Some functions of our own for creating animated effects -----------------
//  Fill RGB3 pixels one after another with a color. RGB3 is NOT cleared
//  first; anything there will be covered pixel by pixel. Pass in color
//  (as a single 'packed' 32-bit value, which you can get by calling
//  RGB3.Color(red, green, blue) as shown in the loop() function above),
//  and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait)
{
    for (int i = 0; i < RGB3.numPixels(); i++)
    {                                 // For each pixel in RGB3...
        RGB3.setPixelColor(i, color); //  Set pixel's color (in RAM)
        RGB3.show();                  //  Update RGB3 to match
        delay(wait);                  //  Pause for a moment
    }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la RGB3.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait)
{
    for (int a = 0; a < 10; a++)
    { // Repeat 10 times...
        for (int b = 0; b < 3; b++)
        {                 //  'b' counts from 0 to 2...
            RGB3.clear(); //   Set all pixels in RAM to 0 (off)
            // 'c' counts up from 'b' to end of RGB3 in steps of 3...
            for (int c = b; c < RGB3.numPixels(); c += 3)
            {
                RGB3.setPixelColor(c, color); // Set pixel 'c' to value 'color'
            }
            RGB3.show(); // Update RGB3 with new contents
            delay(wait); // Pause for a moment
        }
    }
}

void rainbow(int wait)
{
    for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256)
    {

        RGB3.rainbow(firstPixelHue);
        // Above line is equivalent to:
        // RGB3.rainbow(firstPixelHue, 1, 255, 255, true);
        RGB3.show(); // Update RGB3 with new contents
        delay(wait); // Pause for a moment
    }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait)
{
    if (millis() > _millis)
    {
        int firstPixelHue = 0; // First pixel starts at red (hue 0)
        for (int a = 0; a < 30; a++)
        { // Repeat 30 times...
            for (int b = 0; b < 3; b++)
            {                 //  'b' counts from 0 to 2...
                RGB3.clear(); //   Set all pixels in RAM to 0 (off)
                // 'c' counts up from 'b' to end of RGB3 in increments of 3...
                for (int c = b; c < RGB3.numPixels(); c += 3)
                {
                    // hue of pixel 'c' is offset by an amount to make one full
                    // revolution of the color wheel (range 65536) along the length
                    // of the RGB3 (RGB3.numPixels() steps):
                    int hue = firstPixelHue + c * 65536L / RGB3.numPixels();
                    uint32_t color = RGB3.gamma32(RGB3.ColorHSV(hue)); // hue -> RGB
                    RGB3.setPixelColor(c, color);                      // Set pixel 'c' to value 'color'
                }
                RGB3.show(); // Update RGB3 with new contents
                // delay(wait); // Pause for a moment
                firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
                _millis = millis() + 1000;
            }
        }
    }
}