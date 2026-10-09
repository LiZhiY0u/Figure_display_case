#include <cassert>
#include <vector>
#include "Bluetooth/BluetoothService.h"
#include "Bluetooth/ConnectionEvents.h"
#include "Bluetooth/GamepadController.h"
#include "Bluetooth/MiniAppRemote.h"
namespace {
bool queue_ready = false;
std::vector<int> calls;
}
namespace ConnectionEvents { bool Init() { calls.push_back(1); return queue_ready; } }
namespace GamepadController {
void Init() { calls.push_back(2); }
void Update() { calls.push_back(4); }
}
namespace MiniAppRemote { void Init() { calls.push_back(3); } }
int main()
{
    BluetoothService::Update();
    assert(calls.empty());
    assert(!BluetoothService::Init());
    assert(calls == std::vector<int>({1}));
    BluetoothService::Update();
    assert(calls.size() == 1);
    queue_ready = true;
    calls.clear();
    assert(BluetoothService::Init());
    assert(calls == std::vector<int>({1, 2, 3}));
    assert(BluetoothService::Init());
    assert(calls.size() == 3);
    BluetoothService::Update();
    assert(calls == std::vector<int>({1, 2, 3, 4}));
}
