#include "MiniAppRemote.h"

#include <Arduino.h>
#include <btstack.h>

#include "MiniAppProfile.h"
#include "MiniAppConnection.h"
#include "ConnectionEvents.h"
#include "Configs/BluetoothConfig.h"
#include "Input/InputQueue.h"
#include "Input/InputAdapters.h"

namespace
{
MiniAppConnection::State connection;

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
        if (connection.Connect(handle, gap_get_connection_type(handle) == GAP_CONNECTION_LE,
                               gap_get_role(handle) == HCI_ROLE_SLAVE))
        {
            ConnectionEvents::Push(ConnectionEvents::Event::MiniAppConnected);
        }
        break;
    }
    case ATT_EVENT_DISCONNECTED:
        if (connection.Disconnect(att_event_disconnected_get_handle(packet)))
        {
            ConnectionEvents::Push(ConnectionEvents::Event::MiniAppDisconnected);
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
        buffer == NULL)
        return 0;

    const UserInput::Action action = InputAdapters::MiniApp(buffer[0]);
    if (action != UserInput::Action::None)
        InputQueue::Push({action, UserInput::Source::MiniApp, millis()});

    return 0;
}
} // namespace

void MiniAppRemote::Init()
{
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
        sizeof(BluetoothConfig::AdvertisingData),
        const_cast<uint8_t *>(BluetoothConfig::AdvertisingData));
    gap_advertisements_enable(1);

    Serial.println("BLE remote ready: FigureCase");
}
