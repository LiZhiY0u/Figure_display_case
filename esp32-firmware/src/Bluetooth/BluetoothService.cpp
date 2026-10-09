#include "BluetoothService.h"
#include "ConnectionEvents.h"
#include "GamepadController.h"
#include "MiniAppRemote.h"

namespace { bool ready = false; }

bool BluetoothService::Init()
{
    if (ready) return true;
    if (!ConnectionEvents::Init()) return false;
    GamepadController::Init();
    MiniAppRemote::Init();
    ready = true;
    return true;
}

void BluetoothService::Update()
{
    if (ready) GamepadController::Update();
}
