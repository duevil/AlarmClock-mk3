#include "rtc_alarm_manager.h"

#include <util/compile_datetime.h>

#include "event_definitions.h"
#include "pin_map.h"


inline void set_timezone(const char* tz)
{
    setenv("TZ", tz, 1);
    tzset();
}


RtcAlarmManager::RtcAlarmManager(): BootProcess("Rtc initialized") {}

bool RtcAlarmManager::anyAlarmTriggered()
{
    return m_rtc.alarmTriggered(URTCLIB_ALARM_ANY);
}

void RtcAlarmManager::setInternalFromExternal()
{
    if (!m_rtc.refresh() || m_rtc.year() == 0)
    {
        LOG_W("External rtc probably not set, skipping update");
        return;
    }
    tm tm{
        .tm_sec = m_rtc.second(),
        .tm_min = m_rtc.minute(),
        .tm_hour = m_rtc.hour(),
        .tm_mday = m_rtc.day(),
        .tm_mon = m_rtc.month() - 1,
        .tm_year = m_rtc.year() + 100,
        .tm_isdst = 0,
    };

    char buf[32];
    strftime(buf, sizeof(buf), "%FT%TZ", &tm);
    LOG_D("External rtc dt: %s", buf);

    // as mktime interprets the time as being local but the external RTC time is UTC,
    // we temporarily set the system timezone to UTC
    set_timezone("UTC");
    timeval tv{.tv_sec = mktime(&tm)};
    set_timezone(m_timezone->c_str());
    settimeofday(&tv, nullptr);

    auto now = time(nullptr);
    gmtime_r(&now, &tm);
    strftime(buf, sizeof(buf), "%FT%TZ", &tm);
    LOG_I("Internal rtc updated to: %s", buf);
}

void RtcAlarmManager::setExternalFromInternal()
{
    auto now = time(nullptr);
    if (now < 1000)
    {
        LOG_W("Internal rtc probably not set, skipping update");
        return;
    }
    tm tm{};
    gmtime_r(&now, &tm);

    char buf[32];
    strftime(buf, sizeof(buf), "%FT%TZ", &tm);
    LOG_D("Internal rtc dt: %s", buf);

    m_rtc.set(tm.tm_sec, tm.tm_min, tm.tm_hour, tm.tm_wday + 1, tm.tm_mday, tm.tm_mon + 1, tm.tm_year - 100);
    m_rtc.refresh();
    tm = {
        .tm_sec = m_rtc.second(),
        .tm_min = m_rtc.minute(),
        .tm_hour = m_rtc.hour(),
        .tm_mday = m_rtc.day(),
        .tm_mon = m_rtc.month() - 1,
        .tm_year = m_rtc.year() + 100,
    };
    strftime(buf, sizeof(buf), "%FT%TZ", &tm);
    LOG_I("External rtc updated to: %s", buf);
}

NVV<String>& RtcAlarmManager::timezone()
{
    return m_timezone;
}

float RtcAlarmManager::temperature()
{
    auto temp = m_rtc.temp();
    return temp == URTCLIB_TEMP_ERROR
               ? NAN
               : static_cast<float>(temp) / 100.f;
}

void RtcAlarmManager::runBootProcess()
{
    // initialize timezone

    auto&& set_timezone = [](const String& tz)
    {
        LOG_I("System timezone set to: %s", tz.c_str());
        ::set_timezone(tz.c_str());
    };
    m_timezone.observe(set_timezone);
    if (!m_timezone->isEmpty())
        set_timezone(*m_timezone);


    set_internal_rtc_from_compile_datetime();


    // initialize external RTC

    Wire.begin();
#ifdef WOKWI
    m_rtc.set_model(URTCLIB_MODEL_DS1307);
#else
    m_rtc.set_model(URTCLIB_MODEL_DS3231);
#endif
    m_rtc.set_12hour_mode(false);
    m_rtc.refresh();
    if (m_rtc.lostPower())
    {
        LOG_W("External rtc has lost power");
        m_rtc.lostPowerClear();
        m_rtc.enableBattery(); // ensure the battery is enabled
    }
    m_rtc.disable32KOut();
    m_rtc.alarmClearFlag(URTCLIB_ALARM_1);
    m_rtc.alarmClearFlag(URTCLIB_ALARM_2);

    setInternalFromExternal();


    // initialize interrupt

    pinMode(pins::alarm_interrupt, INPUT);
    attachInterrupt(pins::alarm_interrupt, alarmISR, FALLING);


    // register event listeners

    ALARM_EVENT >> TRIGGERED_ANY >> [this](auto)
    {
        // we only refresh RTC data on trigger besides the regular time updates
        m_rtc.refresh();

        if (m_rtc.alarmTriggered(URTCLIB_ALARM_1))
            ALARM_EVENT << TRIGGERED_1;

        if (m_rtc.alarmTriggered(URTCLIB_ALARM_2))
            ALARM_EVENT << TRIGGERED_2;

        m_deactivate_timer.reset();
    };
    ALARM_EVENT >> SNOOZE >> [this](auto)
    {
        auto now = time(nullptr);
        now += 30 * 60;

        if (m_rtc.alarmTriggered(URTCLIB_ALARM_1))
        {
            m_rtc.alarmClearFlag(URTCLIB_ALARM_1);
            m_alarm_1.setAt(now);
        }

        if (m_rtc.alarmTriggered(URTCLIB_ALARM_2))
        {
            m_rtc.alarmClearFlag(URTCLIB_ALARM_2);
            m_alarm_2.setAt(now);
        }

        m_deactivate_timer.stop();
    };
    ALARM_EVENT >> DEACTIVATE >> [this](auto)
    {
        if (m_rtc.alarmTriggered(URTCLIB_ALARM_1))
        {
            m_rtc.alarmClearFlag(URTCLIB_ALARM_1);

            if (*m_alarm_1.repeat)
                m_alarm_1.computeNextAndSet();
            else
                m_alarm_1.enabled = false;
        }

        if (m_rtc.alarmTriggered(URTCLIB_ALARM_2))
        {
            m_rtc.alarmClearFlag(URTCLIB_ALARM_2);

            if (*m_alarm_2.repeat)
                m_alarm_2.computeNextAndSet();
            else
                m_alarm_2.enabled = false;
        }

        m_deactivate_timer.stop();
    };


    // stop alarm after 3 minutes
    m_deactivate_timer.once(1800, [] { ALARM_EVENT << DEACTIVATE; });
    // update internal RTC from external every hour
    m_update_timer.always(3600, [this] { setInternalFromExternal(); });


    m_alarm_1.set();
    m_alarm_2.set();

    setInternalFromExternal();
    setExternalFromInternal();
    setInternalFromExternal();
}

