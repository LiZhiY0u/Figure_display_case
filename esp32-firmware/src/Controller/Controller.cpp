#include "Controller/Controller.h"
#include "Input/BleRemote.h"

#ifndef ENABLE_GAMEPAD_INPUT_LOG
#define ENABLE_GAMEPAD_INPUT_LOG 0
#endif

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

uint8_t gamepad_axis = 0;

// Target gamepad: C0:D6:D5:EE:DB:8A. This filter applies only to gamepads.
static const uint8_t target_gamepad_address[6] = {0xC0, 0xD6, 0xD5, 0xEE, 0xDB, 0x8A};
static bool gamepad_notified[BP32_MAX_GAMEPADS] = {};
static bool gamepad_a_down[BP32_MAX_GAMEPADS] = {};
static uint8_t gamepad_stick_direction[BP32_MAX_GAMEPADS] = {};
static uint8_t gamepad_scroll_direction[BP32_MAX_GAMEPADS] = {};
static uint32_t gamepad_repeat_started[BP32_MAX_GAMEPADS] = {};
static uint32_t gamepad_repeat_wait[BP32_MAX_GAMEPADS] = {};
static const uint32_t kScrollInitialDelayMs = 400;
static const uint32_t kScrollRepeatIntervalMs = 150;
#if ENABLE_GAMEPAD_INPUT_LOG
struct GamepadLogState
{
    bool valid;
    uint16_t buttons;
    uint8_t dpad;
    uint16_t misc;
    int32_t x, y, rx, ry, brake, throttle;
    uint32_t time;
};
static GamepadLogState gamepad_logs[BP32_MAX_GAMEPADS] = {};
#endif

static bool IsTargetGamepad(ControllerPtr ctl)
{
    const ControllerProperties properties = ctl->getProperties();
    for (uint8_t i = 0; i < 6; ++i)
        if (properties.btaddr[i] != target_gamepad_address[i])
            return false;
    return true;
}

// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedController(ControllerPtr ctl)
{
    if (ctl->isGamepad())
    {
        const ControllerProperties properties = ctl->getProperties();
        for (uint8_t i = 0; i < 6; ++i)
        {
            if (properties.btaddr[i] != target_gamepad_address[i])
            {
                ctl->disconnect();
                return;
            }
        }
    }
    bool foundEmptySlot = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++)
    {
        if (myControllers[i] == nullptr)
        {
            Serial.printf("CALLBACK: Controller is connected, index=%d\n", i);
            // Additionally, you can get certain gamepad properties like:
            // Model, VID, PID, BTAddr, flags, etc.
            ControllerProperties properties = ctl->getProperties();
            Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id,
                          properties.product_id);
            myControllers[i] = ctl;
#if ENABLE_GAMEPAD_INPUT_LOG
            gamepad_logs[i].valid = false;
#endif
            gamepad_a_down[i] = false;
            gamepad_stick_direction[i] = 0;
            gamepad_scroll_direction[i] = 0;
            gamepad_notified[i] = ctl->isGamepad();
            if (ctl->isGamepad())
                BleRemote::Notify(BleRemote::ConnectionEvent::GamepadConnected);
            foundEmptySlot = true;
            break;
        }
    }
    if (!foundEmptySlot)
    {
        Serial.println("CALLBACK: Controller connected, but could not found empty slot");
    }
}

void onDisconnectedController(ControllerPtr ctl)
{
    bool foundController = false;

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++)
    {
        if (myControllers[i] == ctl)
        {
            Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
            if (gamepad_notified[i])
                BleRemote::Notify(BleRemote::ConnectionEvent::GamepadDisconnected);
            gamepad_notified[i] = false;
            gamepad_a_down[i] = false;
            gamepad_stick_direction[i] = 0;
            gamepad_scroll_direction[i] = 0;
            myControllers[i] = nullptr;
            foundController = true;
            break;
        }
    }

    if (!foundController)
    {
        Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
    }
}

