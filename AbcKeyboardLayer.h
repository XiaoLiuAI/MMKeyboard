#ifndef ABC_KEYBOARD
#define ABC_KEYBOARD

#include "sdkconfig.h"
#include "Print.h"
#include <iostream>

//  Low level key report: up to 6 keys and shift, ctrl etc at once
typedef struct
{
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t keys[6];
} KeyReport;

class AbcKeyboard : public Print
{
protected:
    KeyReport          _keyReport;
    std::string        _deviceName;
    std::string        _deviceManufacturer;
    uint32_t           _delay_ms = 7;
    bool               connected;
    virtual void delay_ms(uint64_t ms);

    uint16_t vid       = 0x05ac;
    uint16_t pid       = 0x820a;
    uint16_t version   = 0x0210;

public:
    AbcKeyboard(std::string deviceName = "ESP32 Keyboard", std::string deviceManufacturer = "Espressif")
    :_deviceName(deviceName), _deviceManufacturer(deviceManufacturer){}

    virtual void begin(void)=0;
    virtual void end(void)=0;
    virtual void sendReport(KeyReport* keys)=0;
    virtual size_t press(uint8_t k) = 0;
    virtual size_t release(uint8_t k)=0;
    virtual size_t write(uint8_t c)=0;
    virtual size_t write(const uint8_t *buffer, size_t size)=0;
    virtual void releaseAll(void)=0;
    virtual void setName(std::string deviceName)=0;  
    virtual void setDelay(uint32_t ms)=0;
};

#endif // ABC_KEYBOARD