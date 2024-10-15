#include "alarm_clock.h"

void setup() {
    log_setup(&log_vprintf);

#ifdef WOKWI // Wokwi simulation uses pre-defined and fixed Wi-Fi credentials
    WiFi.begin(WIFI_SSID, WIFI_PASS, 6);
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
    }
#endif

    ui::setup();
    ui::runBootProcess(UI_BOOT_PROCESS_LIST);
    ui::setUIForms(UI_FUNCTION_LIST, UI_FORM_DEFINITION);
}

void loop() {
    static uint8_t light = 0;
    ui::loop();
    switch (ui::getInputEvent()) {
        using
        enum ui::ie_t;
        case M_L:
            if (light) {
                lights::fadeDown();
                light = 0;
            } else {
                lights::fadeUp();
                light = 100;
            }
            break;
        case L_S:
            light = light > 10 ? light - 10 : 100;
            lights::fade(light);
            break;
        case R_S:
            light = light < 100 ? light + 10 : 0;
            lights::fade(light);
            break;
        default:
            break;
    }

#ifdef WOKWI
    // Wokwi simulation needs a small delay to run properly
    delay(10);
#endif

    sensors::loop();
}
