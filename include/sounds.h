#ifndef SOUNDS_H
#define SOUNDS_H

#include "alarm_clock.h"

namespace sounds {
    void setup();
    using SoundCol = std::vector<Sound>;
    const SoundCol &get();
    const Sound &get(uint8_t index);
    JsonDocument json();
    void saveToSD();
    const Sound & getRandom();
}

#endif //SOUNDS_H
