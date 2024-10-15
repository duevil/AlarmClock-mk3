#ifndef ALARM_CLOCK_SENSORS_H
#define ALARM_CLOCK_SENSORS_H

#include "alarm_clock.h"

namespace sensors {

    void setup();
    void loop();
    float temperature();
    float humidity();
    float light();
}

#endif //ALARM_CLOCK_SENSORS_H
