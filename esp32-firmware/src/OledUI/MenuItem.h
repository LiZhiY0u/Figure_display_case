#ifndef OLED_UI_MENU_ITEM_H
#define OLED_UI_MENU_ITEM_H

#include <stdint.h>

enum class MenuItemType : uint8_t
{
    Title,
    Action,
    Info,
    Separator,
    Back,
    Value,
    Toggle,
    Choice,
    RotateBinding,
    PressBinding,
};

enum class MenuItemRenderMode : uint8_t
{
    None,
    Value,
    Toggle,
    Choice,
    RotateBinding,
    PressBinding,
};

struct M_SELECT
{
    const char *m_select;
    MenuItemType type;
    uint8_t data_index;

    constexpr M_SELECT(const char *label, MenuItemType item_type, uint8_t index = 0)
        : m_select(label), type(item_type), data_index(index)
    {
    }
};

inline MenuItemRenderMode RenderModeFor(const M_SELECT &item)
{
    switch (item.type)
    {
    case MenuItemType::Value:         return MenuItemRenderMode::Value;
    case MenuItemType::Toggle:        return MenuItemRenderMode::Toggle;
    case MenuItemType::Choice:        return MenuItemRenderMode::Choice;
    case MenuItemType::RotateBinding: return MenuItemRenderMode::RotateBinding;
    case MenuItemType::PressBinding:  return MenuItemRenderMode::PressBinding;
    default:                          return MenuItemRenderMode::None;
    }
}

#endif
