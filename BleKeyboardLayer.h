#ifndef ESP32_BLE_KEYBOARD_H
#define ESP32_BLE_KEYBOARD_H
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#if defined(USE_NIMBLE)

#include "NimBLECharacteristic.h"  // Apache 的5.0，暂时不管
#include "NimBLEHIDDevice.h"

#define BLEDevice                  NimBLEDevice
#define BLEServerCallbacks         NimBLEServerCallbacks
#define BLECharacteristicCallbacks NimBLECharacteristicCallbacks
#define BLEHIDDevice               NimBLEHIDDevice
#define BLECharacteristic          NimBLECharacteristic
#define BLEAdvertising             NimBLEAdvertising
#define BLEServer                  NimBLEServer

#else

#include "BLEHIDDevice.h"
#include "BLECharacteristic.h"

#endif // USE_NIMBLE

#include "AbcKeyboardLayer.h"
#include <iostream>

class BleKeyboard: public AbcKeyboard, public BLEServerCallbacks, public BLECharacteristicCallbacks
{
    public:
        BleKeyboard(std::string deviceName = "ESP32 Keyboard", std::string deviceManufacturer = "Espressif", uint8_t batteryLevel = 100)
        : AbcKeyboard(deviceName, deviceManufacturer), batteryLevel(batteryLevel){}
        ~BleKeyboard(){}
        bool isConnected(void);
        void setBatteryLevel(uint8_t level);
        virtual void begin(void) override;
        virtual void end(void) override;
        virtual void sendReport(KeyReport* keys) override;
        virtual size_t press(uint8_t k) override;
        virtual size_t release(uint8_t k) override;
        virtual void releaseAll(void) override;
        virtual void setName(std::string deviceName) override;  
        virtual void setDelay(uint32_t ms) override;

        virtual void set_vendor_id(uint16_t vid);
        virtual void set_product_id(uint16_t pid);
        virtual void set_version(uint16_t version);
    private:
        BLEHIDDevice* hid;
        BLECharacteristic* inputKeyboard;
        BLECharacteristic* outputKeyboard;  // output keyboard是什么鬼？
        BLEAdvertising*    advertising;
        uint8_t            batteryLevel;
    protected:
        virtual void onStarted(BLEServer *pServer) { };
        virtual void onConnect(BLEServer* pServer) override;  // service callback
        virtual void onDisconnect(BLEServer* pServer) override;  // service callback
        virtual void onWrite(BLECharacteristic* me) override;  // char callback. 不知道为啥会有这种情况。 keyboard怎么会被反向写入
};
#endif // CONFIG_BT_ENABLED
#endif // ESP32_BLE_KEYBOARD_H
