#include "matrix.h"

// TODO: Maybe add support for multiple tabs with animations sometime in the future

static MD_Parola md{MD_MAX72XX::FC16_HW, pins::MATRIX_CS, 4};
static Ticker drawTimer;
static char nowStr[9];

static char subscript(char c) {
    constexpr auto offset = 192 - '0';
    return static_cast<char>(offset + c);
}

static void matrixDrawTime() {
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
    md.setIntensity(15);
    md.setFont(MATRIX_FONT);
    md.setTextAlignment(PA_CENTER);
    md.setCharSpacing(0);
    md.setTextEffect(PA_NO_EFFECT, PA_NO_EFFECT);
    drawTimer.attach_ms(100, matrixDrawTime);
    matrixDrawTime();
}
