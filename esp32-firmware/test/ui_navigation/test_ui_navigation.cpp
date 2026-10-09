#include <cassert>
#include <cstdint>

#include "OledUI/UiNavigation.h"

namespace
{
struct TestContext
{
    uint8_t select[UI_DEPTH] = {};
    uint8_t layer = 0;
    uint8_t index = M_MAIN;
    uint8_t state = S_NONE;
};

struct InitRecorder
{
    uint8_t calls = 0;
    UI_PAGE page = M_WINDOW;

    void operator()(UI_PAGE initialized_page)
    {
        ++calls;
        page = initialized_page;
    }
};
}

int main()
{
    using UiNavigation::Result;

    assert(UiNavigation::ParentOf(M_LIGHT_MAIN) == M_MAIN);
    assert(UiNavigation::ParentOf(M_EDITOR) == M_MAIN);
    assert(UiNavigation::ParentOf(M_KNOB) == M_EDITOR);
    assert(UiNavigation::ParentOf(M_KRF) == M_KNOB);
    assert(UiNavigation::ParentOf(M_KPF) == M_KNOB);
    assert(UiNavigation::ParentOf(M_VOLT) == M_MAIN);
    assert(UiNavigation::ParentOf(M_SETTING) == M_MAIN);
    assert(UiNavigation::ParentOf(M_ANIM_CURVE) == M_SETTING);
    assert(UiNavigation::ParentOf(M_ABOUT) == M_SETTING);
    assert(UiNavigation::ParentOf(M_LIGHT_BRI) == M_LIGHT_MAIN);
    assert(UiNavigation::ParentOf(M_LIGHT_COLOR) == M_LIGHT_MAIN);
    assert(UiNavigation::ParentOf(M_LIGHT_EFFECT) == M_LIGHT_MAIN);

    {
        TestContext context;
        context.layer = 3;
        context.select[0] = 2;
        InitRecorder init;

        UiNavigation::OpenRoot(context, M_MAIN, init);

        assert(context.layer == 0);
        assert(context.select[0] == 0);
        assert(context.index == M_MAIN);
        assert(context.state == S_NONE);
        assert(init.calls == 1);
        assert(init.page == M_MAIN);
    }

    {
        TestContext context;

        assert(UiNavigation::RequestEnter(context, M_SETTING) == Result::Accepted);
        assert(context.index == M_SETTING);
        assert(context.state == S_LAYER_IN);

        context.select[1] = 7;
        InitRecorder init;
        assert(UiNavigation::CompleteEnter(context, init) == Result::Accepted);
        assert(context.layer == 1);
        assert(context.select[1] == 0);
        assert(context.state == S_FADE);
        assert(init.calls == 1);
        assert(init.page == M_SETTING);
    }

    {
        TestContext context;
        context.layer = UI_DEPTH - 1;
        context.index = M_SETTING;
        context.state = S_NONE;

        assert(UiNavigation::RequestEnter(context, M_ABOUT) == Result::DepthLimit);
        assert(context.layer == UI_DEPTH - 1);
        assert(context.index == M_SETTING);
        assert(context.state == S_NONE);
    }

    {
        TestContext context;
        context.layer = 2;
        context.index = M_KRF;
        context.select[2] = 4;

        assert(UiNavigation::RequestBack(context) == Result::Accepted);
        assert(context.index == M_KNOB);
        assert(context.state == S_LAYER_OUT);

        InitRecorder init;
        assert(UiNavigation::CompleteBack(context, init) == Result::Accepted);
        assert(context.layer == 1);
        assert(context.select[2] == 0);
        assert(context.state == S_FADE);
        assert(init.calls == 1);
        assert(init.page == M_KNOB);
    }

    {
        TestContext context;
        context.layer = 0;
        context.index = M_MAIN;
        context.state = S_NONE;

        assert(UiNavigation::RequestBack(context) == Result::AtRoot);
        assert(context.layer == 0);
        assert(context.index == M_MAIN);
        assert(context.state == S_NONE);
    }

    return 0;
}
