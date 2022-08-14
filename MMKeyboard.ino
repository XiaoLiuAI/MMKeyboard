#include "BleKeyboardLayer.h"
#include "usb_hid_keys.h"
#include <iostream>
#include "PhysicalKeyboardLayer.h"
#include "utils.h"

BleKeyboard bleKeyboard;
PhysicalKeyboard *physicalKeyboard;

void setup() {

    Serial.begin(115200);
    Serial.println("Starting BLE work!");
    bleKeyboard.begin();

    physicalKeyboard = new PhysicalKeyboard(2, 25, 26, 27, 1);

    for (int i=0; i<physicalKeyboard->get_num_rows(); i++){
        for (int j=0; j<physicalKeyboard->get_num_rows(); j++){
            physicalKeyboard->onMatrixPress(i, j, test_matrix_callback);
            physicalKeyboard->onMatrixRelease(i, j, test_matrix_callback);
            physicalKeyboard->onMatrixLongPress(i, j, test_matrix_long_callback);
        }
    }
}

void test_matrix_callback(int i, int j, int btn, int state){
    Serial.printf("btn (%d, %d) on pin %d is pressed, and has state %d with chip states ", i, j, btn, state);
    serialPrintBinary(physicalKeyboard->get_matrix_btn(i, j)->get_chip_sates());
}

bool test_matrix_long_callback(int i, int j, int btn, int state){
    Serial.printf("btn (%d, %d) on pin %d is long pressed, and has state %d with chip states ", i, j, btn, state);
    serialPrintBinary(physicalKeyboard->get_matrix_btn(i, j)->get_chip_sates());
    return false;
}

void loop() {
    if(bleKeyboard.isConnected()) {
        if(Serial.available() && Serial.readString() == "run"){
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

    physicalKeyboard->loop();

//   Serial.println("Waiting 1 seconds...");
//   delay(1000);
}