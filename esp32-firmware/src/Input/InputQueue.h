#pragma once
#include "InputEvent.h"

// Task-context API only. Initialize before starting any producers.
namespace InputQueue {
bool Init();
bool Push(const UserInput::Event& event);
bool Read(UserInput::Event& event);
uint32_t DroppedCount();
}
