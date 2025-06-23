#include <Arduino.h>

#include "modules/lights_controller.h"
#include "modules/matrix_controller.h"
#include "modules/rtc_alarm_manager.h"
#include "modules/input_handler.h"

#include "util/compile_datetime.h"
#include "event_definitions.h"
#include "pin_map.h"
#include "log.h"
#include "matrix_font.h"


LightsController lights_controller{
    {.pin = pins::lights, .resolution = 13, .freq = 5000, .fade_time = 250, .gamma = 2.2f}
};
RtcAlarmManager rtc;
MatrixController matrix_controller{
    pins::matrix_cs,
    [](char* buf, size_t size)
    {
        auto now = time(nullptr);
        tm tm{};
        localtime_r(&now, &tm);
        if (!rtc.anyAlarmTriggered())
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
InputHandler input_handler{pins::button_left, pins::button_middle, pins::button_right};


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
    BOOT_EVENT >> [](const Event_t& e)
    {
        if (e.id == BootProcess::EVENT_ALL_COMPLETED)
            LOG_I("(boot) completed");
        else
            LOG_I("(boot %02d/%02d) %s", e.id + 1, BootProcess::count(), e.data<const char*>());
    };
    ALARM_EVENT >> TRIGGERED_ANY >> [](auto) { LIGHTS_EVENT << SET_MAX; };
    INPUT_EVENT >> CLICK_LEFT >> [](auto) { matrix_controller.scrollPrev(); };
    INPUT_EVENT >> CLICK_MIDDLE >> [](auto) { ALARM_EVENT << DEACTIVATE; };
    INPUT_EVENT >> CLICK_RIGHT >> [](auto) { matrix_controller.scrollNext(); };
    INPUT_EVENT >> LONG_PRESS_LEFT >> [](auto) { LIGHTS_EVENT << SET_OFF; };
    INPUT_EVENT >> LONG_PRESS_MIDDLE >> [](auto) { LIGHTS_EVENT << SET_VALUE << 50; };
    INPUT_EVENT >> LONG_PRESS_RIGHT >> [](auto) { LIGHTS_EVENT << SET_MAX; };


    BootProcess::runAll();
    delay(1000);


    rtc.alarm1.setIn8h();
    rtc.alarm2.hour = 8;
    rtc.alarm2.minute = 30;
    rtc.alarm2.enabled = true;
    rtc.alarm2.repeat = 1 << 4;
}

void loop()
{
    delay(1000);
}
