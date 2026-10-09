# UI Menu Types Phase 2A Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make menu rendering semantics explicit and independent from display labels and row positions.

**Architecture:** An Arduino-independent menu model stores label, type, and data index. Existing static menu arrays populate that metadata, while the OLED list renderer switches on type and reads the explicit index; event handlers remain unchanged until phase 2B.

**Tech Stack:** C++11 host tests, Arduino C++, PlatformIO 6.1.19

**Spec:** `docs/superpowers/specs/2026-08-23-ui-menu-types-phase2a.md`

## Global Constraints

- C++11 compatible.
- No heap allocation or new dependency.
- No change to visible menu strings.
- No common action dispatcher in this phase.
- Existing serial debug mode remains controlled by the INI macro.

---

### Task 1: Menu Metadata Model

**Files:**
- Create: `2.controller_test/src/OledUI/MenuItem.h`
- Modify: `2.controller_test/src/OledUI/OledMenus.h`
- Test: `2.controller_test/test/menu_item/test_menu_item.cpp`

- [x] Write a failing test proving identical labels can resolve to different render types and explicit indices.
- [x] Compile with C++11 and verify failure because `MenuItem.h` is absent.
- [x] Implement `MenuItemType`, `MenuItemRenderMode`, `M_SELECT`, and `RenderModeFor()`.
- [x] Run the new and existing host tests without warnings.
- [x] Commit the menu model and test.

### Task 2: Migrate Menu Tables and Renderer

**Files:**
- Modify: `2.controller_test/src/OledUI/OledMenus.cpp`
- Modify: `2.controller_test/src/OledUI/OledUI.cpp`
- Test: `2.controller_test/test/menu_item/test_menu_item.cpp`

- [x] Extend the failing test for all renderer modes and index independence.
- [x] Verify RED before changing production behavior.
- [x] Assign explicit metadata to every menu item and replace prefix/row parsing in the renderer.
- [x] Run all host tests and scan for remaining prefix or `row - 1` behavior parsing.
- [x] Commit the migrated tables and renderer.

### Task 3: Build Matrix and Documentation

**Files:**
- Modify: `docs/superpowers/plans/2026-08-23-ui-menu-types-phase2a.md`

- [x] Run the final C++11 host suite.
- [x] Build PlatformIO with serial debug enabled.
- [x] Build PlatformIO cleanly with serial debug disabled, then restore enabled state.
- [x] Run `git diff --check` and confirm generated files remain ignored.
- [x] Mark this plan complete and commit the plan and spec.
