#ifndef RTC_H
#define RTC_H

#include "alarm_clock.h"

namespace rtc {
    void setup();
    const DateTime &now();
    void updateTimeZone();
}

#endif //RTC_H
