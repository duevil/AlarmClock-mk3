#ifndef WEATHER_H
#define WEATHER_H

#include "alarm_clock.h"

namespace weather {
    struct data {
        String description;
        float temp;
        float feels_like;
        float wind_speed;
        String wind_direction;
    };

    void setup();
    const data &get();
    String dataString();
}

#endif //WEATHER_H
