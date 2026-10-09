#pragma once

namespace BluetoothService {
// Initialize once, before any UI consumer. Returns false on queue failure.
bool Init();
void Update();
}
