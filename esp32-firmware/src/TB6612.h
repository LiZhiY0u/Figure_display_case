#ifndef __TB6612_H__
#define __TB6612_H__

#include <Arduino.h>
#include <driver/ledc.h>

#define DEFAULTSPEED 255

class Motor
{
public:
    Motor(int in_pin_1, int in_pin_2, int pwm_pin, int freq, int channel);

    void drive(int speed);

    void drive(int speed, int duration);

    void brake();

    void standby();

private:
    // variables for the 2 inputs, PWM input, Offset value, and the Standby pin
    int In1, In2, PWM,Channel;

    // private functions that spin the motor CC and CCW
    void fwd(int speed);
    void rev(int speed);
};

#endif // __TB6612_H__
