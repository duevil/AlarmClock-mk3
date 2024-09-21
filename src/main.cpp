#include "alarm_clock.h"

void setup() {
    log_setup();
    delay(100);

    esp_log_set_vprintf(&log_vprintf);
}

void loop() {
}
