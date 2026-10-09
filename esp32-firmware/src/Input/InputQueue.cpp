#include "InputQueue.h"
#include <atomic>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace {
constexpr unsigned Capacity = 16;
QueueHandle_t queue = nullptr;
std::atomic<uint32_t> dropped(0);
}

bool InputQueue::Init()
{
    if (queue == nullptr)
        queue = xQueueCreate(Capacity, sizeof(Input::Event));
    return queue != nullptr;
}

bool InputQueue::Push(const Input::Event& event)
{
    if (queue == nullptr || event.action == Input::Action::None)
        return false;
    if (xQueueSend(queue, &event, 0) == pdTRUE)
        return true;
    dropped.fetch_add(1, std::memory_order_relaxed);
    return false;
}

bool InputQueue::Read(Input::Event& event)
{
    return queue != nullptr && xQueueReceive(queue, &event, 0) == pdTRUE;
}

uint32_t InputQueue::DroppedCount()
{
    return dropped.load(std::memory_order_relaxed);
}
