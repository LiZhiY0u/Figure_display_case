#include "ButtonEvent.h"

#ifdef ARDUINO
#include "Arduino.h"
#define GET_TICK() millis()
#endif

#ifndef GET_TICK
#error "Please define the GET_TICK() function to get the system time "
#endif

#ifndef UINT32_MAX
#define UINT32_MAX 4294967295u
#endif

/*
用法：
1、实现millis()，esp32不用实现
2、在初始化里注册回调函数到EventAttach
3、把EventMonitor放10ms时间片里，参数是实时按键状态，因此按键是绑定在事件里的，而不是class实例的时候
*/

/**
 * @brief  按键事件构造函数
 * @param  LongPressTimeMs_Set: 按键长按触发超时设置
 * @param  LongPressTimeRepeatMs_Set: 长按重复触发时间
 * @retval 无
 */
ButtonEvent::ButtonEvent(
    uint16_t longPressTime,
    uint16_t longPressTimeRepeat,
    uint16_t doubleClickTime)
{
    memset(&priv, 0, sizeof(priv));

    priv.longPressTimeCfg = longPressTime;
    priv.longPressRepeatTimeCfg = longPressTimeRepeat;
    priv.doubleClickTimeCfg = doubleClickTime;

    priv.lastLongPressTime = priv.lastClickTime = priv.lastPressTime = 0;
    priv.isLongPressed = false;
    priv.nowState = STATE_NO_PRESS;

    priv.eventCallback = NULL;
}

/**
 * @brief  获取与上次时间的时间差(带uint32溢出识别)
 * @param  prevTick: 上的时间戳
 * @retval 时间差
 */
uint32_t ButtonEvent::GetTickElaps(uint32_t prevTick)
{
    uint32_t actTime = GET_TICK();

    if (actTime >= prevTick)
    {
        prevTick = actTime - prevTick;
    }
    else
    {
        prevTick = UINT32_MAX - prevTick + 1;
        prevTick += actTime;
    }

    return prevTick;
}

/**
 * @brief  按键事件绑定，参数是回调函数，相当于ms_task,这里把Encoder_PushHandler放里面
 * @param  function: 回调函数指针
 * @retval 无
 */
void ButtonEvent::EventAttach(FuncCallback_t function)
{
    priv.eventCallback = function;
}

/**
 * @brief  监控事件，建议扫描周期10ms，放10ms时间片里，参数是按键状态，这里用Encoder_GetIsPush，也可以直接读按键
 * @param  nowState: 当前按键状态
 * @retval 无
 */
void ButtonEvent::EventMonitor(bool isPress)
{
    if (priv.eventCallback == NULL)
    {
        return;
    }

    if (isPress && priv.nowState == STATE_NO_PRESS) // 按下而且状态有变
    {
        priv.nowState = STATE_PRESS;

        IsPressed = true;
        priv.lastPressTime = GET_TICK();

        priv.eventCallback(this, EVENT_PRESSED); // 按下
        priv.eventCallback(this, EVENT_CHANGED); // 改变
    }

    if (priv.nowState == STATE_NO_PRESS)
    {
        return;
    }

    if (isPress)
    {
        priv.eventCallback(this, EVENT_PRESSING); // 按住
    }

    if (isPress && GetTickElaps(priv.lastPressTime) >= priv.longPressTimeCfg)
    {
        priv.nowState = STATE_LONG_PRESS;

        if (!priv.isLongPressed)
        {
            priv.eventCallback(this, EVENT_LONG_PRESSED); // 长按
            priv.lastLongPressTime = GET_TICK();
            IsLongPressed = priv.isLongPressed = true;
        }
        else if (GetTickElaps(priv.lastLongPressTime) >= priv.longPressRepeatTimeCfg)
        {
            priv.lastLongPressTime = GET_TICK();
            priv.eventCallback(this, EVENT_LONG_PRESSED_REPEAT); // 长按超时
        }
    }
    else if (!isPress)
    {
        priv.nowState = STATE_NO_PRESS;

        if (GetTickElaps(priv.lastClickTime) < priv.doubleClickTimeCfg)
        {
            priv.clickCnt++;
            priv.eventCallback(this, EVENT_DOUBLE_CLICKED); // 双击
        }

        if (priv.isLongPressed)
        {
            priv.eventCallback(this, EVENT_LONG_PRESSED_RELEASED); // 长按释放
        }

        priv.isLongPressed = false;
        IsClicked = true;
        priv.lastClickTime = GET_TICK();

        if (GetTickElaps(priv.lastPressTime) < priv.longPressTimeCfg)
        {
            priv.eventCallback(this, EVENT_SHORT_CLICKED); // 短按
        }

        priv.eventCallback(this, EVENT_CLICKED);
        priv.eventCallback(this, EVENT_RELEASED);
        priv.eventCallback(this, EVENT_CHANGED);
    }
}
