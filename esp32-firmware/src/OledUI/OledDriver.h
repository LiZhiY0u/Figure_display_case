/**
 * @file   OledDriver.h
 * @author LZY (original author: RQNG, WouoUI)
 * @brief  OLED driver layer - u8g2 object, init, buffer management
 *
 * Hardware: SH1106 128x64 via I2C (HW I2C, ESP32 GPIO21/22)
 */

#ifndef __OLED_DRIVER_H
#define __OLED_DRIVER_H

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// Hardware pins (from main.h)
#ifndef OLED_SCL
#define OLED_SCL 22
#endif
#ifndef OLED_SDA
#define OLED_SDA 21
#endif

// Display dimensions
#define DISP_H 64   // Screen height (pixels)
#define DISP_W 128  // Screen width (pixels)

// I2C bus settings
#define I2C_FREQ 400000   // 400kHz
#define U8G2_BUSCLK 800000 // u8g2 bus clock

// ===========================
// u8g2 global object (extern)
// ===========================
extern U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;

// ===========================
// Driver API
// ===========================
namespace OledDriver
{
    /**
     * @brief Initialize I2C bus and OLED display
     * @note  Must be called before any other OLED operations
     */
    void Init();

    /**
     * @brief Send the internal buffer to the display
     * @note  Called at the end of each UI frame in ui_proc()
     */
    void SendBuffer();

    /**
     * @brief Clear the internal buffer
     */
    void ClearBuffer();

    /**
     * @brief Set display contrast/brightness (0-255)
     */
    void SetContrast(uint8_t contrast);

    /**
     * @brief Get pointer to display buffer (for fade effects)
     */
    uint8_t* GetBufferPtr();

    /**
     * @brief Get display buffer length in bytes
     */
    uint16_t GetBufferLen();
}

#endif // __OLED_DRIVER_H
