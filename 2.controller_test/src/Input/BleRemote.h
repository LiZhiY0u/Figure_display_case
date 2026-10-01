#ifndef INPUT_BLE_REMOTE_H
#define INPUT_BLE_REMOTE_H

#include "RemoteKeyInput.h"

namespace BleRemote
{
enum class ConnectionEvent : uint8_t
{
    GamepadConnected,
    GamepadDisconnected,
    MiniAppConnected,
    MiniAppDisconnected
};
void Notify(ConnectionEvent event);
bool ReadConnectionEvent(ConnectionEvent &event);
void Init();
void Update();
bool Read(RemoteKeyInput::Action &action);
} // namespace BleRemote

#endif
