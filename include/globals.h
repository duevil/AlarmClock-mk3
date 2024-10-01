#ifndef GLOBALS_H
#define GLOBALS_H

#include <alarm_clock.h>

namespace global {
    extern bool sdAvailable; //!< True if the SD card is available
    extern uint8_t logFileIndex; //!< Index of the current log file
    extern uint8_t lightDuration; //!< Duration for the lights to turn off after in minutes
}

#endif //GLOBALS_H
