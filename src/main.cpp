#include "alarm_clock.h"

nvs::Handler nvsHandler{
        nvs::Var{"log_file_index", global::logFileIndex},
        nvs::Var{"light_duration", global::lightDuration}
};

void setup() {
    log_setup();
    nvsHandler.setup();
    global::sdAvailable = SD.begin();
    if (global::sdAvailable) log_i("SD card initialized");
    else log_e("SD card initialization failed");
    esp_log_set_vprintf(&log_vprintf);

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
    if (!ui::isFormActive()) {
        nvsHandler.store_all();
    }
}
