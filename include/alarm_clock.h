#ifndef ALARM_CLOCK_H
#define ALARM_CLOCK_H

// std includes
#include <span>
#include <sstream>
#include <unordered_set>
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
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <MD_MAX72xx.h>
#include <MD_Parola.h>
// own includes
#include "secrets.h"
#include "log.h"
#include "lights.h"
#include "nvs.h"
#include "ui_boot_process.h"
#include "fonts.h"
#include "ui.h"
#include "http_request.h"
#include "rtc.h"
#include "weather.h"
#include "matrix_font.h"
#include "matrix.h"

#define PIN_T inline constexpr uint8_t
namespace pins {
    PIN_T I2S_DATA = GPIO_NUM_26;
    PIN_T I2S_BCK = GPIO_NUM_25;
    PIN_T I2S_LRC = GPIO_NUM_4;
    PIN_T DISPLAY_DC = GPIO_NUM_7;
    PIN_T DISPLAY_CS = GPIO_NUM_8;
    PIN_T LIGHTS = GPIO_NUM_13;
    PIN_T DISPLAY_RST = GPIO_NUM_12;
    PIN_T MATRIX_CS = GPIO_NUM_27;
    PIN_T LEFT_BUTTON = GPIO_NUM_34;
    PIN_T MIDDLE_BUTTON = GPIO_NUM_39;
    PIN_T RIGHT_BUTTON = GPIO_NUM_36;
}

namespace global {
    inline bool sdAvailable{false}; //!< True if the SD card is available
    inline uint8_t logFileIndex{}; //!< Index of the current log file
    inline uint8_t lightDuration{45}; //!< Duration for the lights to turn off after in minutes
    inline float latitude{51.83119723675312}; //!< Latitude of the device
    inline float longitude{10.787628924924052}; //!< Longitude of the device
    inline DateTime now{}; //!< Current date and time
}

int log_vprintf(const char *, va_list);

extern const std::span<ui_bp_t> UI_BOOT_PROCESS_LIST;
extern const std::span<muif_t> UI_FUNCTION_LIST;
extern fds_t *UI_FORM_DEFINITION;

inline nvs::Handler nvsHandler{
        nvs::Var{"log_file_index", global::logFileIndex},
        nvs::Var{"light_duration", global::lightDuration},
        nvs::Var{"latitude", global::latitude},
        nvs::Var{"longitude", global::longitude},
}; //!< Handler for the non-volatile storage

#endif //ALARM_CLOCK_H
