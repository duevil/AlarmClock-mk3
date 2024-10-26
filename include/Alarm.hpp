#ifndef ALARM_HPP
#define ALARM_HPP

#include "alarm_clock.h"


class Alarm {
public:
    enum class N : uint8_t {
        A1 = 0,
        A2 = 1
    };

    const uint8_t n;
    NVSValue<bool> enabled;
    NVSValue<uint8_t> hour;
    NVSValue<uint8_t> minute;
    NVSValue<uint8_t> repeat;
    NVSValue<uint8_t> sound;

    [[nodiscard]] DateTime getAlarmTime() const;
    void alarmToJson(const JsonVariant &json) const;
    void alarmFromJson(const JsonVariant &json);

    static Alarm A1;
    static Alarm A2;

private:
    explicit(false) Alarm(N);
};


#endif //ALARM_HPP
