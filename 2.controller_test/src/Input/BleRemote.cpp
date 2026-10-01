#include "BleRemote.h"

#include <Arduino.h>
#include <Bluepad32.h>
#include <btstack.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "BleRemoteProfile.h"
#include "Controller/Controller.h"

namespace
{
QueueHandle_t key_queue = NULL;
QueueHandle_t connection_queue = NULL;
hci_con_handle_t mini_app_handle = HCI_CON_HANDLE_INVALID;

const uint8_t advertising_data[] = {
    0x02, 0x01, 0x06,
    0x0b, 0x09,
    'F', 'i', 'g', 'u', 'r', 'e', 'C', 'a', 's', 'e',
};

void OnAttEvent(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    (void)channel;
    (void)size;
    if (packet_type != HCI_EVENT_PACKET)
        return;
    switch (hci_event_packet_get_type(packet))
    {
    case ATT_EVENT_CONNECTED:
    {
        const hci_con_handle_t handle = att_event_connected_get_handle(packet);
        // Gamepad ATT links are not phone connections. The phone is central;
        // ESP32 must be the LE peripheral for this notification.
        if (gap_get_connection_type(handle) == GAP_CONNECTION_LE &&
            gap_get_role(handle) == HCI_ROLE_SLAVE &&
            mini_app_handle == HCI_CON_HANDLE_INVALID)
        {
            mini_app_handle = handle;
            BleRemote::Notify(BleRemote::ConnectionEvent::MiniAppConnected);
        }
        break;
    }
    case ATT_EVENT_DISCONNECTED:
        if (att_event_disconnected_get_handle(packet) == mini_app_handle)
        {
            mini_app_handle = HCI_CON_HANDLE_INVALID;
            BleRemote::Notify(BleRemote::ConnectionEvent::MiniAppDisconnected);
            gap_advertisements_enable(1);
        }
        break;
    default:
        break;
    }
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
    connection_queue = xQueueCreate(8, sizeof(ConnectionEvent));
    if (key_queue == NULL || connection_queue == NULL)
    {
        Serial.println("BLE remote queue allocation failed");
        return;
    }

    Controller_init();
    BP32.enableNewBluetoothConnections(true);

    att_server_init(kBleRemoteProfileData, NULL, OnWrite);
    att_server_register_packet_handler(OnAttEvent);

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
    Controller_loop();
}

void BleRemote::Notify(ConnectionEvent event)
{
    if (connection_queue != NULL)
        xQueueSend(connection_queue, &event, 0);
}

bool BleRemote::ReadConnectionEvent(ConnectionEvent &event)
{
    return connection_queue != NULL &&
        xQueueReceive(connection_queue, &event, 0) == pdTRUE;
}

bool BleRemote::Read(RemoteKeyInput::Action &action)
{
    return key_queue != NULL && xQueueReceive(key_queue, &action, 0) == pdTRUE;
}
