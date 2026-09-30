#ifndef INPUT_BLE_REMOTE_H
#define INPUT_BLE_REMOTE_H

#include "RemoteKeyInput.h"

namespace BleRemote
{
void Init();
void Update();
bool Read(RemoteKeyInput::Action &action);
} // namespace BleRemote

#endif
