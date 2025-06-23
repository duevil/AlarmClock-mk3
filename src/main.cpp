#include <Arduino.h>

#include "log.h"
#include "util/events.hpp"
#include "util/BootProcess.hpp"
#include "util/Timer.hpp"
#include "util/NVS.hpp"
#include "pin_map.h"
#include "event_definitions.h"
#include "util/compile_datetime.h"
#include "modules/MatrixController.hpp"
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


LightsController lights_controller;
MatrixController matrix_controller{
    pins::matrix_cs,
    [](char* buf, size_t size)
    {
        auto now = time(nullptr);
        tm tm{};
        localtime_r(&now, &tm);
        if (millis() > 30000) // when alarm is triggered
        {
            strftime(buf, size, "%R %S", &tm);
            // seconds are at index 6 and 7
            matrix_font_to_subscript(buf[6]);
            matrix_font_to_subscript(buf[7]);
        }
        else
        {
            const char* fmt;
            if (auto ms = millis() % 1000; ms < 250)
                fmt = "%R $   ";
            else if (ms < 500 || ms >= 750)
                fmt = "%R  #  ";
            else
                fmt = "%R   $ ";
            strftime(buf, size, fmt, &tm);
        }
    },
    [](char* buf, size_t size)
    {
        auto now = time(nullptr);
        tm tm{};
        localtime_r(&now, &tm);
        strftime(buf, size, "%d. %b", &tm);
    }
};


void test()
{
    delay(1000);
    LOG_T("millis: %lu", millis());
}

const FuncThread tTest{&test, {.name = "test", .priority = 10}};
Timer scroll_matrix_timer{};


void setup()
{
    set_internal_rtc_from_compile_datetime();

    using namespace logging;
#ifdef ENV_DEBUG
    Logger.registerDevice<SerialLog>(Level::TRACE, DEFAULT_FORMAT ^ LEVEL_SHORT | LEVEL_LETTER);
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

    scroll_matrix_timer.always(20, [] { matrix_controller.scrollNext(); }, true);


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