void dumpGamepad(ControllerPtr ctl)
{
    int slot = -1;
    for (int i = 0; i < BP32_MAX_GAMEPADS; ++i)
        if (myControllers[i] == ctl)
            slot = i;
    if (slot < 0)
        return;
    // Track every report, independently of the rate-limited debug log.
    const bool a_down = ctl->a();
    const bool confirm_pressed = a_down && !gamepad_a_down[slot];
    gamepad_a_down[slot] = a_down;
    const uint32_t now = millis();
    // Rearm only after both axes return to the center dead zone.
    // Separate engage/release thresholds prevent jitter from retriggering.
    if (abs(ctl->axisX()) <= 200 && abs(ctl->axisY()) <= 200)
        gamepad_stick_direction[slot] = 0;
    else if (ctl->axisX() > 500 || ctl->axisY() > 500)
        gamepad_stick_direction[slot] = 2;
    else if (ctl->axisX() < -500 || ctl->axisY() < -500)
        gamepad_stick_direction[slot] = 1;
    // Bluepad32 D-pad: up=0x01, down=0x02, right=0x04, left=0x08.
    // Match the mini-app's Previous/Next UI mapping (axis 2/1 respectively).
    const uint8_t dpad = ctl->dpad();
    uint8_t direction = gamepad_stick_direction[slot];
    if (dpad != 0)
    {
        const bool previous = (dpad & 0x09) != 0;
        const bool next = (dpad & 0x06) != 0;
        direction = previous == next ? 0 : previous ? 2 : 1;
    }
    // D-pad takes priority; both sources share one repeat clock.
    if (direction != gamepad_scroll_direction[slot])
    {
        gamepad_scroll_direction[slot] = direction;
        gamepad_repeat_started[slot] = now;
        gamepad_repeat_wait[slot] = kScrollInitialDelayMs;
        if (direction != 0)
            gamepad_axis = direction;
    }
    if (confirm_pressed)
    {
        gamepad_axis = 3;
    }
    else
    {
        // gamepad_axis = 0;
    }
#if ENABLE_GAMEPAD_INPUT_LOG
    GamepadLogState &last = gamepad_logs[slot];
    const bool digital_changed = !last.valid || last.buttons != ctl->buttons() ||
        last.dpad != ctl->dpad() || last.misc != ctl->miscButtons();
    const bool analog_changed = !last.valid ||
        abs(last.x - ctl->axisX()) >= 16 || abs(last.y - ctl->axisY()) >= 16 ||
        abs(last.rx - ctl->axisRX()) >= 16 || abs(last.ry - ctl->axisRY()) >= 16 ||
        abs(last.brake - ctl->brake()) >= 16 || abs(last.throttle - ctl->throttle()) >= 16 ||
        (last.x != 0 && ctl->axisX() == 0) || (last.y != 0 && ctl->axisY() == 0);
    if (!digital_changed && !(analog_changed && now - last.time >= 100u))
        return;

    const uint16_t pressed = ctl->buttons() & ~(last.valid ? last.buttons : 0);
    const uint16_t released = (last.valid ? last.buttons : 0) & ~ctl->buttons();
    const int mapped = ctl->a() ? 3 :
        ((dpad & 0x09) != 0 && (dpad & 0x06) == 0) ? 2 :
        ((dpad & 0x06) != 0 && (dpad & 0x09) == 0) ? 1 :
        (ctl->axisX() > 500 || ctl->axisY() > 500) ? 2 :
        (ctl->axisX() < -500 || ctl->axisY() < -500) ? 1 : 0;
    Serial.printf("[PAD %lu] idx=%d btn=%04X press=%04X release=%04X dpad=%02X misc=%02X "
                  "L=(%ld,%ld) R=(%ld,%ld) trigger=(%ld,%ld) map=%d\n",
                  static_cast<unsigned long>(now), ctl->index(), ctl->buttons(), pressed, released,
                  ctl->dpad(), ctl->miscButtons(), static_cast<long>(ctl->axisX()),
                  static_cast<long>(ctl->axisY()), static_cast<long>(ctl->axisRX()),
                  static_cast<long>(ctl->axisRY()), static_cast<long>(ctl->brake()),
                  static_cast<long>(ctl->throttle()), mapped);
    last = {true, ctl->buttons(), ctl->dpad(), ctl->miscButtons(), ctl->axisX(),
            ctl->axisY(), ctl->axisRX(), ctl->axisRY(), ctl->brake(), ctl->throttle(), now};
#endif
}

