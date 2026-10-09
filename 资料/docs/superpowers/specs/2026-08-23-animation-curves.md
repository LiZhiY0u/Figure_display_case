# Runtime Animation Curves Design

## Status

Approved by the user on 2026-08-23.

## Goal

Replace frame-rate-dependent exponential UI motion with fixed-duration animation
tracks whose easing curve can be selected at runtime from the Settings menu.

## Curves

- Linear: diagnostic baseline.
- Out Cubic: responsive motion that decelerates into the target; default.
- InOut Cubic: smooth page motion with acceleration and deceleration.
- Out Back: a small, controlled overshoot for visual comparison.

## User Interface

- Add `Anim Curve` to the Settings menu.
- Add a child page containing the four curves.
- Selecting a row immediately changes the curve used by all positional UI
  animations.
- The selection is runtime-only, consistent with the existing UI parameters.

## Architecture

- Keep curve evaluation and animation tracking Arduino-independent and C++11
  compatible.
- Each animated property owns an `AnimationTrack` containing its start value,
  target, start time, duration, curve, and active state.
- Existing animation speed parameters map monotonically to duration: larger
  values remain slower.
- Retargeting an active animation starts a new track from its current rendered
  value, preventing jumps.
- Curve changes during motion also restart from the current value.

## Constraints

- No heap allocation or new dependency.
- Preserve existing menu behavior, labels, navigation bounds, and serial debug
  input.
- Complete each track at the exact target value.
- Handle `millis()` wraparound with unsigned elapsed-time arithmetic.

## Acceptance Criteria

1. All four curves map progress 0 to 0 and progress 1 to 1.
2. Out Cubic and InOut Cubic produce the expected normalized midpoint values.
3. Out Back overshoots above 1 before settling at 1.
4. Tracks use fixed duration, land exactly on target, and retarget without a
   discontinuity.
5. Settings exposes all four curves and navigation returns to Settings.
6. Existing and new host tests pass under C++11 with warnings treated as errors.
7. PlatformIO builds with serial input debug enabled and disabled.
