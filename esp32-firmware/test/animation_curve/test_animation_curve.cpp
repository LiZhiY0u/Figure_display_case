#include <cassert>
#include <cmath>
#include <cstdint>

#include "OledUI/AnimationCurve.h"

namespace
{
bool NearlyEqual(float actual, float expected, float tolerance = 0.0001f)
{
    return std::fabs(actual - expected) <= tolerance;
}
}

int main()
{
    const AnimationCurve curves[] = {
        AnimationCurve::Linear,
        AnimationCurve::OutCubic,
        AnimationCurve::InOutCubic,
        AnimationCurve::OutBack,
    };

    for (uint8_t i = 0; i < sizeof(curves) / sizeof(curves[0]); ++i)
    {
        assert(NearlyEqual(EvaluateAnimationCurve(curves[i], 0.0f), 0.0f));
        assert(NearlyEqual(EvaluateAnimationCurve(curves[i], 1.0f), 1.0f));
    }

    assert(NearlyEqual(EvaluateAnimationCurve(AnimationCurve::Linear, 0.5f), 0.5f));
    assert(NearlyEqual(EvaluateAnimationCurve(AnimationCurve::OutCubic, 0.5f), 0.875f));
    assert(NearlyEqual(EvaluateAnimationCurve(AnimationCurve::InOutCubic, 0.5f), 0.5f));
    assert(EvaluateAnimationCurve(AnimationCurve::OutBack, 0.8f) > 1.0f);

    assert(AnimationCurveFromSelectionPosition(1) == AnimationCurve::Linear);
    assert(AnimationCurveFromSelectionPosition(2) == AnimationCurve::OutCubic);
    assert(AnimationCurveFromSelectionPosition(3) == AnimationCurve::InOutCubic);
    assert(AnimationCurveFromSelectionPosition(4) == AnimationCurve::OutBack);
    assert(AnimationCurveFromSelectionPosition(0) == AnimationCurve::OutCubic);
    assert(AnimationCurveSelectionPosition(AnimationCurve::Linear) == 1);
    assert(AnimationCurveSelectionPosition(AnimationCurve::OutCubic) == 2);
    assert(AnimationCurveSelectionPosition(AnimationCurve::InOutCubic) == 3);
    assert(AnimationCurveSelectionPosition(AnimationCurve::OutBack) == 4);

    assert(NearlyEqual(EvaluateAnimationCurve(AnimationCurve::Linear, -1.0f), 0.0f));
    assert(NearlyEqual(EvaluateAnimationCurve(AnimationCurve::Linear, 2.0f), 1.0f));

    assert(AnimationDurationMs(0) == 1);
    assert(AnimationDurationMs(10) == 60);
    assert(AnimationDurationMs(100) == 600);

    AnimationTrack track = {};
    float value = 0.0f;
    assert(UpdateAnimation(track, value, 10.0f, 100, AnimationCurve::Linear, 1000));
    assert(NearlyEqual(value, 0.0f));
    assert(UpdateAnimation(track, value, 10.0f, 100, AnimationCurve::Linear, 1050));
    assert(NearlyEqual(value, 5.0f));
    assert(!UpdateAnimation(track, value, 10.0f, 100, AnimationCurve::Linear, 1100));
    assert(NearlyEqual(value, 10.0f));

    ResetAnimationTrack(track);
    value = 0.0f;
    UpdateAnimation(track, value, 10.0f, 100, AnimationCurve::Linear, 2000);
    UpdateAnimation(track, value, 10.0f, 100, AnimationCurve::Linear, 2050);
    assert(NearlyEqual(value, 5.0f));
    UpdateAnimation(track, value, 20.0f, 100, AnimationCurve::Linear, 2050);
    assert(NearlyEqual(value, 5.0f));
    UpdateAnimation(track, value, 20.0f, 100, AnimationCurve::Linear, 2100);
    assert(NearlyEqual(value, 12.5f));

    ResetAnimationTrack(track);
    value = 0.0f;
    UpdateAnimation(track, value, 10.0f, 100, AnimationCurve::Linear, UINT32_MAX - 15u);
    UpdateAnimation(track, value, 10.0f, 100, AnimationCurve::Linear, 34u);
    assert(NearlyEqual(value, 5.0f));

    return 0;
}