void RtcAlarmManager::alarmISR()
{
    ALARM_EVENT << TRIGGERED_ANY << FROM_ISR;
}

void RtcAlarmManager::Alarm::setIn8h()
{
    // get the current timestamp, add 8 hours and convert it to a tm value
    auto t = time(nullptr);
    t += 8 * 60 * 60;
    tm tm{};
    gmtime_r(&t, &tm);

    // we edit the NVVs, which would trigger recomputation with every assignment,
    // so we temporarily halt recomputing the next ring time
    m_no_compute = true;

    hour = tm.tm_hour;
    minute = tm.tm_min;

    // if the alarm is set to repeat,
    // ensure that alarm time's week day is set
    if (*repeat)
        repeat = *repeat | 1 << tm.tm_wday;

    enabled = true;

    m_no_compute = false;
    computeNextAndSet();
}

time_t RtcAlarmManager::Alarm::next() const
{
    return m_next;
}

RtcAlarmManager::Alarm::Alarm(const char* key_hour, const char* key_minute, const char* key_repeat,
                              const char* key_sound, const char* key_enabled, RtcAlarmManager& mgr,
                              uint8_t id): hour(key_hour),
                                           minute(key_minute),
                                           repeat(key_repeat),
                                           sound(key_sound),
                                           enabled(key_enabled),
                                           m_mgr(mgr),
                                           m_id(id)
{
    hour.observe([this](auto)
    {
        if (!m_no_compute)
        {
            enabled = true;
            computeNextAndSet();
        }
    });
    minute.observe([this](auto)
    {
        if (!m_no_compute)
        {
            enabled = true;
            computeNextAndSet();
        }
    });
    repeat.observe([this](auto)
    {
        if (!m_no_compute)
        {
            enabled = true;
            computeNextAndSet();
        }
    });
    enabled.observe([this](auto)
    {
        if (!m_no_compute)
            set();
    });
}

// set the RTC to trigger its alarm at the given time
void RtcAlarmManager::Alarm::setAt(const time_t& t) const
{
    // convert the timestamp to a tm value
    tm tm{};
    gmtime_r(&t, &tm);

    // the 8th bit is the alarm id, the macro sets only the 6th bit for the alarm mode
    m_mgr.m_rtc.alarmSet(m_id | URTCLIB_ALARM_TYPE_1_FIXED_DHMS, tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_mday);
    char buf[32];
    strftime(buf, sizeof(buf), "%FT%TZ", &tm);
    LOG_I("Alarm #%u set at %s", (m_id >> 7) + 1, buf);
}

// either set the alarm at it's next ring time
// or disable the RTC's alarm if the alarm is disabled
void RtcAlarmManager::Alarm::set()
{
    if (*enabled)
        setAt(m_next);
    else
        m_mgr.m_rtc.alarmDisable(m_id);
}

// if computation is allowed and the alarm is enabled,
// compute the next ring time and set the alarm
void RtcAlarmManager::Alarm::computeNextAndSet()
{
    if (m_no_compute || !*enabled) return;

    // get the current time and convert it to a tm value
    auto now = time(nullptr);
    tm tm{};
    gmtime_r(&now, &tm);

    int days_increment = 0;

    // if the current time is already in the past, add one day
    if (*hour * 3600 + *minute * 60 <= tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec)
        ++days_increment;

    // if the alarm is repeating, find the next weekday the alarm will repeat at
    while (*repeat && !(*repeat & 1 << ((tm.tm_wday + days_increment) % 7)))
        ++days_increment;

    tm.tm_hour = *hour;
    tm.tm_min = *minute;
    tm.tm_sec = 0;
    m_next = mktime(&tm) + days_increment * 86400LL;

    set();
}
