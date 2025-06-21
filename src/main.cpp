#include <Arduino.h>
#include "log.h"
#include "util/events.hpp"

void test()
{
    delay(1000);
    LOG_I("millis: %lu", millis());
}

const ThreadFunc<typeof(&test), 2048> tTest{test, {.name = "test", .priority = 10}};
constexpr auto s = sizeof(tTest);

EVENT_DEFINE(FOO_EVENT);

using namespace logging;

void setup()
{
    Logger.registerDevice<SerialLog>(Level::DEBUG);
    LOG_I("Hello World");


    events::init();
    events::GLOBAL >> [](auto& event) { LOG_D("Event posted: %s #%d", event.base, event.id); };

    FOO_EVENT >> [](auto& event) { LOG_I("%s #%d", event.base, event.id); };
    FOO_EVENT >> 2 >> [](auto& event)
    {
        LOG_W("%s #%d - %llu", event.base, event.id, *static_cast<uint32_t*>(event.data));
    };
}

void loop()
{
    delay(2000);
    FOO_EVENT << 1;
    delay(3000);
    FOO_EVENT << 2 << millis();
}
