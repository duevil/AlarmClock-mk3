#ifndef ALARM_CLOCK_H
#define ALARM_CLOCK_H

#include <Arduino.h>
#include <SD.h>
#include "globals.h"
#include "pinmap.h"
#include "log.h"
#include "lights.h"
#include "nvs.h"

int log_vprintf(const char *, va_list);

#endif //ALARM_CLOCK_H
