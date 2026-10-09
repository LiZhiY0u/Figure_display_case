# UI Navigation Phase 1 Design

## Status

Approved by the user on 2026-08-23.

## Goal

Make page transitions and page-entry initialization consistent so cold boot,
wake, forward navigation, and back navigation cannot silently skip required
page state setup.

## Scope

- Add typed, host-testable page navigation primitives.
- Centralize the parent relationship for every non-modal page.
- Provide `NavigateTo()`, `Back()`, and `OpenRoot()` as the only page navigation
  entry points used by page processors.
- Centralize page-specific initialization and call it for boot, wake, forward
  navigation, and back navigation.
- Reject navigation beyond `UI_DEPTH` and reject back navigation at the root.
- Preserve the current tile, list, fade, window, input, and rendering behavior.

## Constraints

- No heap allocation.
- No new runtime, SDK, package, or library.
- Keep the existing `UI_Context` storage layout and `uint8_t` page/state fields.
- Keep the modal value window separate from hierarchical page navigation.
- Keep generated `.pio/` state untracked.

## Acceptance Criteria

1. Cold boot and wake both open `M_MAIN` through the same root-page lifecycle.
2. Every hierarchical page transition uses `NavigateTo()` or `Back()`.
3. `Back()` derives its destination from one centralized parent map.
4. Entering at `UI_DEPTH - 1` is rejected without changing page or state.
5. Back at layer zero is rejected without underflow or state mutation.
6. Existing host tests pass.
7. Navigation host tests pass with warning-free compilation.
8. PlatformIO builds successfully with serial debug mode enabled and disabled.
