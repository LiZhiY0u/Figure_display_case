#pragma once
#include <stdint.h>

namespace MiniAppConnection {
class State {
public:
    bool Connect(uint16_t handle, bool is_le, bool peripheral)
    {
        if (!is_le || !peripheral || handle == Invalid || handle_ != Invalid) return false;
        handle_ = handle;
        return true;
    }
    bool Disconnect(uint16_t handle)
    {
        if (handle_ == Invalid || handle != handle_) return false;
        handle_ = Invalid;
        return true;
    }
private:
    static constexpr uint16_t Invalid = 0xffff;
    uint16_t handle_ = Invalid;
};
}
