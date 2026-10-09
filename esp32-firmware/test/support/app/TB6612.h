#pragma once
#include <cassert>
struct Motor {
    Motor(int in1,int in2,int pwm,int frequency,int channel) {
        assert(in1==16 && in2==4 && pwm==17 && frequency==1000 && channel==1);
    }
};
