#include "BleKeyboardLayer.h"
#include "usb_hid_keys.h"
#include <iostream>
#include "utils.h"


static int data_pin=25;
static int clk_pin=26;
static int shift_pin=27;

BleKeyboard bleKeyboard;

#define MM_KEYBOARD 1;

#ifdef MM_KEYBOARD
#include "ManuFormKeyboard.h"

ManuFormKeyboard *mm_keyboard = new ManuFormKeyboard(true, 2, data_pin, clk_pin, shift_pin, &bleKeyboard);
#endif

#ifdef PHY_KEYBOARD
#include "PhysicalKeyboardLayer.h"

PhysicalKeyboard *physicalKeyboard;
#endif

#ifdef SINGLE_BTN
#include "PBtnTogglePISOController.h"
#include "PBtnTogglePISO.h"

PBtnTogglePISOController *btnController = new PBtnTogglePISOController(2, data_pin, clk_pin, shift_pin);
PBtnTogglePISO *pbtn = new PBtnTogglePISO(0, 0, HIGH);
#endif


void setup() {

    Serial.begin(115200);
    Serial.println("Starting BLE work!");
    bleKeyboard.begin();

#ifdef PHY_KEYBOARD
    Serial.println("initial physical key board");
    physicalKeyboard = new PhysicalKeyboard(2, data_pin, clk_pin, shift_pin, 1, 1, 0);

    for (int i=0; i<physicalKeyboard->get_num_rows(); i++){
        for (int j=0; j<physicalKeyboard->get_num_rows(); j++){
            physicalKeyboard->onMatrixPress(i, j, test_matrix_callback);
            physicalKeyboard->onMatrixRelease(i, j, test_matrix_callback);
            physicalKeyboard->onMatrixLongPress(i, j, test_matrix_long_callback);
        }
    }
#endif

#ifdef SINGLE_BTN
    pbtn->onPress(test_btn_callback);
    pbtn->onLongPress(test_btn_long_callback);
    btnController->add(pbtn);
    Serial.println("btn is initialized as ");
    pbtn->reportStatus("setup");
#endif

#ifdef MM_KEYBOARD
    mm_keyboard->reportKeyMap();
#endif
}

#ifdef SINGLE_BTN
void test_btn_callback(int btn, int state){
    if (state){
        Serial.printf("btn on pin %d is pressed with chip states ", btn);
    }else{
        Serial.printf("btn on pin %d is released with chip states ", btn);
    }
    
    serialPrintBinary(pbtn->get_chip_sates());
}

bool test_btn_long_callback(int btn, int state){
    if (state){
        Serial.printf("btn on pin %d is long pressed with state %d with chip states ", btn, state);
    }else{
        Serial.printf("btn on pin %d is long pressed with state %d with chip states ", btn, state);
    }
    serialPrintBinary(pbtn->get_chip_sates());
    return false;
}
#endif

#ifdef PHY_KEYBOARD
void test_matrix_callback(int i, int j, int btn, int state){
    Serial.printf("btn (%d, %d) on pin %d is pressed, and has state %d with chip states ", i, j, btn, state);
    serialPrintBinary(physicalKeyboard->get_matrix_btn(i, j)->get_chip_sates());
}

bool test_matrix_long_callback(int i, int j, int btn, int state){
    Serial.printf("btn (%d, %d) on pin %d is long pressed, and has state %d with chip states ", i, j, btn, state);
    serialPrintBinary(physicalKeyboard->get_matrix_btn(i, j)->get_chip_sates());
    return false;
}
#endif

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

#ifdef PHY_KEYBOARD
    physicalKeyboard->loop();
#endif

#ifdef SINGLE_BTN
    btnController -> check();
#endif

//   Serial.println("Waiting 1 seconds...");
//   delay(1000);
}