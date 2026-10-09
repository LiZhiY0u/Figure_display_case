/**
 * @file   OledDriver.cpp
 * @author LZY (original author: RQNG, WouoUI)
 * @brief  OLED driver implementation
 */

#include "OledDriver.h"

// ===========================
// Global u8g2 object (hardware I2C)
// ===========================
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0,
    /* clock=*/ OLED_SCL,
    /* data=*/  OLED_SDA);

// ===========================
// Buffer globals (used by UI for fade effects)
// ===========================
uint8_t* g_buf_ptr = nullptr;
uint16_t g_buf_len = 0;

namespace OledDriver
{

void Init()
{
    Wire.begin(OLED_SDA, OLED_SCL, I2C_FREQ);
    u8g2.setBusClock(U8G2_BUSCLK);
    u8g2.begin();
    // Note: font is set per-page in each show function, not here

    g_buf_ptr = u8g2.getBufferPtr();
    g_buf_len = 8 * u8g2.getBufferTileHeight() * u8g2.getBufferTileWidth();
}

void SendBuffer()
{
    u8g2.sendBuffer();
}

void ClearBuffer()
{
    u8g2.clearBuffer();
}

void SetContrast(uint8_t contrast)
{
    u8g2.setContrast(contrast);
}

uint8_t* GetBufferPtr()
{
    return g_buf_ptr;
}

uint16_t GetBufferLen()
{
    return g_buf_len;
}

} // namespace OledDriver
