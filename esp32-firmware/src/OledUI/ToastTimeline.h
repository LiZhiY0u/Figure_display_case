#pragma once
#include <cstdint>
namespace ToastTimeline {
class State {
public:
    void Start(uint32_t now) { started_=now; active_=true; }
    bool Active(uint32_t now) const { return active_ && now-started_<2600u; }
    int X(uint32_t now) const {
        const uint32_t elapsed=now-started_;
        if(elapsed<300u) { const uint32_t remaining=300u-elapsed; return 18+static_cast<int>(110u*remaining*remaining/90000u); }
        if(elapsed>=2300u && elapsed<2600u) { const uint32_t departing=elapsed-2300u; return 18+static_cast<int>(110u*departing*departing/90000u); }
        return 18;
    }
private:
    uint32_t started_=0;
    bool active_=false;
};
}
