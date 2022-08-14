#include "Arduino.h"


template <typename T> T serialPrintBinary(T x, bool usePrefix=true)
{
    if(usePrefix) Serial.print("0b");
    for(int i = 0; i < 8 * sizeof(x); i++)
    {
        Serial.print(bitRead(x, sizeof(x) * 8 - i - 1));
    }
    Serial.println();
    return x;
}