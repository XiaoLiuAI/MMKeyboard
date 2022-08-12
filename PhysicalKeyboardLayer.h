#ifndef PHYSICALKEYBOARD_H
#define PHYSICALKEYBOARD_H

#include "sdkconfig.h"
#include <iostream>
#include "PBtnToggle.h"
#include "PBtnTogglePISO.h"
#include "PBtnTogglePISOController.h"
#include <new>
#include "Arduino.h"
#include <functional>
#include <utility>


typedef std::function<void(int, int, int, int)> matrix_callback;
typedef std::function<bool(int, int, int, int)> matrix_long_callback;


//                  from /Users/xiaoliu/ExSpace/Arduino/esp32-study/SendKeyStrokes/SendKeyStrokes.ino:5:
// /Users/xiaoliu/ExSpace/Arduino/esp32-study/SendKeyStrokes/PhysicalKeyboardLayer.h: In member function 'void PhysicalKeyboard::onMatrixPress(int, int, matrix_callback)':
// PhysicalKeyboardLayer.h:66:37: error: 
// conversion from 'std::_Bind_helper<false, std::function<void(int, int, int, int)>&, int&, int&, const std::_Placeholder<3>&, const std::_Placeholder<4>&>::type' 
// {aka 'std::_Bind<std::function<void(int, int, int, int)>(int, int, std::_Placeholder<3>, std::_Placeholder<4>)>'} 
// to 
// non-scalar type 'ToggleFunc' {aka 'std::function<void(int, int)>'} requested
//              ToggleFunc f = std::bind(callback, i, j, std::placeholders::_3, std::placeholders::_4);

// aka 'std::_Bind<std::function<void(int, int, int, int)>*(int, int, std::_Placeholder<3>, std::_Placeholder<4>)>'
// aka 'std::function<void(int, int)>'

class PhysicalKeyboard
{
    public:
        PhysicalKeyboard(
            int chips, int data_pin, int clk_pin, int shift_load_pin, 
            uint8_t num_rows=6, uint8_t num_cols=7, uint8_t num_thumb_keys=6)
        :_num_rows(num_rows), _num_cols(num_cols), _num_thumb_keys(num_thumb_keys){
            btnController = new PBtnTogglePISOController(chips, data_pin, clk_pin, shift_load_pin);  // 这个库的实现没有使用CE引脚，不是很好

            // 因为我最大配列只到7列，所以这里稍微浪费了几个IO
            btn_matrix = new PBtnTogglePISO*[_num_rows];
            for (int i=0; i<_num_rows; i++){
                // C++ knowledge placement new
                btn_matrix[i] = reinterpret_cast<PBtnTogglePISO *>(new char[_num_cols * sizeof(PBtnTogglePISO)]);
                // btn_matrix[i] new PBtnTogglePISO[_num_cols];

                for (int j=0; j<_num_cols; j++){
                    // C++ knowledge placement new
                    new (btn_matrix[i]+j) PBtnTogglePISO(i, j, HIGH);

                    btnController -> add(btn_matrix[i]+j);
                }
            }

            // 为了简单，不要弄得狗啃一样，拇指键区使用单独一块PISO芯片
            thumb_btns = reinterpret_cast<PBtnTogglePISO *>(new char[_num_thumb_keys * sizeof(PBtnTogglePISO)]);
            for (int i=0; i<_num_thumb_keys; i++){
                new (thumb_btns+i) PBtnTogglePISO(_num_rows, i, HIGH);
                btnController -> add(thumb_btns+i);
            }
        }
        ~PhysicalKeyboard(){}
        void loop(){
            btnController->check();
        }

        void onMatrixPress(int i, int j, matrix_callback callback){
            ToggleFunc f = std::bind(callback, i, j, std::placeholders::_1, std::placeholders::_2);
            btn_matrix[i][j].onPress(f);
        }
        void onMatrixLongPress(int i, int j, matrix_long_callback callback){
            LongPressFunc f = std::bind(callback, i, j, std::placeholders::_1, std::placeholders::_2);
            btn_matrix[i][j].onLongPress(f);
        }
        void onMatrixRelease(int i, int j, matrix_callback callback){
            ToggleFunc f = std::bind(callback, i, j, std::placeholders::_1, std::placeholders::_2);
            btn_matrix[i][j].onRelease(f);
        }
        void onThumbPress(int ind, ToggleFunc callback){
            thumb_btns[ind].onPress(callback);
        }
        void onThumbLongPress(int ind, LongPressFunc callback){
            thumb_btns[ind].onLongPress(callback);
        }
        void onThumbRelease(int ind, ToggleFunc callback){
            thumb_btns[ind].onRelease(callback);
        }
        uint8_t get_num_rows(){return _num_rows;}
        uint8_t get_num_cols(){return _num_cols;}
        uint8_t get_num_thum_keys(){return _num_thumb_keys;}
        
    protected:
        uint8_t _num_rows;
        uint8_t _num_cols;
        uint8_t _num_thumb_keys;
        PBtnTogglePISO** btn_matrix;
        PBtnTogglePISO* thumb_btns;
        PBtnTogglePISOController* btnController;
};

#endif
