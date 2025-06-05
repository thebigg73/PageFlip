// The libraries used
#include <BleKeyboard.h>
extern "C" esp_err_t esp_wifi_stop(void);
#include <esp_bt.h>

// The button information that can be changed
// Each array should have the same number of entries
// The first array is the pins to use for the footswitch controls
// I have this code set up to use up to 5 switches on a foot pedal (you can use less)
const int buttonPins[] = {12,  // 1
                          14,  // 2
                          27,  // 3
                          26,  // 4
                          25}; // 5

// The next array is the keycode that is sent when each footswitch/pin above is pressed
const int keyCodes[] = {KEY_LEFT_ARROW,  // 1
                        KEY_RIGHT_ARROW, // 2
                        KEY_UP_ARROW,    // 3
                        KEY_DOWN_ARROW,  // 4
                        KEY_HOME};       // 5

// Finally the default button state when starting - each should be BUTTON_RELEASED
int buttonStates[buttonCount] = {BUTTON_RELEASED, // 1
                                BUTTON_RELEASED,  // 2
                                BUTTON_RELEASED,  // 3
                                BUTTON_RELEASED,  // 4 
                                BUTTON_RELEASED}; // 5

// You shouldn't need to change any of the code below unless you want to change how the pedal functions
bool debug = false;
#define DBG(...) if (debug) Serial.printf(__VA_ARGS__)

// The state for on/off as high/low states
constexpr int LED_ON  = HIGH;
constexpr int LED_OFF = LOW;

// Buttons pulled-up, so LOW means pressed
constexpr int BUTTON_PRESSED = LOW;  
constexpr int BUTTON_RELEASED = HIGH;
int connectedLEDState = LED_OFF;

// Check the array sizes match
constexpr size_t buttonCount = sizeof(buttonPins) / sizeof(buttonPins[0]);
constexpr size_t keyCount = sizeof(keyCodes) / sizeof(keyCodes[0]);
static_assert(buttonCount == keyCount, "Mismatch between buttonPins and keyCodes");

// The pins to use for the LEDs
constexpr int pinLEDPress = 33;
constexpr int pinLEDConnected = 32;

// The timings for flashes (when paired/not paired)
unsigned long previousMillis = 0;
const long notConnectedInterval = 500;
const long connectedInterval = 5000;
const long ledOnInterval = 50;

// The settings for registering and flashing when buttons are pressed
bool pressLEDActive = false;
unsigned long pressLEDMillis = 0;
constexpr unsigned long pressLEDDuration = 100;  // milliseconds

// Debounce tries to avoid flickers/multiple commands rapidly due to intermittent connections
unsigned long lastDebounceTime[buttonCount] = {0};
constexpr unsigned long debounceDelay = 20;

// Define the starting device name, manufacturer and initial battery charge
BleKeyboard bleKeyboard("PageFlip", "OpenSongApp", 100);

// Setup code initialises all of the settings for the pedal
void setup() {
  // Start the system and log this
  Serial.begin(115200);
  Serial.println("PageFlip Pedal starting...");
  Serial.flush();

  // Turn of WiFi for power optimisation
  esp_wifi_stop();

  // Set the mode of the led pins as outputs
  pinMode(pinLEDPress, OUTPUT);
  pinMode(pinLEDConnected, OUTPUT);

  // For each button pin, initialise it as a input_pullup
  for (size_t i = 0; i < buttonCount; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  // Start with the LEDs off
  digitalWrite(pinLEDPress, LED_OFF);
  digitalWrite(pinLEDConnected, connectedLEDState);

  // Use maximum Bluetooth power (better range).
  // If range isn't an issue, you can lower to ESP_PWR_LVL_P3 or ESP_PWR_LVL_DEFAULT
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P9);

  // Start the keyboard function and wait for a short period for initialisation
  bleKeyboard.begin();
  delay(100);
}

// This next part is the code that constantly runs in the background
void loop() {
  // The lastButtonState is part of the debounce check
  static int lastButtonState[buttonCount] = {BUTTON_RELEASED};

  // Go through the buttons and see if any have changed state on/off and haven't been a debouce
  for (size_t i = 0; i < buttonCount; i++) {
    int currentState = digitalRead(buttonPins[i]);

    if (currentState != lastButtonState[i]) {
      if ((millis() - lastDebounceTime[i]) > debounceDelay) {
        lastDebounceTime[i] = millis();
        lastButtonState[i] = currentState;
        
        // If a button was pressed, send the matching keycode as a keydown event
        if (currentState == BUTTON_PRESSED) {
          DBG("\nButton %d pressed", i + 1);
          sendKey(keyCodes[i], true);

        // Otherwise, send the matching keycode as a keyup event
        } else {
          DBG("\nButton %d released", i + 1);
          sendKey(keyCodes[i], false);
          triggerPressLED();
        }
      }
    }
  }

  // Update the led press to off and check the connected led pulse
  updatePressLED();
  connectedLEDPulse();
}

// Pulse the led to show it isn't paired (fast), or is connected (slow with quick flash)
void connectedLEDPulse() {
  unsigned long currentMillis = millis();
  int interval;

  if (bleKeyboard.isConnected()) {
    interval = (connectedLEDState == LED_ON) ? ledOnInterval : connectedInterval;
  } else {
    interval = notConnectedInterval;
  }

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    connectedLEDState = (connectedLEDState == LED_ON) ? LED_OFF : LED_ON;
    digitalWrite(pinLEDConnected, connectedLEDState);
  }
}

// Switch the button pressed led on
void triggerPressLED() {
  digitalWrite(pinLEDPress, LED_ON);
  pressLEDActive = true;
  pressLEDMillis = millis();
}

// Turn off the button pressed led after a pause
void updatePressLED() {
  if (pressLEDActive && (millis() - pressLEDMillis >= pressLEDDuration)) {
    digitalWrite(pinLEDPress, LED_OFF);
    pressLEDActive = false;
  }
}

// Send the keycode and keyup/keydown via Bluetooth
void sendKey(int keyCode, bool keyDown) {
  if (bleKeyboard.isConnected()) {
    if (keyDown) {
      bleKeyboard.press(keyCode);
    } else {
      bleKeyboard.release(keyCode);
    }
  }
}