void dumpMouse(ControllerPtr ctl)
{
    Serial.printf("idx=%d, buttons: 0x%04x, scrollWheel=0x%04x, delta X: %4d, delta Y: %4d\n",
                  ctl->index(),       // Controller Index
                  ctl->buttons(),     // bitmask of pressed buttons
                  ctl->scrollWheel(), // Scroll Wheel
                  ctl->deltaX(),      // (-511 - 512) left X Axis
                  ctl->deltaY()       // (-511 - 512) left Y axis
    );
}

void dumpKeyboard(ControllerPtr ctl)
{
    static const char *key_names[] = {
        // clang-format off
        // To avoid having too much noise in this file, only a few keys are mapped to strings.
        // Starts with "A", which is offset 4.
        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V",
        "W", "X", "Y", "Z", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
        // Special keys
        "Enter", "Escape", "Backspace", "Tab", "Spacebar", "Underscore", "Equal", "OpenBracket", "CloseBracket",
        "Backslash", "Tilde", "SemiColon", "Quote", "GraveAccent", "Comma", "Dot", "Slash", "CapsLock",
        // Function keys
        "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
        // Cursors and others
        "PrintScreen", "ScrollLock", "Pause", "Insert", "Home", "PageUp", "Delete", "End", "PageDown",
        "RightArrow", "LeftArrow", "DownArrow", "UpArrow",
        // clang-format on
    };
    static const char *modifier_names[] = {
        // clang-format off
        // From 0xe0 to 0xe7
        "Left Control", "Left Shift", "Left Alt", "Left Meta",
        "Right Control", "Right Shift", "Right Alt", "Right Meta",
        // clang-format on
    };
    Serial.printf("idx=%d, Pressed keys: ", ctl->index());
    for (int key = Keyboard_A; key <= Keyboard_UpArrow; key++)
    {
        if (ctl->isKeyPressed(static_cast<KeyboardKey>(key)))
        {
            const char *keyName = key_names[key - 4];
            Serial.printf("%s,", keyName);
        }
    }
    for (int key = Keyboard_LeftControl; key <= Keyboard_RightMeta; key++)
    {
        if (ctl->isKeyPressed(static_cast<KeyboardKey>(key)))
        {
            const char *keyName = modifier_names[key - 0xe0];
            Serial.printf("%s,", keyName);
        }
    }
    Console.printf("\n");
}

void dumpBalanceBoard(ControllerPtr ctl)
{
    Serial.printf("idx=%d,  TL=%u, TR=%u, BL=%u, BR=%u, temperature=%d\n",
                  ctl->index(),       // Controller Index
                  ctl->topLeft(),     // top-left scale
                  ctl->topRight(),    // top-right scale
                  ctl->bottomLeft(),  // bottom-left scale
                  ctl->bottomRight(), // bottom-right scale
                  ctl->temperature()  // temperature: used to adjust the scale value's precision
    );
}

void processGamepad(ControllerPtr ctl)
{
    // There are different ways to query whether a button is pressed.
    // By query each button individually:
    //  a(), b(), x(), y(), l1(), etc...
    if (ctl->a())
    {
        static int colorIdx = 0;
        // Some gamepads like DS4 and DualSense support changing the color LED.
        // It is possible to change it by calling:
        switch (colorIdx % 3)
        {
        case 0:
            // Red
            ctl->setColorLED(255, 0, 0);
            break;
        case 1:
            // Green
            ctl->setColorLED(0, 255, 0);
            break;
        case 2:
            // Blue
            ctl->setColorLED(0, 0, 255);
            break;
        }
        colorIdx++;
    }

    if (ctl->b())
    {
        // Turn on the 4 LED. Each bit represents one LED.
        static int led = 0;
        led++;
        // Some gamepads like the DS3, DualSense, Nintendo Wii, Nintendo Switch
        // support changing the "Player LEDs": those 4 LEDs that usually indicate
        // the "gamepad seat".
        // It is possible to change them by calling:
        ctl->setPlayerLEDs(led & 0x0f);
    }

    if (ctl->x())
    {
        // Some gamepads like DS3, DS4, DualSense, Switch, Xbox One S, Stadia support rumble.
        // It is possible to set it by calling:
        // Some controllers have two motors: "strong motor", "weak motor".
        // It is possible to control them independently.
        ctl->playDualRumble(0 /* delayedStartMs */, 250 /* durationMs */, 0x80 /* weakMagnitude */,
                            0x40 /* strongMagnitude */);
    }

    // Another way to query controller data is by getting the buttons() function.
    // See how the different "dump*" functions dump the Controller info.
    dumpGamepad(ctl);
}

