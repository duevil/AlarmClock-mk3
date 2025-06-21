#include <Arduino.h>
#include "log.h"
#include "util/events.hpp"
#include <Ticker.h>
#include "util/Timer.hpp"


void test()
{
    delay(1000);
    LOG_T("millis: %lu", millis());
}

const ThreadFunc<typeof(&test), 2048> tTest{test, {.name = "test", .priority = 10}};


EVENT_DEFINE(FOO_EVENT);


void setup()
{
    using namespace logging;
#ifdef ENV_DEBUG
    Logger.registerDevice<SerialLog>(Level::DEBUG, DEFAULT_FORMAT ^ LEVEL_SHORT | LEVEL_LETTER);
#endif
    LOG_I("Hello World");


    events::init();
    events::GLOBAL >> [](auto& event) { LOG_D("Event posted: %s #%d", event.base, event.id); };

    auto &x = FOO_EVENT >> [](auto& event) { LOG_I("%s #%d", event.base, event.id); };
    FOO_EVENT >> 2 >> [](auto& event)
    {
        LOG_W("%s #%d - %llu", event.base, event.id, *static_cast<uint32_t*>(event.data));
    };

    Timer::detached(20, [&x]
    {
        LOG_D("Event handler unregistered");
        FOO_EVENT.unregister(x);
    });
}

void loop()
{
    delay(2000);
    FOO_EVENT << 1;
    delay(3000);
    FOO_EVENT << 2 << millis();
}
