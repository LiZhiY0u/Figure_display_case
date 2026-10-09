#include <cassert>
#include <cstdint>
#include <initializer_list>
#include "Input/GamepadInput.h"

using Input::Action;
using GamepadInput::Report;
using GamepadInput::State;

static void confirm_edges()
{
    State s;
    assert(s.Update({1, 0, 0, 0}, 100) == Action::Confirm);
    assert(s.Update({1, 0, 0, 0}, 110) == Action::None);
    assert(s.Tick(1000) == Action::None);
    assert(s.Update({0, 0, 0, 0}, 1010) == Action::None);
    assert(s.Update({3, 0, 0, 0}, 1020) == Action::Confirm);
}
static void repeat_timing(uint32_t start)
{
    State s;
    assert(s.Update({0, 1, 0, 0}, start) == Action::Previous);
    assert(s.Tick(start + 399u) == Action::None);
    assert(s.Tick(start + 400u) == Action::Previous);
    assert(s.Tick(start + 549u) == Action::None);
    assert(s.Tick(start + 550u) == Action::Previous);
    assert(s.Tick(start + 1900u) == Action::Previous);
    assert(s.Tick(start + 2049u) == Action::None);
    assert(s.Tick(start + 2050u) == Action::Previous);
    assert(s.Update({0, 0, 0, 0}, start + 2060u) == Action::None);
    assert(s.Tick(start + 3000u) == Action::None);
}
static void directions_and_priority()
{
    State s;
    for (uint8_t d : {uint8_t(1), uint8_t(8), uint8_t(2), uint8_t(4)}) {
        s.Reset();
        assert(s.Update({0, d, 0, 0}, 0) == ((d == 1 || d == 8) ? Action::Previous : Action::Next));
    }
    s.Reset();
    assert(s.Update({0, 3, 0, -512}, 0) == Action::None);
    assert(s.Tick(500) == Action::None);
    assert(s.Update({0, 1, 0, -512}, 600) == Action::Previous);
    assert(s.Update({0, 2, 0, -512}, 700) == Action::Next);
    assert(s.Tick(1099) == Action::None);
    assert(s.Tick(1100) == Action::Next);
    s.Reset();
    assert(s.Update({1, 1, 0, 0}, 0) == Action::Confirm);
    assert(s.Tick(0) == Action::None);
    assert(s.Update({1, 1, 0, 0}, 20) == Action::None);
    assert(s.Tick(400) == Action::Previous);
    s.Reset();
    assert(s.Tick(1000) == Action::None);
}
static void stick_hysteresis()
{
    State s;
    assert(s.Update({0, 0, 500, 0}, 0) == Action::None);
    assert(s.Update({0, 0, 501, 0}, 10) == Action::Previous);
    assert(s.Update({0, 0, 450, 0}, 20) == Action::None);
    assert(s.Tick(410) == Action::Previous);
    assert(s.Update({0, 0, 200, -200}, 420) == Action::None);
    assert(s.Tick(900) == Action::None);
    assert(s.Update({0, 0, 0, -501}, 1000) == Action::Next);
    assert(s.Update({0, 0, 0, 501}, 1010) == Action::Previous);
    assert(s.Tick(1409) == Action::None);
    assert(s.Tick(1410) == Action::Previous);
}
int main()
{
    confirm_edges();
    repeat_timing(100);
    repeat_timing(UINT32_MAX - 200u);
    directions_and_priority();
    stick_hysteresis();
}
