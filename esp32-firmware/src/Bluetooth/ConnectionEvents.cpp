#include "ConnectionEvents.h"
#include <atomic>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace {
QueueHandle_t queue = nullptr;
std::atomic<uint32_t> dropped(0);
}
bool ConnectionEvents::Init()
{
    if (queue == nullptr) queue = xQueueCreate(8, sizeof(Event));
    return queue != nullptr;
}
bool ConnectionEvents::Push(Event event)
{
    if (queue == nullptr) return false;
    if (xQueueSend(queue, &event, 0) == pdTRUE) return true;
    dropped.fetch_add(1, std::memory_order_relaxed);
    return false;
}
bool ConnectionEvents::Read(Event& event)
{
    return queue != nullptr && xQueueReceive(queue, &event, 0) == pdTRUE;
}
uint32_t ConnectionEvents::DroppedCount()
{
    return dropped.load(std::memory_order_relaxed);
}
