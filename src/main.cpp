#include <Arduino.h>
#include "log.h"
#include "util/events.hpp"
#include "util/BootProcess.hpp"
#include "util/Timer.hpp"
#include "util/NVS.hpp"
#include "pin_map.h"
#include "event_definitions.h"
#include "modules/LightsController.hpp"


EVENT_DEFINE(FOO_EVENT);


const struct : BootProcess
{
    using BootProcess::BootProcess;

    void runBootProcess() override
    {
        LOG_I("Hello World");
        FOO_EVENT << 3;
    }
} foo{"Foo initialized"};

const FuncBootProcess bar1{"Bar 1", [] { LOG_I("Hello from Bar 1"); }};
const FuncBootProcess bar2{"Bar 2", [] { LOG_I("Hello from Bar 2"); }};
const FuncBootProcess bar3{"Bar 3", [] { LOG_I("Hello from Bar 3"); }};


LightsController lightsController;


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

    NVS::begin("alarm_clock");

    events::init();
    events::GLOBAL >> [](const Event_t& e) { LOG_D("Event posted: %s #%d", e.base, e.id); };

    auto& x = FOO_EVENT >> [](const Event_t& e) { LOG_I("%s #%d", e.base, e.id); };
    FOO_EVENT >> 2 >> [](const Event_t& e)
    {
        LOG_W("%s #%d - %lu", e.base, e.id, e.data<uint32_t>());
    };
    BOOT_EVENT >> [](const Event_t& e)
    {
        if (e.id == BootProcess::EVENT_ALL_COMPLETED)
        {
            LOG_I("(boot) completed");
        }
        else
        {
            LOG_I("(boot %02d/%02d) %s", e.id + 1, BootProcess::count(), e.data<const char*>());
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
    static uint8_t lightValue = 50;

    delay(2000);
    FOO_EVENT << 1;
    delay(3000);
    FOO_EVENT << 2 << millis();
    LIGHTS_EVENT << SET_VALUE << lightValue;
    lightValue = (lightValue + 10) % 110;
}