void processMouse(ControllerPtr ctl)
{
    // This is just an example.
    if (ctl->scrollWheel() > 0)
    {
        // Do Something
    }
    else if (ctl->scrollWheel() < 0)
    {
        // Do something else
    }

    // See "dumpMouse" for possible things to query.
    dumpMouse(ctl);
}

void processKeyboard(ControllerPtr ctl)
{
    if (!ctl->isAnyKeyPressed())
        return;

    // This is just an example.
    if (ctl->isKeyPressed(Keyboard_A))
    {
        // Do Something
        Serial.println("Key 'A' pressed");
    }

    // Don't do "else" here.
    // Multiple keys can be pressed at the same time.
    if (ctl->isKeyPressed(Keyboard_LeftShift))
    {
        // Do something else
        Serial.println("Key 'LEFT SHIFT' pressed");
    }

    // Don't do "else" here.
    // Multiple keys can be pressed at the same time.
    if (ctl->isKeyPressed(Keyboard_LeftArrow))
    {
        // Do something else
        Serial.println("Key 'Left Arrow' pressed");
    }

    // See "dumpKeyboard" for possible things to query.
    dumpKeyboard(ctl);
}

void processBalanceBoard(ControllerPtr ctl)
{
    // This is just an example.
    if (ctl->topLeft() > 10000)
    {
        // Do Something
    }

    // See "dumpBalanceBoard" for possible things to query.
    dumpBalanceBoard(ctl);
}

void processControllers()
{
    for (int i = 0; i < BP32_MAX_GAMEPADS; ++i)
    {
        ControllerPtr myController = myControllers[i];
        // On first pairing, the connection callback can precede class data.
        if (myController && myController->isConnected() &&
            myController->isGamepad() && !gamepad_notified[i])
        {
            if (!IsTargetGamepad(myController))
            {
                myController->disconnect();
                continue;
            }
            gamepad_notified[i] = true;
            BleRemote::Notify(BleRemote::ConnectionEvent::GamepadConnected);
            Serial.printf("Gamepad ready: connection notification, index=%d\n", i);
        }
        if (myController && myController->isConnected() && myController->hasData())
        {
            if (myController->isGamepad())
            {
                processGamepad(myController);
            }
            else if (myController->isMouse())
            {
                processMouse(myController);
            }
            else if (myController->isKeyboard())
            {
                processKeyboard(myController);
            }
            else if (myController->isBalanceBoard())
            {
                processBalanceBoard(myController);
            }
            else
            {
                Serial.println("Unsupported controller");
            }
        }
        // Poll even without a new report: some controllers only report changes.
        const uint32_t now = millis();
        if (myController && myController->isConnected() && gamepad_scroll_direction[i] != 0 &&
            now - gamepad_repeat_started[i] >= gamepad_repeat_wait[i])
        {
            if (gamepad_axis == 0)
            {
                gamepad_axis = gamepad_scroll_direction[i];
                gamepad_repeat_started[i] = now;
                gamepad_repeat_wait[i] = kScrollRepeatIntervalMs;
            }
        }
    }
}

void Controller_init()
{
    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());

    const uint8_t *addr = BP32.localBdAddress();
    Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    // Setup the Bluepad32 callbacks
    BP32.setup(&onConnectedController, &onDisconnectedController);

    // "forgetBluetoothKeys()" should be called when the user performs
    // a "device factory reset", or similar.
    // Calling "forgetBluetoothKeys" in setup() just as an example.
    // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
    // But it might also fix some connection / re-connection issues.
    // Preserve pairing keys so known gamepads can reconnect after reboot.

    // Enables mouse / touchpad support for gamepads that support them.
    // When enabled, controllers like DualSense and DualShock4 generate two connected devices:
    // - First one: the gamepad
    // - Second one, which is a "virtual device", is a mouse.
    // By default, it is disabled.
    BP32.enableVirtualDevice(false);
}

void Controller_loop()
{
    BP32.update();
    processControllers();
}
