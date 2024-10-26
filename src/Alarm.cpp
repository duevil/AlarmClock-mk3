#include "Alarm.hpp"

Alarm Alarm::A1 = N::A1;
Alarm Alarm::A2 = N::A2;

static const char *alarmStr(const Alarm::N &n, const char *str) {
    auto temp = "a" + String(std::to_underlying(n)) + str;
    auto c_str = new char[temp.length() + 1];
    strcpy(c_str, temp.c_str());
    return c_str;
}

Alarm::Alarm(N n) :
        n{std::to_underlying(n)},
        enabled{alarmStr(n, "enabled"), false},
        hour{alarmStr(n, "hour"), 0},
        minute{alarmStr(n, "minute"), 0},
        repeat{alarmStr(n, "repeat"), 0},
        sound{alarmStr(n, "sound"), 0} {}

DateTime Alarm::getAlarmTime() const {
    auto alarmTime = DateTime{
            global::now.year(),
            global::now.month(),
            global::now.day(),
            *hour,
            *minute,
            0
    };
    auto dayOfWeek = alarmTime.dayOfTheWeek();

    // If the alarm is not set to repeat, and the alarm time is in the past, return the next day
    const auto repeat_i = *repeat;
    if (repeat_i == 0) {
        if (alarmTime < global::now) {
            alarmTime = alarmTime + TimeSpan(1, 0, 0, 0);
        }
        return alarmTime;
    }

    // If the alarm is set to repeat, and the alarm time is in the past, return the next weekday
    for (short i = 0; i < 7; ++i) {
        auto nextWeekday = (dayOfWeek + i) % 7;
        if (repeat_i & 1 << nextWeekday) {
            alarmTime = alarmTime + TimeSpan(i, 0, 0, 0);
            if (alarmTime > global::now) return alarmTime;
        }
    }

    return {};
}

void Alarm::alarmToJson(const JsonVariant &json) const {
    json["enabled"] = *enabled;
    json["hour"] = *hour;
    json["minute"] = *minute;
    json["repeat"] = *repeat;
    json["sound"] = *sound;
    json["nextDateTime"] = getAlarmTime() != DateTime() ? getAlarmTime().unixtime() : 0;
}

void Alarm::alarmFromJson(const JsonVariant &json) {
    enabled = json["enabled"].as<bool>();
    hour = json["hour"].as<uint8_t>();
    minute = json["minute"].as<uint8_t>();
    repeat = json["repeat"].as<uint8_t>();
    sound = json["sound"].as<uint8_t>();
}
