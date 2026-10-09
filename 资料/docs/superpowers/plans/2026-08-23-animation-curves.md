# Runtime Animation Curves Implementation Plan

**Goal:** Add runtime-selectable, fixed-duration easing curves to OLED UI motion.

**Spec:** `docs/superpowers/specs/2026-08-23-animation-curves.md`

## Task 1: Animation Core

- [x] Add failing C++11 tests for curve endpoints, midpoints, overshoot, fixed
  duration, completion, retargeting, and timer wraparound.
- [x] Implement the Arduino-independent curve evaluator and animation track.
- [x] Run all host tests warning-free and commit.

## Task 2: UI Animation Integration

- [x] Add one track per animated Tile, List, Window, and Voltage property.
- [x] Convert existing speed values to monotonic durations.
- [x] Reset tracks whenever page lifecycle code directly resets animated values.
- [x] Preserve exact-target completion checks used by existing rendering logic.
- [x] Run host tests and PlatformIO build, then commit.

## Task 3: Runtime Curve Selection

- [x] Add the Anim Curve child page and explicit typed menu metadata.
- [x] Add Settings navigation, selection state, and parent mapping.
- [x] Default to Out Cubic and apply selection immediately.
- [x] Extend navigation tests and compile the complete firmware.
- [x] Commit.

## Task 4: Verification

- [x] Run the complete C++11 host suite with warnings as errors.
- [x] Build with serial input debug enabled.
- [x] Clean-build with serial input debug disabled and restore the enabled default.
- [x] Confirm generated PlatformIO state is ignored and the worktree is clean.
- [x] Mark this plan complete and commit the documentation.
