#include "matrix.h"

// TODO: Maybe add support for multiple tabs with animations sometime in the future

static MD_Parola md{MD_MAX72XX::FC16_HW, pins::MATRIX_CS, 4};
static uint32_t lastUpdate{0};
static char nowStr[9];

static char subscript(char c) {
    constexpr auto offset = 192 - '0';
    return static_cast<char>(offset + c);
}

static void matrixDrawTime() {
    auto lightLevel = sensors::light();
    const auto b = fabs(lightLevel) < 1e-6;
    md.displayShutdown(b);
    if (b) return;
    md.setIntensity(static_cast<uint8_t>(2.5f * log(lightLevel / 20.0f + 1.0f)));
    strcpy(nowStr, "hh:mm ss");
    global::now.toString(nowStr);
    nowStr[6] = subscript(nowStr[6]);
    nowStr[7] = subscript(nowStr[7]);
    md.setTextBuffer(nowStr);
    if (md.displayAnimate()) {
        md.displayReset();
    }
}

void matrix::setup() {
    auto res = md.begin();
    assert(res && "Matrix setup failed");
    (void) res; // Suppress unused variable warning
    md.setIntensity(0);
    md.setFont(MATRIX_FONT);
    md.setTextAlignment(PA_CENTER);
    md.setCharSpacing(0);
    md.setTextEffect(PA_NO_EFFECT, PA_NO_EFFECT);
    matrixDrawTime();
}

void matrix::loop() {
    if (millis() - lastUpdate > 100) {
        lastUpdate = millis();
        matrixDrawTime();
    }
}
