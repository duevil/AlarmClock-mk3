#include <Arduino.h>
#include "log.h"

void test()
{
    delay(1000);
    LOG_I("millis: %lu", millis());
}

const ThreadFunc<typeof(&test), 2048> tTest{test, {.name = "test", .priority = 10}};
constexpr auto s = sizeof(tTest);


using namespace logging;

void setup()
{
    Logger.registerDevice<SerialLog>(Level::DEBUG);
    LOG_I("Hello World");
}

void loop()
{
    delay(10);
}
