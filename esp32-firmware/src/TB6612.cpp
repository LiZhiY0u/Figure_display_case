#include "TB6612.h"
#include <Arduino.h>
#include <driver/ledc.h>

// 增加位置模式



Motor::Motor(int in_pin_1, int in_pin_2, int pwm_pin, int freq, int channel)
{
    In1 = in_pin_1;
    In2 = in_pin_2;
    PWM = pwm_pin;
    Channel = channel;

    pinMode(In1, OUTPUT);
    pinMode(In2, OUTPUT);
    pinMode(pwm_pin, OUTPUT);

    ledcSetup(channel, freq, 8);
    ledcAttachPin(pwm_pin, channel);
}

void Motor::drive(int speed)
{
    if (speed >= 0)
        fwd(speed);
    else
        rev(-speed);
}
void Motor::drive(int speed, int duration)
{
    drive(speed);
    delay(duration);
}

void Motor::fwd(int speed)
{
    digitalWrite(In1, HIGH);
    digitalWrite(In2, LOW);
    ledcWrite(Channel, speed);
}

void Motor::rev(int speed)
{
    digitalWrite(In1, LOW);
    digitalWrite(In2, HIGH);
    ledcWrite(Channel, speed);
}

void Motor::brake()
{
    digitalWrite(In1, HIGH);
    digitalWrite(In2, HIGH);
    ledcWrite(Channel, 0);
}

// void Motor::standby()
// {
//     digitalWrite(Standby, LOW);
// }
