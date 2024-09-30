#ifndef ALARM_CLOCK_H
#define ALARM_CLOCK_H

// std includes
#include <span>
// Arduino library includes
#include <Arduino.h>
#include <Ticker.h>
#include <Preferences.h>
#include <SD.h>
#include <sntp.h>
#include <WiFi.h>
// external library includes
#include <Bounce2.h>
#include <U8g2lib.h>
#include <MUIU8g2.h>
#include <RTClib.h>
// own includes
#include "secrets.h"
#include "globals.h"
#include "pinmap.h"
#include "log.h"
#include "lights.h"
#include "nvs.h"
#include "ui_boot_process.h"
#include "fonts.h"
#include "ui.h"
#include "rtc.h"

int log_vprintf(const char *, va_list);

extern const std::span<ui_bp_t> UI_BOOT_PROCESS_LIST;
extern const std::span<muif_t> UI_FUNCTION_LIST;
extern fds_t *UI_FORM_DEFINITION;

#endif //ALARM_CLOCK_H
