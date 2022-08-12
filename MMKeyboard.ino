/**
 * This example turns the ESP32 into a Bluetooth LE keyboard that writes the words, presses Enter, presses a media key and then Ctrl+Alt+Delete
 */
#include "BleKeyboardLayer.h"
#include "usb_hid_keys.h"
#include <iostream>

BleKeyboard bleKeyboard;

void setup() {

  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  bleKeyboard.begin();
}

void loop() {
  if(bleKeyboard.isConnected()) {
    if(Serial.available()){
        if (Serial.readString() == "run"){
            Serial.println("Sending 'Hello world'...");
            bleKeyboard.press(KEY_LEFTSHIFT);
            bleKeyboard.press(KEY_H);
            bleKeyboard.releaseAll();
            std::string msg = "Hello world";
            for (int i=1; i<msg.size(); i++){
                uint8_t key = KEY_A + (msg[i] - 'a');
                bleKeyboard.press(key);
                bleKeyboard.release(key);
            }

            delay(1000);

            Serial.println("Sending Enter key...");
            bleKeyboard.press(KEY_ENTER);
            bleKeyboard.release(KEY_ENTER);

            delay(1000);
        }
    }

    // Serial.println("Sending Play/Pause media key...");
    // bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);

    // delay(1000);

   //
   // Below is an example of pressing multiple keyboard modifiers 
   // which by default is commented out.
    /*
    Serial.println("Sending Ctrl+Alt+Delete...");
    bleKeyboard.press(KEY_LEFT_CTRL);
    bleKeyboard.press(KEY_LEFT_ALT);
    bleKeyboard.press(KEY_DELETE);
    delay(100);
    bleKeyboard.releaseAll();
    */
  }

  Serial.println("Waiting 5 seconds...");
  delay(5000);
}