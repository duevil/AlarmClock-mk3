#include <Arduino.h>
#include "log.h"
#include "util/events.hpp"
#include "util/BootProcess.hpp"
#include "util/Timer.hpp"
#include "util/NVS.hpp"


EVENT_DEFINE(FOO_EVENT);


const struct : BootProcess
{
    using BootProcess::BootProcess;

    void runProcess() override
    {
        LOG_I("Hello World");
        FOO_EVENT << 3;
    }
} foo{"Foo initialized"};

const FuncBootProcess bar1{"Bar 1", [] { LOG_I("Hello from Bar 1"); }};
const FuncBootProcess bar2{"Bar 2", [] { LOG_I("Hello from Bar 2"); }};
const FuncBootProcess bar3{"Bar 3", [] { LOG_I("Hello from Bar 3"); }};


void test()
{
    delay(1000);
    LOG_T("millis: %lu", millis());
}

const ThreadFunc<typeof(&test), 2048> tTest{test, {.name = "test", .priority = 10}};


void setup()
{
    using namespace logging;
#ifdef ENV_DEBUG
    Logger.registerDevice<SerialLog>(Level::DEBUG, DEFAULT_FORMAT ^ LEVEL_SHORT | LEVEL_LETTER);
#endif


    events::init();
    events::GLOBAL >> [](auto& event) { LOG_D("Event posted: %s #%d", event.base, event.id); };

    auto& x = FOO_EVENT >> [](auto& event) { LOG_I("%s #%d", event.base, event.id); };
    FOO_EVENT >> 2 >> [](auto& event)
    {
        LOG_W("%s #%d - %llu", event.base, event.id, *static_cast<uint32_t*>(event.data));
    };
    BOOT_EVENT >> [](auto& event)
    {
        if (event.id == BootProcess::EVENT_ALL_COMPLETED)
        {
            LOG_I("(boot) completed");
        }
        else
        {
            LOG_I("(boot %02d/%02d) %s", event.id + 1, BootProcess::count(), BootProcess::description(event.id));
        }
    };

    Timer::detached(20, [&x]
    {
        LOG_D("Event handler unregistered");
        FOO_EVENT.unregister(x);
    });


    BootProcess::runAll();


    delay(1000);
}

void loop()
{
    delay(2000);
    FOO_EVENT << 1;
    delay(3000);
    FOO_EVENT << 2 << millis();
}
