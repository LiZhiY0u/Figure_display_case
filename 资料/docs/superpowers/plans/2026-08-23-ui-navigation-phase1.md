# UI Navigation Phase 1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Introduce consistent, bounded page navigation and page-entry lifecycle handling for the OLED UI.

**Architecture:** Page and transition enums move into an Arduino-independent type header. A static navigation module owns parent routes and performs root, enter, and back state transitions against the existing `UI_Context`; `OledUI.cpp` supplies page-specific initialization callbacks and rendering-state resets.

**Tech Stack:** C++17 host tests, Arduino C++ for ESP32, PlatformIO 6.1.19

**Spec:** `docs/superpowers/specs/2026-08-23-ui-navigation-phase1.md`

## Global Constraints

- No heap allocation.
- No new runtime, SDK, package, or library.
- Keep the existing `UI_Context` storage layout and `uint8_t` page/state fields.
- Keep the modal value window separate from hierarchical page navigation.
- Keep generated `.pio/` state untracked.

---

### Task 1: Typed Navigation Core

**Files:**
- Create: `2.controller_test/src/OledUI/UiTypes.h`
- Create: `2.controller_test/src/OledUI/UiNavigation.h`
- Modify: `2.controller_test/src/OledUI/OledUI.h`
- Test: `2.controller_test/test/ui_navigation/test_ui_navigation.cpp`

**Interfaces:**
- Produces: `UI_PAGE`, `UI_STATE`, `UiNavigation::ParentOf()`, `OpenRoot()`, `RequestEnter()`, `CompleteEnter()`, `RequestBack()`, and `CompleteBack()`.
- Consumes: Existing `UI_Context` fields `index`, `state`, `layer`, and `select`.

- [x] **Step 1: Write the failing navigation test**

  Add assertions that the parent map is correct, root opening resets layer and
  invokes initialization, forward navigation is accepted below the limit,
  maximum-depth entry is rejected, normal back selects the mapped parent, and
  root back is rejected.

- [x] **Step 2: Run the test to verify RED**

  Run:

  ```powershell
  & 'E:\clion_for_stm32\MinGW\bin\g++.exe' -std=c++17 -Wall -Wextra -pedantic -I src test/ui_navigation/test_ui_navigation.cpp -o $env:TEMP\test_ui_navigation.exe
  ```

  Expected: compilation fails because `OledUI/UiNavigation.h` does not exist.

- [x] **Step 3: Implement the minimal navigation core**

  Define page/state enums without Arduino dependencies, a `constexpr` parent
  map, and template transition functions that mutate the existing context only
  after their boundary checks pass. All functions return a result that callers
  can inspect without exceptions.

- [x] **Step 4: Run the navigation and existing host tests**

  Compile each test with `-Wall -Wextra -pedantic`, run each executable, and
  require exit code zero with no compiler warnings.

- [x] **Step 5: Commit the navigation core**

  ```powershell
  git add 2.controller_test/src/OledUI/UiTypes.h 2.controller_test/src/OledUI/UiNavigation.h 2.controller_test/src/OledUI/OledUI.h 2.controller_test/test/ui_navigation/test_ui_navigation.cpp
  git commit -m "feat: add bounded UI navigation core"
  ```

### Task 2: Integrate Navigation and Page Lifecycle

**Files:**
- Modify: `2.controller_test/src/OledUI/OledUI.cpp`
- Modify: `2.controller_test/src/OledUI/OledUI.h`
- Test: `2.controller_test/test/ui_navigation/test_ui_navigation.cpp`

**Interfaces:**
- Consumes: Navigation primitives from Task 1.
- Produces: `OledUI::NavigateTo(UI_PAGE)`, `OledUI::Back()`, and `OledUI::OpenRoot(UI_PAGE)`.

- [x] **Step 1: Extend the failing test for lifecycle completion**

  Assert that completing enter increments the layer once, clears the new
  layer selection, calls the page initializer once, and selects `S_FADE`.
  Assert that completing back clears the departed layer, decrements once,
  initializes the parent once, and selects `S_FADE`.

- [x] **Step 2: Run the test to verify RED**

  Compile and run the navigation test. Expected: failure because lifecycle
  completion functions or their required behavior are absent.

- [x] **Step 3: Integrate the lifecycle**

  Add one page-initializer dispatcher in `OledUI.cpp`. Route boot and wake
  through `OpenRoot(M_MAIN)`, route page selections through `NavigateTo()`, and
  route hierarchical exits through `Back()`. Preserve `Window_Value_Init()` as
  a modal transition and preserve existing animation resets.

- [x] **Step 4: Run every host test**

  Build and run `ui_navigation`, `main_page_state`, and `serial_debug_input`.
  Require all tests to exit zero without warnings.

- [x] **Step 5: Commit lifecycle integration**

  ```powershell
  git add 2.controller_test/src/OledUI/OledUI.cpp 2.controller_test/src/OledUI/OledUI.h 2.controller_test/test/ui_navigation/test_ui_navigation.cpp
  git commit -m "refactor: centralize OLED page lifecycle"
  ```

### Task 3: Full Build Matrix and Documentation Verification

**Files:**
- Modify: `docs/superpowers/plans/2026-08-23-ui-navigation-phase1.md`

**Interfaces:**
- Consumes: Completed navigation and lifecycle implementation.
- Produces: Verification evidence for host behavior and both PlatformIO macro configurations.

- [x] **Step 1: Run final host tests**

  Compile and execute all three host tests with warnings enabled.

- [x] **Step 2: Build with serial debug enabled**

  Run the configured PlatformIO environment with
  `ENABLE_SERIAL_INPUT_DEBUG=1` and require `[SUCCESS]`.

- [x] **Step 3: Build with serial debug disabled**

  Temporarily set the INI macro to `0`, perform a clean PlatformIO build,
  require `[SUCCESS]`, and restore the checked-in value to `1`.

- [x] **Step 4: Review acceptance criteria and repository status**

  Run `git diff --check`, confirm `.pio/` is absent from `git status`, and
  compare every acceptance criterion against test or build evidence.

- [x] **Step 5: Commit verification records**

  Mark completed checklist items in this plan, then commit the plan and spec:

  ```powershell
  git add docs/superpowers/plans/2026-08-23-ui-navigation-phase1.md docs/superpowers/specs/2026-08-23-ui-navigation-phase1.md
  git commit -m "docs: record UI navigation phase one plan"
  ```
