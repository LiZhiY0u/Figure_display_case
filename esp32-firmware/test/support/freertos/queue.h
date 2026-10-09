#pragma once
#include "FreeRTOS.h"
#include <cassert>
#include <cstring>
#include <deque>
#include <vector>

namespace QueueFake {
struct Queue { size_t capacity, item_size; std::deque<std::vector<uint8_t>> items; };
extern bool fail_create;
extern Queue queue;
}
using QueueHandle_t = QueueFake::Queue*;
inline QueueHandle_t xQueueCreate(unsigned capacity, unsigned item_size)
{
    if (QueueFake::fail_create) return nullptr;
    QueueFake::queue = {capacity, item_size, {}};
    return &QueueFake::queue;
}
inline BaseType_t xQueueSend(QueueHandle_t queue, const void* item, TickType_t wait)
{
    assert(wait == 0);
    if (queue->items.size() == queue->capacity) return pdFALSE;
    const auto* data = static_cast<const uint8_t*>(item);
    queue->items.emplace_back(data, data + queue->item_size);
    return pdTRUE;
}
inline BaseType_t xQueueReceive(QueueHandle_t queue, void* item, TickType_t wait)
{
    assert(wait == 0);
    if (queue->items.empty()) return pdFALSE;
    std::memcpy(item, queue->items.front().data(), queue->item_size);
    queue->items.pop_front();
    return pdTRUE;
}
