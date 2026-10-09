#include <cassert>
#include <type_traits>

#include "OledUI/MenuItem.h"

int main()
{
    static_assert(!std::is_constructible<M_SELECT, const char *>::value,
                  "menu items must declare an explicit type");

    const M_SELECT value_without_prefix("Brightness", MenuItemType::Value, 7);
    const M_SELECT action_with_value_prefix("~ Not a value", MenuItemType::Action);

    assert(RenderModeFor(value_without_prefix) == MenuItemRenderMode::Value);
    assert(value_without_prefix.data_index == 7);
    assert(RenderModeFor(action_with_value_prefix) == MenuItemRenderMode::None);

    assert(RenderModeFor(M_SELECT("Title", MenuItemType::Title)) == MenuItemRenderMode::None);
    assert(RenderModeFor(M_SELECT("Info", MenuItemType::Info)) == MenuItemRenderMode::None);
    assert(RenderModeFor(M_SELECT("Separator", MenuItemType::Separator)) == MenuItemRenderMode::None);
    assert(RenderModeFor(M_SELECT("Back", MenuItemType::Back)) == MenuItemRenderMode::None);
    assert(RenderModeFor(M_SELECT("Toggle", MenuItemType::Toggle, 4)) == MenuItemRenderMode::Toggle);
    assert(RenderModeFor(M_SELECT("Choice", MenuItemType::Choice, 9)) == MenuItemRenderMode::Choice);
    assert(RenderModeFor(M_SELECT("Rotate", MenuItemType::RotateBinding, 0)) == MenuItemRenderMode::RotateBinding);
    assert(RenderModeFor(M_SELECT("Press", MenuItemType::PressBinding, 1)) == MenuItemRenderMode::PressBinding);

    return 0;
}
