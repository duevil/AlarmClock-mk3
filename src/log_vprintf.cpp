#include "alarm_clock.h"

int log_vprintf(const char *format, va_list args) {
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
#ifdef DEBUG
    Serial.print(buffer);
#endif
    // TODO: Implement writing to a log file
    return strlen(buffer);
}