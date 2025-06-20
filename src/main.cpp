#include <Arduino.h>
#include "log.h"
#include "events/events.hpp"


void setup()
{
    using namespace logging;
    Logger.initialize();
    Logger.registerDevice<SerialLog>(DEFAULT_LEVEL, LEVEL_LETTER | TIMESTAMP_FULL | FILE_TRACE | FUNCTION_TRACE);
    LOG_I("Hello World");
}

void loop()
{
    delay(1000);
    LOG_I("millis: %lu", millis());
}
