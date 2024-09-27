#include "alarm_clock.h"

nvs::Handler nvsHandler{
        nvs::Var{"log_file_index", global::logFileIndex},
        nvs::Var{"light_duration", global::lightDuration}
};
uint8_t percent = 0;

void setup() {
    log_setup();
    nvsHandler.setup();
    if (SD.begin()) {
        global::sdAvailable = true;
    }
    lights::setup();


    esp_log_set_vprintf(&log_vprintf);
}

void loop() {
    nvsHandler.store_all();
    percent = percent < 100 ? percent + 10 : 0;
    lights::fade(percent);

    delay(2000);
    log_v("Loop [%ul]\n", millis());
}
