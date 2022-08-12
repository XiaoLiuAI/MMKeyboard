#include "BleKeyboardLayer.h"

#if defined(USE_NIMBLE)
#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEUtils.h>
#include <NimBLEHIDDevice.h>
#else
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BLE2902.h"
#include "BLEHIDDevice.h"
#endif // USE_NIMBLE
#include "HIDTypes.h"
#include <driver/adc.h>
#include "sdkconfig.h"


#if defined(CONFIG_ARDUHAL_ESP_LOG)
  #include "esp32-hal-log.h"
  #define LOG_TAG ""
#else
  #include "esp_log.h"
  static const char* LOG_TAG = "BLEDevice";
#endif


// Report IDs:
#define KEYBOARD_ID 0x01
#define MEDIA_KEYS_ID 0x02

static const uint8_t _hidReportDescriptor[] = {
  USAGE_PAGE(1),      0x01,          // USAGE_PAGE (Generic Desktop Ctrls)
  USAGE(1),           0x06,          // USAGE (Keyboard)
  COLLECTION(1),      0x01,          // COLLECTION (Application)
  // ------------------------------------------------- Keyboard
  REPORT_ID(1),       KEYBOARD_ID,   //   REPORT_ID (1)
  USAGE_PAGE(1),      0x07,          //   USAGE_PAGE (Kbrd/Keypad)
  USAGE_MINIMUM(1),   0xE0,          //   USAGE_MINIMUM (0xE0)
  USAGE_MAXIMUM(1),   0xE7,          //   USAGE_MAXIMUM (0xE7)
  LOGICAL_MINIMUM(1), 0x00,          //   LOGICAL_MINIMUM (0)
  LOGICAL_MAXIMUM(1), 0x01,          //   Logical Maximum (1)
  REPORT_SIZE(1),     0x01,          //   REPORT_SIZE (1)
  REPORT_COUNT(1),    0x08,          //   REPORT_COUNT (8)
  HIDINPUT(1),        0x02,          //   INPUT (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  REPORT_COUNT(1),    0x01,          //   REPORT_COUNT (1) ; 1 byte (Reserved)
  REPORT_SIZE(1),     0x08,          //   REPORT_SIZE (8)
  HIDINPUT(1),        0x01,          //   INPUT (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
  REPORT_COUNT(1),    0x05,          //   REPORT_COUNT (5) ; 5 bits (Num lock, Caps lock, Scroll lock, Compose, Kana)
  REPORT_SIZE(1),     0x01,          //   REPORT_SIZE (1)
  USAGE_PAGE(1),      0x08,          //   USAGE_PAGE (LEDs)
  USAGE_MINIMUM(1),   0x01,          //   USAGE_MINIMUM (0x01) ; Num Lock
  USAGE_MAXIMUM(1),   0x05,          //   USAGE_MAXIMUM (0x05) ; Kana
  HIDOUTPUT(1),       0x02,          //   OUTPUT (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
  REPORT_COUNT(1),    0x01,          //   REPORT_COUNT (1) ; 3 bits (Padding)
  REPORT_SIZE(1),     0x03,          //   REPORT_SIZE (3)
  HIDOUTPUT(1),       0x01,          //   OUTPUT (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
  REPORT_COUNT(1),    0x06,          //   REPORT_COUNT (6) ; 6 bytes (Keys)  终于明白了，这里是报文的结构定义，6是6键无冲. 卧槽，要全键无冲这里就有点慢了... 能不能可变长度的参数？这得玩透蓝牙协议了
  REPORT_SIZE(1),     0x08,          //   REPORT_SIZE(8)
  LOGICAL_MINIMUM(1), 0x00,          //   LOGICAL_MINIMUM(0)
  LOGICAL_MAXIMUM(1), 0x65,          //   LOGICAL_MAXIMUM(0x65) ; 101 keys
  USAGE_PAGE(1),      0x07,          //   USAGE_PAGE (Kbrd/Keypad)
  USAGE_MINIMUM(1),   0x00,          //   USAGE_MINIMUM (0)
  USAGE_MAXIMUM(1),   0x65,          //   USAGE_MAXIMUM (0x65)
  HIDINPUT(1),        0x00,          //   INPUT (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
  END_COLLECTION(0),                 // END_COLLECTION
};


// uint8_t USBPutChar(uint8_t c);

void BleKeyboard::begin(void)
{
  BLEDevice::init(_deviceName);
  BLEServer* pServer = BLEDevice::createServer();
  pServer->setCallbacks(this);

//   AbcKeyboard = new AbcKeyboard();

  hid = new BLEHIDDevice(pServer);
  inputKeyboard = hid->inputReport(KEYBOARD_ID);  // <-- input REPORTID from report map
  outputKeyboard = hid->outputReport(KEYBOARD_ID);
//   inputMediaKeys = hid->inputReport(MEDIA_KEYS_ID);

  outputKeyboard->setCallbacks(this);

  hid->manufacturer()->setValue(_deviceManufacturer);

  hid->pnp(0x02, vid, pid, version);
  hid->hidInfo(0x00, 0x01);


#if defined(USE_NIMBLE)

  BLEDevice::setSecurityAuth(true, true, true);

#else

  BLESecurity* pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);

