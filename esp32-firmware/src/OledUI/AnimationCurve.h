#ifndef OLED_UI_ANIMATION_CURVE_H
#define OLED_UI_ANIMATION_CURVE_H

#include <stdint.h>

enum class AnimationCurve : uint8_t
{
    Linear,
    OutCubic,
    InOutCubic,
    OutBack,
};

inline AnimationCurve AnimationCurveFromSelectionPosition(uint8_t position)
{
    switch (position)
    {
    case 1: return AnimationCurve::Linear;
    case 2: return AnimationCurve::OutCubic;
    case 3: return AnimationCurve::InOutCubic;
    case 4: return AnimationCurve::OutBack;
    default: return AnimationCurve::OutCubic;
    }
}

inline uint8_t AnimationCurveSelectionPosition(AnimationCurve curve)
{
    switch (curve)
    {
    case AnimationCurve::Linear: return 1;
    case AnimationCurve::OutCubic: return 2;
    case AnimationCurve::InOutCubic: return 3;
    case AnimationCurve::OutBack: return 4;
    }

    return 2;
}

inline AnimationCurve AnimationCurveFromValue(uint8_t value)
{
    switch (value)
    {
    case static_cast<uint8_t>(AnimationCurve::Linear):
        return AnimationCurve::Linear;
    case static_cast<uint8_t>(AnimationCurve::OutCubic):
        return AnimationCurve::OutCubic;
    case static_cast<uint8_t>(AnimationCurve::InOutCubic):
        return AnimationCurve::InOutCubic;
    case static_cast<uint8_t>(AnimationCurve::OutBack):
        return AnimationCurve::OutBack;
    default:
        return AnimationCurve::OutCubic;
    }
}

struct AnimationTrack
{
    float start;
    float target;
    uint32_t start_time_ms;
    uint16_t duration_ms;
    AnimationCurve curve;
    bool active;
};

inline float ClampAnimationProgress(float progress)
{
    if (progress <= 0.0f)
        return 0.0f;
    if (progress >= 1.0f)
        return 1.0f;
    return progress;
}

inline float EvaluateAnimationCurve(AnimationCurve curve, float progress)
{
    const float t = ClampAnimationProgress(progress);

    if (t == 0.0f || t == 1.0f)
        return t;

    switch (curve)
    {
    case AnimationCurve::Linear:
        return t;

    case AnimationCurve::OutCubic:
    {
        const float u = 1.0f - t;
        return 1.0f - u * u * u;
    }

    case AnimationCurve::InOutCubic:
        if (t < 0.5f)
            return 4.0f * t * t * t;
        else
        {
            const float u = -2.0f * t + 2.0f;
            return 1.0f - u * u * u / 2.0f;
        }

    case AnimationCurve::OutBack:
    {
        const float overshoot = 1.70158f;
        const float scale = overshoot + 1.0f;
        const float u = t - 1.0f;
        return 1.0f + scale * u * u * u + overshoot * u * u;
    }
    }

    return t;
}

inline uint16_t AnimationDurationMs(uint8_t speed)
{
    return speed == 0 ? 1 : static_cast<uint16_t>(speed) * 6u;
}

inline void ResetAnimationTrack(AnimationTrack &track)
{
    track.start = 0.0f;
    track.target = 0.0f;
    track.start_time_ms = 0;
    track.duration_ms = 0;
    track.curve = AnimationCurve::Linear;
    track.active = false;
}

inline void StartAnimationTrack(
    AnimationTrack &track,
    float current,
    float target,
    uint16_t duration_ms,
    AnimationCurve curve,
    uint32_t now_ms)
{
    track.start = current;
    track.target = target;
    track.start_time_ms = now_ms;
    track.duration_ms = duration_ms;
    track.curve = curve;
    track.active = true;
}

inline bool UpdateAnimation(
    AnimationTrack &track,
    float &value,
    float target,
    uint16_t duration_ms,
    AnimationCurve curve,
    uint32_t now_ms)
{
    if (value == target)
    {
        track.active = false;
        return false;
    }

    if (duration_ms == 0)
    {
        value = target;
        track.active = false;
        return false;
    }

    if (!track.active ||
        track.target != target ||
        track.duration_ms != duration_ms ||
        track.curve != curve)
    {
        StartAnimationTrack(track, value, target, duration_ms, curve, now_ms);
    }

    const uint32_t elapsed_ms = now_ms - track.start_time_ms;
    if (elapsed_ms >= track.duration_ms)
    {
        value = track.target;
        track.active = false;
        return false;
    }

    const float progress =
        static_cast<float>(elapsed_ms) / static_cast<float>(track.duration_ms);
    const float eased = EvaluateAnimationCurve(track.curve, progress);
    value = track.start + (track.target - track.start) * eased;
    return true;
}

#endif
