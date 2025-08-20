#ifndef RTC_ALARM_MANAGER_H
#define RTC_ALARM_MANAGER_H

#include <uRTCLib.h>
#include "util/boot_process.hpp"
#include "util/nvs.hpp"
#include "util/timer.h"


// TODO: Test DS3231 alarms


/**
 * Class for managing the internal and external RTC and alarms
 * @note The external RTC stores the time as UTC
 */
class RtcAlarmManager final : BootProcess
{
    class Alarm;

public:
    RtcAlarmManager();

    //! Alarm 1 accessor
    Alarm& alarm1{m_alarm_1};
    //! Alarm 2 accessor
    Alarm& alarm2{m_alarm_2};

    /**
     * Check if either alarm 1 or alarm 2 was triggered
     * @return true if any alarm was triggered
     */
    [[nodiscard]] bool anyAlarmTriggered();

    /**
     * Update the internal RTC with the time from the external RTC
     */
    void setInternalFromExternal();

    /**
     * Update the external RTC with the time from the internal RTC
     */
    void setExternalFromInternal();

    /**
     * Get an accessor the system timezone represented as POSIX TZ string
     * @return Reference to the timezone value
     */
    [[nodiscard]] NVV<String>& timezone();

    /**
     * Get the temperature of the external RTC chip
     * @return The temperature of the external RTC chip in °C
     */
    [[nodiscard]] float temperature();

    // delete copy constructor and assignment operator

    RtcAlarmManager(const RtcAlarmManager&) = delete;
    RtcAlarmManager& operator=(const RtcAlarmManager&) = delete;

private:
    void runBootProcess() override;

    static void alarmISR();

    /**
     * Alarm class storing the alarm data and being responsible for setting the alarm
     */
    class [[gnu::packed]] Alarm
    {
    public:
        /**
         * The alarm hour; changing the value will set the alarm
         */
        NVV<uint8_t> hour;
        /**
         * The alarm minute; changing the value will set the alarm
         */
        NVV<uint8_t> minute;
        /**
         * The days of the week the alarm should repeat at as a bitmask [0: sunday, ..., 6: saturday]
         */
        NVV<uint8_t> repeat;
        /**
         * The sound number to be played when this alarm triggers
         */
        NVV<uint8_t> sound;
        /**
         * Controls whether the alarm is enabled; changing the value will either set or disable the alarm
         */
        NVV<bool> enabled;

        /**
         * Set the alarm time to trigger in 8 hours from now
         */
        void setIn8h();

        /**
         * Get the next time the alarm will ring at (if enabled)
         * @return Epoch timestamp (UTC) the alarm will next trigger at
         */
        [[nodiscard]] time_t next() const;

    private:
        friend class RtcAlarmManager;

        Alarm(const char* key_hour,
              const char* key_minute,
              const char* key_repeat,
              const char* key_sound,
              const char* key_enabled,
              RtcAlarmManager& mgr,
              uint8_t id);
        void setAt(const time_t& t) const;
        void set();
        void computeNextAndSet();

        RtcAlarmManager& m_mgr;
        time_t m_next{};
        uint8_t m_id;
        bool m_no_compute = false;
    };

    uRTCLib m_rtc;
    Alarm m_alarm_1{
        "a1hour",
        "a1minute",
        "a1repeat",
        "a1sound",
        "a1enabled",
        *this, URTCLIB_ALARM_1
    };
    Alarm m_alarm_2{
        "a2hour",
        "a2minute",
        "a2repeat",
        "a2sound",
        "a2enabled",
        *this, URTCLIB_ALARM_2
    };
    Timer m_update_timer;
    Timer m_deactivate_timer;
    NVV<String> m_timezone{"timezone", "UTC"};
};


#endif //RTC_ALARM_MANAGER_H
