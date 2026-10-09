# UI Menu Types Phase 2A Design

## Status

Approved by the user on 2026-08-23.

## Goal

Replace menu rendering behavior inferred from label prefixes with explicit,
statically allocated menu metadata while preserving all visible labels and
page behavior.

## Scope

- Add an Arduino-independent `MenuItemType` and menu item structure.
- Give every menu item an explicit rendering type.
- Give value, toggle, choice, and binding rows an explicit data index.
- Render list tails from metadata instead of `m_select[0]` and `row - 1`.
- Preserve existing page handlers, labels, navigation, animations, and modal behavior.

## Constraints

- C++11 compatible.
- No heap allocation or new dependency.
- No change to visible menu strings.
- No common action dispatcher in this phase.
- Existing serial debug mode remains controlled by the INI macro.

## Acceptance Criteria

1. Changing a label prefix does not change its render type.
2. No production code reads `m_select[0]` to determine behavior.
3. No list-tail renderer derives a parameter index with `row - 1`.
4. Every existing menu item has an explicit type.
5. Existing navigation and startup tests pass.
6. New menu metadata tests pass under C++11 without warnings.
7. PlatformIO builds with serial debug enabled and disabled.
