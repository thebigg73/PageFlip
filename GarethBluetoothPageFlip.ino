#include <BleKeyboard.h>

#include "esp_wifi.h"
#include <esp_bt.h>

// Set up the pin numbers for the buttons
const int pinButton1 = 12;
const int pinButton2 = 14;
const int pinButton3 = 27;
const int pinButton4 = 26;
const int pinButton5 = 25;

// Set up the pin numbers for the LEDs
// const int pinLEDOnboard = 36;
const int pinLEDPress = 33;
const int pinLEDConnected = 32;

// Because we connect to GND we use reverse HIGH/LOW logic
// HIGH = off, LOW = on
// Set up the pin states (track changes)
int button1State = HIGH;
int button2State = HIGH;
int button3State = HIGH;
int button4State = HIGH;
int button5State = HIGH;
int connectedLEDState = HIGH;
int pressedLEDState = HIGH;

// Variables for blinking the Bluetooth LED
// The last time LED was updated
unsigned long previousMillis = 0;
// Interval at which to blink (milliseconds)
const long notconnectedInterval = 500;
const long connectedInterval = 5000;
const long ledOnInterval = 50;

// First param is name of the pedal
// Second is manufacturer
// Third is initial battery level
BleKeyboard bleKeyboard("PageFlip", "OpenSongApp", 100);

void setup() {
  Serial.begin(115200);
  //pinMode(pinLEDOnboard, OUTPUT);
  
  // Disable WiFi
  esp_wifi_stop();
  
  // Initialize Bluetooth (example)
  btStop();

  pinMode(pinLEDPress,OUTPUT);
  pinMode(pinLEDConnected,OUTPUT);
  pinMode(pinButton1,INPUT_PULLUP);
  pinMode(pinButton2,INPUT_PULLUP);
  pinMode(pinButton3,INPUT_PULLUP);
  pinMode(pinButton4,INPUT_PULLUP);
  pinMode(pinButton5,INPUT_PULLUP);
  digitalWrite(pinLEDPress,pressedLEDState);
  digitalWrite(pinLEDConnected,connectedLEDState);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9); 
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN ,ESP_PWR_LVL_P9);
  bleKeyboard.begin();
}

void loop() {
  // Check the key press state
  int new_button1State = digitalRead(pinButton1);
  int new_button2State = digitalRead(pinButton2);
  int new_button3State = digitalRead(pinButton3);
  int new_button4State = digitalRead(pinButton4);
  int new_button5State = digitalRead(pinButton5);
  
  boolean ispressed1 = (new_button1State == LOW);
  boolean ispressed2 = (new_button2State == LOW);
  boolean ispressed3 = (new_button3State == LOW);
  boolean ispressed4 = (new_button4State == LOW);
  boolean ispressed5 = (new_button5State == LOW);

  // If we have pressed a button, light the LED, otherwise turn it off)
  triggerPressLED(ispressed1 || ispressed2 || 
    ispressed3 || ispressed4 || ispressed5);

  if (button1State != new_button1State) {
    // We have changed the button state, so need to do something
    button1State = new_button1State;
    if (new_button1State == LOW) {
      // We have pressed the first button (left arrow)
      Serial.print("\nButton1 pressed");
      sendKey(KEY_LEFT_ARROW,true);
    } else {
      Serial.print("\nButton1 released");
      sendKey(KEY_LEFT_ARROW,false);
    }
  }
  
  if (button2State != new_button2State) {
    // We have changed the button state, so need to do something
    button2State = new_button2State;
    if (new_button2State == LOW) {
      // We have pressed the second button (right arrow)
      Serial.print("\nButton2 pressed");
      sendKey(KEY_RIGHT_ARROW,true);
    } else {
      Serial.print("\nButton2 released");
      sendKey(KEY_RIGHT_ARROW,false);
    }
  }

  if (button3State != new_button3State) {
    // We have changed the button state, so need to do something
    button3State = new_button3State;
    if (new_button3State == LOW) {
      // We have pressed the third button (up arrow)
      Serial.print("\nButton3 pressed");
      sendKey(KEY_UP_ARROW,true);
    } else {
      Serial.print("\nButton3 released");
      sendKey(KEY_UP_ARROW,false);
    }
  }

  if (button4State != new_button4State) {
    // We have changed the button state, so need to do something
    button4State = new_button4State;
    if (new_button4State == LOW) {
      // We have pressed the fourth button (down arrow)
      Serial.print("\nButton4 pressed");
      sendKey(KEY_DOWN_ARROW,true);
    } else {
      Serial.print("\nButton4 released");
      sendKey(KEY_DOWN_ARROW,false);
    }
  }

  if (button5State != new_button5State) {
    // We have changed the button state, so need to do something
    button5State = new_button5State;
    if (new_button5State == LOW) {
      // We have pressed the fifth button (home)
      Serial.print("\nButton5 pressed");
      sendKey(KEY_HOME,true);
    } else {
      Serial.print("\nButton5 released");
      sendKey(KEY_HOME,false);
    }
  }

  connectedLEDPulse();

  // Allow for flickering on/off as connection is made
  delay(50);
}

 void connectedLEDPulse() {
  // check to see if it's time to blink the LED; that is, if the difference
  // between the current time and last time you blinked the LED is bigger than
  // the interval at which you want to blink the LED.
  unsigned long currentMillis = millis();

  int interval;
  if (bleKeyboard.isConnected()) {
    if (connectedLEDState == HIGH) {
      interval = ledOnInterval;
    } else {
      interval = connectedInterval;
    }
  } else {
    interval = notconnectedInterval;
  }
  
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (connectedLEDState == LOW) {
      connectedLEDState = HIGH;
    } else {
      connectedLEDState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(pinLEDConnected, connectedLEDState);
  }
}

void triggerPressLED(boolean ispressed) {
  pressedLEDState = ispressed;
  if (ispressed) {
    digitalWrite(pinLEDPress,HIGH);
  } else {
    digitalWrite(pinLEDPress,LOW);
  }
}

void sendKey(int keyCode, boolean keyDown) {
  // Only try to send the command if we are connected
  if (bleKeyboard.isConnected()) {
    if (keyDown) {
      bleKeyboard.press(keyCode);
    } else {
      bleKeyboard.release(keyCode);
    }
  }
}
