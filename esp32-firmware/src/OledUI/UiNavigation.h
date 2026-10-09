#ifndef OLED_UI_NAVIGATION_H
#define OLED_UI_NAVIGATION_H

#include "UiTypes.h"

namespace UiNavigation
{
enum class Result : uint8_t
{
    Accepted,
    AtRoot,
    DepthLimit,
    NoParent,
};

constexpr bool HasParent(UI_PAGE page)
{
    return page != M_WINDOW && page != M_SLEEP && page != M_MAIN;
}

inline UI_PAGE ParentOf(UI_PAGE page)
{
    switch (page)
    {
    case M_EDITOR:
    case M_VOLT:
    case M_SETTING:
    case M_LIGHT_MAIN:
        return M_MAIN;
    case M_KNOB:
        return M_EDITOR;
    case M_KRF:
    case M_KPF:
        return M_KNOB;
    case M_ANIM_CURVE:
    case M_ABOUT:
        return M_SETTING;
    case M_LIGHT_BRI:
    case M_LIGHT_COLOR:
    case M_LIGHT_EFFECT:
        return M_LIGHT_MAIN;
    default:
        return page;
    }
}

template <typename Context, typename InitializePage>
void OpenRoot(Context &context, UI_PAGE page, InitializePage &&initialize_page)
{
    context.layer = 0;
    context.select[0] = 0;
    context.index = page;
    initialize_page(page);
    context.state = S_NONE;
}

template <typename Context>
Result RequestEnter(Context &context, UI_PAGE page)
{
    if (context.layer >= UI_DEPTH - 1)
        return Result::DepthLimit;

    context.index = page;
    context.state = S_LAYER_IN;
    return Result::Accepted;
}

template <typename Context, typename InitializePage>
Result CompleteEnter(Context &context, InitializePage &&initialize_page)
{
    if (context.layer >= UI_DEPTH - 1)
    {
        context.state = S_NONE;
        return Result::DepthLimit;
    }

    ++context.layer;
    context.select[context.layer] = 0;
    initialize_page(static_cast<UI_PAGE>(context.index));
    context.state = S_FADE;
    return Result::Accepted;
}

template <typename Context>
Result RequestBack(Context &context)
{
    if (context.layer == 0)
        return Result::AtRoot;

    const UI_PAGE current_page = static_cast<UI_PAGE>(context.index);
    if (!HasParent(current_page))
        return Result::NoParent;

    context.index = ParentOf(current_page);
    context.state = S_LAYER_OUT;
    return Result::Accepted;
}

template <typename Context, typename InitializePage>
Result CompleteBack(Context &context, InitializePage &&initialize_page)
{
    if (context.layer == 0)
    {
        context.state = S_NONE;
        return Result::AtRoot;
    }

    context.select[context.layer] = 0;
    --context.layer;
    initialize_page(static_cast<UI_PAGE>(context.index));
    context.state = S_FADE;
    return Result::Accepted;
}
}

#endif
