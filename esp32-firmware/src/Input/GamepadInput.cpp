#include "GamepadInput.h"
#include "Configs/InputConfig.h"

using UserInput::Action;

Action GamepadInput::State::Update(const Report& report, uint32_t now_ms)
{
    const bool a_down = (report.buttons & 0x01) != 0;
    const bool confirm = a_down && !a_down_;
    a_down_ = a_down;

    if (report.x >= -InputConfig::StickRelease && report.x <= InputConfig::StickRelease &&
        report.y >= -InputConfig::StickRelease && report.y <= InputConfig::StickRelease)
        stick_ = Action::None;
    else if (report.x > InputConfig::StickEngage || report.y > InputConfig::StickEngage)
        stick_ = Action::Previous;
    else if (report.x < -InputConfig::StickEngage || report.y < -InputConfig::StickEngage)
        stick_ = Action::Next;

    Action direction = stick_;
    if (report.dpad != 0) {
        const bool previous = (report.dpad & 0x09) != 0;
        const bool next = (report.dpad & 0x06) != 0;
        direction = previous == next ? Action::None : previous ? Action::Previous : Action::Next;
    }
    const bool changed = direction != direction_;
    if (changed || confirm) {
        direction_ = direction;
        repeat_started_ = now_ms;
        repeat_wait_ = InputConfig::InitialRepeatMs;
    }
    if (confirm)
        return Action::Confirm;
    return changed ? direction : Action::None;
}

Action GamepadInput::State::Tick(uint32_t now_ms)
{
    if (direction_ == Action::None || now_ms - repeat_started_ < repeat_wait_)
        return Action::None;
    repeat_started_ = now_ms;
    repeat_wait_ = InputConfig::RepeatMs;
    return direction_;
}

void GamepadInput::State::Reset()
{
    *this = State();
}