#endif // USE_NIMBLE

  hid->reportMap((uint8_t*)_hidReportDescriptor, sizeof(_hidReportDescriptor));
  hid->startServices();

  onStarted(pServer);

  advertising = pServer->getAdvertising();
  advertising->setAppearance(HID_KEYBOARD);
  advertising->addServiceUUID(hid->hidService()->getUUID());
  advertising->setScanResponse(false);
  advertising->start();
  hid->setBatteryLevel(batteryLevel);

  ESP_LOGD(LOG_TAG, "Advertising started!");
}

void BleKeyboard::end(void)
{
}

bool BleKeyboard::isConnected(void) {
  return this->connected;
}

void BleKeyboard::setBatteryLevel(uint8_t level) {
  this->batteryLevel = level;
  if (hid != 0)
    this->hid->setBatteryLevel(this->batteryLevel);
}

//must be called before begin in order to set the name
void BleKeyboard::setName(std::string deviceName) {
  this->_deviceName = deviceName;
}

/**
 * @brief Sets the waiting time (in milliseconds) between multiple keystrokes in NimBLE mode.
 * 
 * @param ms Time in milliseconds
 */
void BleKeyboard::setDelay(uint32_t ms) {
  this->_delay_ms = ms;
}

void BleKeyboard::set_vendor_id(uint16_t vid) { 
    this->vid = vid; 
}

void BleKeyboard::set_product_id(uint16_t pid) { 
    this->pid = pid; 
}

void BleKeyboard::set_version(uint16_t version) { 
    this->version = version; 
}

void BleKeyboard::sendReport(KeyReport* keys)
{
  if (this->isConnected())
  {
    this->inputKeyboard->setValue((uint8_t*)keys, sizeof(KeyReport));
    this->inputKeyboard->notify();
#if defined(USE_NIMBLE)        
    // vTaskDelay(delayTicks);
    this->delay_ms(_delay_ms);
#endif // USE_NIMBLE
  }    
}


size_t BleKeyboard::press(uint8_t k)
{
    // TODO k 被按下，同时应该检查整个physical的状态才对. 这样的话，其实两个physical可以共享一个ble => 2合1
    // 先把这坑爹的char to key给改掉再说吧
    if (0xe0<=k && k<0xe8){
        _keyReport.modifiers |= (1<<(k-224));
        k = 0;
    }

    // Add k to the key report only if it's not already present
    // and if there is an empty slot.
    if (_keyReport.keys[0] != k && _keyReport.keys[1] != k &&
        _keyReport.keys[2] != k && _keyReport.keys[3] != k &&
        _keyReport.keys[4] != k && _keyReport.keys[5] != k) {
        int i=0;
        for (; i<6; i++) {
            if (_keyReport.keys[i] == 0x00) {
                _keyReport.keys[i] = k;
                break;
            }
        }
        if (i == 6) {
            setWriteError();
            return 0;
        }
    }
    sendReport(&_keyReport);
    return 1;
}

size_t BleKeyboard::release(uint8_t k)
{
    if (0xe0<=k && k<0xe8){
        _keyReport.modifiers &= ~(1<<(k-224));
        k = 0;
        return 1;
    }

    // Test the key report to see if k is present.  Clear it if it exists.
    // Check all positions in case the key is present more than once (which it shouldn't be)
    // 芯片稍微快点，ram稍微大点，其实完全可以全键直接映射，最后report的时候一次性过滤，用不着在这里遍历
    for (int i=0; i<6; i++) {
        if (0 != k && _keyReport.keys[i] == k) {
            _keyReport.keys[i] = 0x00;
        }
    }

    sendReport(&_keyReport);
    return 1;
}

void BleKeyboard::releaseAll(void)  // 很好奇这个在什么时候会用到
{
    _keyReport.keys[0] = 0;
    _keyReport.keys[1] = 0;
    _keyReport.keys[2] = 0;
    _keyReport.keys[3] = 0;
    _keyReport.keys[4] = 0;
    _keyReport.keys[5] = 0;
    _keyReport.modifiers = 0;
    sendReport(&_keyReport);
}

size_t BleKeyboard::write(uint8_t c)
{
    uint8_t p = press(c);  // Keydown
    release(c);            // Keyup
    return p;              // just return the result of press() since release() almost always returns 1
}

size_t BleKeyboard::write(const uint8_t *buffer, size_t size) {
    size_t n = 0;
    while (size--) {
            if (*buffer != '\r') {
                    if (write(*buffer)) {
                        n++;
                    } else {
                        break;
                    }
            }
            buffer++;
    }
    return n;
}

void BleKeyboard::onConnect(BLEServer* pServer) {
  this->connected = true;

#if !defined(USE_NIMBLE)

  BLE2902* desc = (BLE2902*)this->inputKeyboard->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
  desc->setNotifications(true);

#endif // !USE_NIMBLE

}

void BleKeyboard::onDisconnect(BLEServer* pServer) {
  this->connected = false;

#if !defined(USE_NIMBLE)

  BLE2902* desc = (BLE2902*)this->inputKeyboard->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
  desc->setNotifications(false);

  advertising->start();

#endif // !USE_NIMBLE
}

void BleKeyboard::onWrite(BLECharacteristic* me) {
  uint8_t* value = (uint8_t*)(me->getValue().c_str());
  (void)value;
  ESP_LOGI(LOG_TAG, "special keys: %d", *value);
}