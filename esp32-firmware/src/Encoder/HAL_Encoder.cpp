#include "Encoder/ButtonEvent.h"

static ButtonEvent EncoderPush(3000);

static bool EncoderEnable = true;        // encoder使能
static volatile int16_t EncoderDiff = 0; // encoder脉冲，用一次就清零
static bool EncoderDiffDisable = false;

static void Encoder_IrqHandler()
{
    if (!EncoderEnable || EncoderDiffDisable)
    {
        return;
    }

    static volatile int count, countLast;
    static volatile uint8_t a0, b0;
    static volatile uint8_t ab0;
    uint8_t a = digitalRead(KEY_A);
    uint8_t b = digitalRead(KEY_B);
    if (a != a0)
    {
        a0 = a;
        if (b != b0)
        {
            b0 = b;
            count += ((a == b) ? 1 : -1);
            if ((a == b) != ab0)
            {
                count += ((a == b) ? 1 : -1);
            }
            ab0 = (a == b);
        }
    }

    if (count != countLast)
    {
        EncoderDiff += (count - countLast) > 0 ? 1 : -1;
        // Serial.print(EncoderDiff);
        countLast = count;
    }
}

// Encoder button pressed flag
bool key_en = false;

// 按键回调
static void Encoder_PushHandler(ButtonEvent *btn, int event)
{

    if (event == ButtonEvent::EVENT_CLICKED)
    {
        // Serial.printf("EVENT_CLICKED");
    }
    else if (event == ButtonEvent::EVENT_PRESSED)
    {
        // Serial.printf("EVENT_PRESSED");
    }
    else if (event == ButtonEvent::EVENT_PRESSING)
    {
        // Serial.printf("EVENT_PRESSING");
    }
    else if (event == ButtonEvent::EVENT_LONG_PRESSED)
    {
        // Serial.printf("EVENT_LONG_PRESSED");
    }
    else if (event == ButtonEvent::EVENT_RELEASED)
    {
        // Serial.printf("EVENT_RELEASED");
    }
    else if (event == ButtonEvent::EVENT_SHORT_CLICKED)
    {
        // Serial.printf("EVENT_SHORT_CLICKED");
        // key_msg.id = 2;
        // key_msg.pressed = true;
        key_en=1;
    }
    else if (event == ButtonEvent::EVENT_DOUBLE_CLICKED)
    {
        // Serial.printf("EVENT_DOUBLE_CLICKED");
    }
}

static void Encoder_RotateHandler(int16_t diff)
{
}

void ENCODER::Encoder_Init()
{
    pinMode(KEY_A, INPUT_PULLUP);
    pinMode(KEY_B, INPUT_PULLUP);
    pinMode(KEY_EN, INPUT_PULLUP);

    // 脉冲还测不了，有个脚没上拉能力
    attachInterrupt(KEY_A, Encoder_IrqHandler, CHANGE);

    Serial.printf("encoder init succ!!!");

    EncoderPush.EventAttach(Encoder_PushHandler);
}

void ENCODER::Encoder_Update()
{
    EncoderPush.EventMonitor(Encoder_GetIsPush());
}

int16_t ENCODER::Encoder_GetDiff()
{
    int16_t diff = -EncoderDiff / 2;
    if (diff != 0)
    {
        // EncoderDiff是实际的脉冲数；把本次变量用掉了，需要重新置0
        EncoderDiff = 0;
        // Encoder_RotateHandler(diff);
    }

    return diff;
}

// 按下判断
bool ENCODER::Encoder_GetIsPush()
{
    if (!EncoderEnable)
    {
        return false;
    }
    // Serial.print(digitalRead(KEY_EN));
    return (digitalRead(KEY_EN) == LOW);
}

void ENCODER::Encoder_SetEnable(bool en)
{
    EncoderEnable = en;
}
