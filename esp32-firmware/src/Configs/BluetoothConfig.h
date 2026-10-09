#pragma once
#include <stdint.h>

namespace BluetoothConfig {
constexpr uint8_t TargetGamepadAddress[6] = {0xC0, 0xD6, 0xD5, 0xEE, 0xDB, 0x8A};
constexpr uint8_t AdvertisingData[] = {
    0x02, 0x01, 0x06,
    0x0b, 0x09, 'F', 'i', 'g', 'u', 'r', 'e', 'C', 'a', 's', 'e'
};
// ATT service FF10 / write FF11 and handle are defined in MiniAppProfile.h.
}
