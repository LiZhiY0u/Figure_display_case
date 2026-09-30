#include "BleRemote.h"

#include <Arduino.h>
#include <Bluepad32.h>
#include <btstack.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "BleRemoteProfile.h"

namespace
{
QueueHandle_t key_queue = NULL;

const uint8_t advertising_data[] = {
    0x02, 0x01, 0x06,
    0x0b, 0x09,
    'F', 'i', 'g', 'u', 'r', 'e', 'C', 'a', 's', 'e',
};

void OnConnected(ControllerPtr controller)
{
    (void)controller;
}

void OnDisconnected(ControllerPtr controller)
{
    (void)controller;
}

int OnWrite(
    hci_con_handle_t connection_handle,
    uint16_t attribute_handle,
    uint16_t transaction_mode,
    uint16_t offset,
    uint8_t *buffer,
    uint16_t buffer_size)
{
    (void)connection_handle;
    (void)transaction_mode;

    if (attribute_handle != kBleRemoteWriteValueHandle ||
        offset != 0 ||
        buffer_size != 1 ||
        buffer == NULL ||
        key_queue == NULL)
        return 0;

    const RemoteKeyInput::Action action = RemoteKeyInput::Decode(buffer[0]);
    if (action != RemoteKeyInput::Action::None)
        xQueueSend(key_queue, &action, 0);

    return 0;
}
} // namespace

void BleRemote::Init()
{
    key_queue = xQueueCreate(8, sizeof(RemoteKeyInput::Action));
    if (key_queue == NULL)
    {
        Serial.println("BLE remote queue allocation failed");
        return;
    }

    BP32.setup(&OnConnected, &OnDisconnected);
    BP32.enableNewBluetoothConnections(false);

    att_server_init(kBleRemoteProfileData, NULL, OnWrite);

    bd_addr_t null_address = {0, 0, 0, 0, 0, 0};
    gap_advertisements_set_params(
        0x0030,
        0x0030,
        0,
        0,
        null_address,
        0x07,
        0x00);
    gap_advertisements_set_data(
        sizeof(advertising_data),
        const_cast<uint8_t *>(advertising_data));
    gap_advertisements_enable(1);

    Serial.println("BLE remote ready: FigureCase");
}

void BleRemote::Update()
{
    BP32.update();
}

bool BleRemote::Read(RemoteKeyInput::Action &action)
{
    return key_queue != NULL && xQueueReceive(key_queue, &action, 0) == pdTRUE;
}
