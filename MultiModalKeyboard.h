#ifndef MMKEYBOARD_H
#define MMKEYBOARD_H

#include "sdkconfig.h"
#include <iostream>
#include "PhysicalKeyboardLayer.h"
#include "BleKeyboardLayer.h"
#include "usb_hid_keys.h"
#include <functional>

// TODO 其它键盘配列

const uint8_t default_left_matrix_map[6][7] = {
    {KEY_ESC, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_NONE},
    {KEY_EQUAL, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_NONE},
    {KEY_TAB, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_NONE},
    {KEY_CAPS_LOCK, KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_NONE},
    {KEY_LEFTSHIFT, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_NONE},
    {KEY_NONE, KEY_GRAVE, KEY_BACKSLASH, KEY_LEFT, KEY_RIGHT, KEY_NONE, KEY_NONE}
};

const uint8_t default_right_matrix_map[6][7] = {
    {KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12},
    {KEY_NONE, KEY_6, KEY_7, KEY_F8, KEY_9, KEY_0, KEY_MINUS},
    {KEY_NONE, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P, KEY_BACKSLASH},
    {KEY_NONE, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON, KEY_APOSTROPHE},
    {KEY_NONE, KEY_N, KEY_M, KEY_COMMA, KEY_DOT, KEY_SLASH, KEY_RIGHTSHIFT},
    {KEY_NONE, KEY_NONE, KEY_UP, KEY_DOWN, KEY_LEFTBRACE, KEY_RIGHTBRACE, KEY_NONE}
};

const uint8_t default_left_thumbs_map[6] = {
    KEY_LEFTCTRL, KEY_LEFTALT, KEY_BACKSPACE, KEY_DELETE, KEY_HOME, KEY_END
};

const uint8_t default_right_thumbs_map[6] = {
    KEY_RIGHTMETA, KEY_HOME, KEY_RIGHTCTRL, KEY_PAGE_UP, KEY_PAGE_DOWN, KEY_RETURN
};


#define num_rows 6
#define num_cols 7
#define num_thum_keys 6


class MultiModalKeyboard: public PhysicalKeyboard  // TODO 模板元编程
{
    // TODO 键值remap
    // TODO 键盘宏
    // TODO 特殊的键转换Fn
    public:
        MultiModalKeyboard(
            bool right_hand,
            int chips, int data_pin, int clk_pin, int shift_load_pin, 
            BleKeyboard &bleKeyboard
        ):PhysicalKeyboard(chips, data_pin, clk_pin, shift_load_pin, num_rows, num_cols, num_thum_keys), _right_hand(right_hand), _bleKeyboard(bleKeyboard){
            for (int i=0; i<num_rows; i++){
                for(int j=0; j<_num_cols; j++){
                    if(_right_hand){
                        // TODO 掉电存储
                        matrix_keymap[i][j] = default_right_matrix_map[i][j];
                    }else{
                        matrix_keymap[i][j] = default_left_matrix_map[i][j];
                    }
                    // C++ knowledge bind/function for closure
                    onMatrixPress(i, j, std::bind(&MultiModalKeyboard::onMatrixButtonPress, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
                    // onLongPress(false, i*num_rows+j, this->onButtonLongPress)
                    // onRelease(false, i*num_rows+j, this->onButtonRelease)
                }
            }
            for (int i=0; i<num_thum_keys; i++){
                if(right_hand){
                    thumb_keymap[i] = default_left_thumbs_map[i];
                }else{
                    thumb_keymap[i] = default_right_thumbs_map[i];
                }
                onThumbPress(i+num_cols*num_rows, std::bind(&MultiModalKeyboard::onThumbButtonPress, this, std::placeholders::_1, std::placeholders::_2));
                // onPress(true, i, this->onButtonPress)
                // onLongPress(true, i, this->onButtonLongPress)
                // onRelease(true, i, this->onButtonRelease)
            }
        }
        ~MultiModalKeyboard(){}
        void loop(){
            btnController->check();
        }
        void onMatrixButtonPress(int i, int j, int btn, int status) {
            Serial.print("Button pressed on pin ");
            Serial.print(btn);
            Serial.print(" which represents ");
            Serial.printf("%d, %c\n", matrix_keymap[i][j], matrix_keymap[i][j]);
        }
        void onThumbButtonPress(int btn, int status) {
            btn = btn-num_cols*num_rows;
            Serial.print("Button pressed on pin ");
            Serial.print(btn);
            Serial.print(" which represents ");
            Serial.printf("%d, %c\n", thumb_keymap[btn], thumb_keymap[btn]);
        }
        // bool onButtonLongPress(int btn, int status) {
        //     Serial.print("Long button press on pin ");
        //     Serial.println(btn);
        //     return false;
        // }
        // void onButtonRelease(int btn, int status) {
        //     Serial.print("Button released on pin ");
        //     Serial.println(btn);
        // }

    private: 
        uint8_t matrix_keymap[num_rows][num_cols];
        uint8_t thumb_keymap[num_thum_keys];
        bool _right_hand;
        BleKeyboard _bleKeyboard;

        // status of all keys?
    // protected:
};

#endif