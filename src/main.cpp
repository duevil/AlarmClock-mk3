#ifdef ARDUINO
#include <Arduino.h>
#else
#include "mockup/mockup.h"
#endif


void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);
    printf("Hello World\n");
}

void loop()
{
    printf("%u\n", millis());
    delay(1000);
}