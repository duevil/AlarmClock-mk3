#include "alarm_clock.h"

NVS nvs({});

void setup() {
    log_setup();
    nvs.setup();

    esp_log_set_vprintf(&log_vprintf);
}

void loop() {
}
