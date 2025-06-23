#include "rtc_alarm_manager.h"

#include "event_definitions.h"
#include "pin_map.h"


RtcAlarmManager::RtcAlarmManager(): BootProcess("Rtc initialized") {}

void RtcAlarmManager::setInternalFromExternal()
{
    m_rtc.refresh();
    if (m_rtc.year() == 0)
    {
        LOG_W("External RTC probably not set, skipping update");
        return;
    }
    tm tm{
        .tm_sec = m_rtc.second(),
        .tm_min = m_rtc.minute(),
        .tm_hour = m_rtc.hour(),
        .tm_mday = m_rtc.day(),
        .tm_mon = m_rtc.month() - 1,
        .tm_year = m_rtc.year() + 100,
    };

    char buf[32];
    strftime(buf, sizeof(buf), "%FT%TZ", &tm);
    LOG_D("External rtc dt: %s", buf);

    timeval tv{.tv_sec = mktime(&tm)};
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

    m_rtc.refresh();
    m_rtc.set(tm.tm_sec, tm.tm_min, tm.tm_hour, tm.tm_wday + 1, tm.tm_mday, tm.tm_mon + 1, tm.tm_year - 100);
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

void RtcAlarmManager::runBootProcess()
{
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


    pinMode(pins::alarm_interrupt, INPUT);
    attachInterrupt(pins::alarm_interrupt, alarmISR, FALLING);


    ALARM_EVENT >> TRIGGERED_ANY >> [this](auto)
    {
        m_rtc.refresh();

        if (m_rtc.alarmTriggered(URTCLIB_ALARM_1))
            ALARM_EVENT << TRIGGERED_1;

        if (m_rtc.alarmTriggered(URTCLIB_ALARM_2))
            ALARM_EVENT << TRIGGERED_2;

        m_deactivate_timer.reset();
    };
    ALARM_EVENT >> SNOOZED >> [this](auto)
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
    };
    ALARM_EVENT >> DEACTIVATED >> [this](auto)
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
    };


    m_deactivate_timer.once(1800, [] { ALARM_EVENT << DEACTIVATED; });
    m_update_timer.always(3600, [this] { setInternalFromExternal(); });


    m_alarm_1.set();
    m_alarm_2.set();
}

void RtcAlarmManager::alarmISR()
{
    ALARM_EVENT << TRIGGERED_ANY << FROM_ISR;
}

void RtcAlarmManager::Alarm::setIn8h()
{
    tm tm{};
    auto t = time(nullptr);
    t += 8 * 60 * 60;
    gmtime_r(&t, &tm);

    m_no_compute = true;

    hour = tm.tm_hour;
    minute = tm.tm_min;

    if (*repeat)
        repeat = *repeat | 1 << tm.tm_wday;

    enabled = true;

    m_no_compute = false;
    computeNextAndSet();
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
        enabled = true;
        computeNextAndSet();
    });
    minute.observe([this](auto)
    {
        enabled = true;
        computeNextAndSet();
    });
    repeat.observe([this](auto)
    {
        enabled = true;
        computeNextAndSet();
    });
    enabled.observe([this](auto)
    {
        if (!m_no_compute)
            set();
    });
}

void RtcAlarmManager::Alarm::setAt(const time_t& t) const
{
    tm tm{};
    gmtime_r(&t, &tm);

    if (m_id == 1)
        m_mgr.m_rtc.alarmSet(URTCLIB_ALARM_TYPE_1_FIXED_DHMS, tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_mday);
    else if (m_id == 2)
        m_mgr.m_rtc.alarmSet(URTCLIB_ALARM_TYPE_2_FIXED_DHM, tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_mday);
    char buf[32];
    strftime(buf, sizeof(buf), "%FT%TZ", &tm);
    LOG_I("Alarm #%u set at %s", m_id, buf);
}

void RtcAlarmManager::Alarm::set()
{
    if (*enabled)
        setAt(next);
    else if (m_id == 1)
        m_mgr.m_rtc.alarmDisable(URTCLIB_ALARM_1);
    else if (m_id == 2)
        m_mgr.m_rtc.alarmDisable(URTCLIB_ALARM_2);
}

void RtcAlarmManager::Alarm::computeNextAndSet()
{
    if (m_no_compute || !*enabled) return;

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
    m_next = mktime(&tm) + days_increment * 86400;

    set();
}
