#include <Arduino.h>

#include "modules/input_handler.h"
#include "modules/matrix_controller.h"
#include "modules/rtc_alarm_manager.h"
#include "modules/lights_controller.h"
#include "modules/ui_display_manager.h"
#include "modules/sensor_manager.h"
#include "modules/audio_controller.h"
#include "event_definitions.h"
#include "log.h"
#include "matrix_font.h"
#include "u8g2_fonts.h"
#include "pin_map.h"

#ifdef ENV_DEBUG
#define DEBUG_ONLY(x) x
#else
#define DEBUG_ONLY(x)
#endif


// Boot #1
[[maybe_unused]] static struct : private BootProcess
{
    void runBootProcess() override { NVS::begin("alarm_clock"); }
    using BootProcess::BootProcess;
}
nvs_boot{"NVS initialized"};

// Boot #2
[[maybe_unused]] static RtcAlarmManager rtc{};


static void matrix_time(char* buf, size_t size);
static void matrix_date(char* buf, size_t size);
static void matrix_temp(char* buf, size_t size);
static void matrix_hum(char* buf, size_t size);

// Boot #3
[[maybe_unused]] static MatrixController matrix{pins::matrix_cs, matrix_time, matrix_date, matrix_temp, matrix_hum};

// Boot #4
[[maybe_unused]] static SensorManager sensors{};

// Boot #5
[[maybe_unused]] static InputHandler inputs{
    pins::button_left, pins::button_middle, pins::button_right
};

// Boot #6
[[maybe_unused]] static LightsController lights{
    {.pin = pins::lights, .resolution = 13, .freq = 5000, .fade_time = 250, .gamma = 2.2f}
};

// Boot #X
[[maybe_unused]] static AudioController audio{pins::i2s_data, pins::i2s_bck, pins::i2s_lrc};


static uint8_t nvv_tmp[16]{};
#define NVV_TMP(i) &nvv_tmp[i % 16]
#define NVV_EDIT(base, nvv) [](mui_t* ui, uint8_t msg) { \
    auto* vmm = (mui_u8g2_u8_min_max_t *)muif_get_data(ui->uif); auto* value = mui_u8g2_u8mm_get_valptr(vmm); \
    auto res = base(ui, msg); if (msg == MUIF_MSG_FORM_START) *value = *nvv;\
    else if ((msg == MUIF_MSG_CURSOR_SELECT || msg == MUIF_MSG_VALUE_INCREMENT || msg == MUIF_MSG_VALUE_DECREMENT) \
    && !ui->is_mud) nvv = *value; return res; }

// Boot #7
static UiDisplayManager ui{
    pins::ui_display_cs, pins::ui_display_dc, pins::ui_display_rst,
    MUI_FORM(1)
    MUI_STYLE(0)
    MUI_LABEL(5, 12, "Duration:")
    MUI_XY("IN", 56, 12)
    MUI_LABEL(5, 30, "A1 Hour:")
    MUI_XY("A1", 56, 30)
    MUI_XYT("LV", 64, 59, " OK "),
    {
        MUIF_U8G2_FONT_STYLE(0, fonts::helv<8>),
        MUIF_BUTTON("LV", mui_u8g2_btn_exit_wm_fi),
        MUIF_U8G2_U8_MIN_MAX_STEP_WIDTH("IN", NVV_TMP(0), 0, 180, 10, 40, MUI_MMS_SHOW_VALUE,
                                        NVV_EDIT(mui_u8g2_u8_fixed_width_bar_wm_mud_pi, lights.duration())),
        MUIF_U8G2_U8_MIN_MAX("A1", NVV_TMP(1), 0, 23,
                             NVV_EDIT(mui_u8g2_u8_min_max_wm_mud_pi, rtc.alarm1.hour)),
        MUIF_LABEL(mui_u8g2_draw_text)
    }
};


void setup()
{
    using namespace logging;
    DEBUG_ONLY(Logger.registerDevice<SerialLog>(Level::TRACE, DEFAULT_FORMAT ^ LEVEL_SHORT | LEVEL_LETTER));


    events::init();
    DEBUG_ONLY(events::GLOBAL >> [](const Event_t& e) { LOG_D("Event posted: %s #%d", e.base, e.id); });

    BOOT_EVENT >> [](const Event_t& e)
    {
        if (e.id == BootProcess::EVENT_ALL_COMPLETED)
            LOG_I("(boot) completed");
        else
            LOG_I("(boot %02d/%02d) %s", e.id + 1, BootProcess::count(), e.data<const char*>());
    };
    ALARM_EVENT >> TRIGGERED_ANY >> [](auto)
    {
        lights.max();
    };
    INPUT_EVENT >> CLICK_LEFT >> [](auto)
    {
        if (ui.active())
            ui.actionPrev();
        else
            matrix.scrollPrev();
    };
    INPUT_EVENT >> CLICK_MIDDLE >> [](auto)
    {
        if (ui.active())
            ui.actionSelect();
        else
            ui.enter();
    };
    INPUT_EVENT >> CLICK_RIGHT >> [](auto)
    {
        if (ui.active())
            ui.actionNext();
        else
            matrix.scrollNext();
    };
    INPUT_EVENT >> LONG_PRESS_MIDDLE >> [](auto)
    {
        if (ui.active())
            ui.exit();
        else if (lights.currentValue() > 0)
            lights.off();
        else
            lights.max();
    };
    INPUT_EVENT >> REPEATING_LEFT >> [](auto)
    {
        if (ui.active())
            ui.actionPrev();
        else
            lights.set((lights.currentValue() - 5) % 105);
    };
    INPUT_EVENT >> REPEATING_RIGHT >> [](auto)
    {
        if (ui.active())
            ui.actionNext();
        else
            lights.set((lights.currentValue() + 5) % 105);
    };
    SENSOR_EVENT >> LIGHT >> [](const Event_t& e)
    {
        auto lux = e.data<float>();
        if (lux < 0.001)
        {
            matrix.shutdown(true);
            return;
        }

        // Logarithmic normalization
        constexpr float log_min = log10(1.f);
        constexpr float log_max = log10(10000.f);
        auto log_lux = log10(lux);
        auto norm = min(max(0.f, (log_lux - log_min) / (log_max - log_min)), 1.f); // Normalize to 0.0-1.0

        // Scale to brightness range
        auto brightness = lround(norm * 15.0 + 0.5); // Round to the nearest int
        matrix.shutdown(false);
        matrix.setBrightness(brightness);
        LOG_T("matrix brightness set to %d (for %f)", brightness, lux);
    };


    BootProcess::runAll();


    rtc.alarm1.setIn8h();
    rtc.alarm2.hour = 8;
    rtc.alarm2.minute = 30;
    rtc.alarm2.enabled = true;
    rtc.alarm2.repeat = 1 << 4;


    DEBUG_ONLY(delay(1000));
}

void loop()
{
    delay(1000);
}


static void matrix_time(char* buf, size_t size)
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
}

static void matrix_date(char* buf, size_t size)
{
    auto now = time(nullptr);
    tm tm{};
    localtime_r(&now, &tm);
    strftime(buf, size, "%d. %b", &tm);
}

static void matrix_temp(char* buf, size_t size)
{
    snprintf(buf, size, "%02.1f \xB0 C", sensors.temperature());
}

static void matrix_hum(char* buf, size_t size)
{
    snprintf(buf, size, "%02.1f %%", sensors.humidity());
}
