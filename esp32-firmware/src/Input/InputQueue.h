#pragma once
#include "InputEvent.h"

// Task-context API only. Initialize before starting any producers.
namespace InputQueue {
bool Init();
bool Push(const Input::Event& event);
bool Read(Input::Event& event);
uint32_t DroppedCount();
}
