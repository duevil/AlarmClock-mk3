#ifndef LIGHTS_H
#define LIGHTS_H

#include <alarm_clock.h>

namespace lights {
    void setup();
    void set(uint8_t = 100);
    uint32_t get();
    void fade(uint8_t = 100);
    void fadeUp();
    void fadeDown();
}

#endif //LIGHTS_H
