#ifndef AUDIO_H
#define AUDIO_H

#include "alarm_clock.h"

namespace audio {
    void setup();
    void loop();
    bool isPlaying();
    void play(const Sound & = Sound::S_DEFAULT, bool = false);
    void stop();
    void volume(uint8_t = 80);
    void volumeUp();
    void volumeDown();
}

#endif //AUDIO_H
