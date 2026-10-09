#ifndef OLED_UI_TYPES_H
#define OLED_UI_TYPES_H

#include <stdint.h>

constexpr uint8_t UI_DEPTH = 20;

enum UI_PAGE
{
    M_WINDOW,
    M_SLEEP,
    M_MAIN,
    M_EDITOR,
    M_KNOB,
    M_KRF,
    M_KPF,
    M_VOLT,
    M_SETTING,
    M_ANIM_CURVE,
    M_ABOUT,
    M_LIGHT_MAIN,
    M_LIGHT_BRI,
    M_LIGHT_COLOR,
    M_LIGHT_EFFECT,
};

enum UI_STATE
{
    S_FADE,
    S_WINDOW,
    S_LAYER_IN,
    S_LAYER_OUT,
    S_NONE,
};

#endif
