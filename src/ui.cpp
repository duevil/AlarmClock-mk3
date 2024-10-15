#include "ui.h"

// TODO:
//  Currently the UI was only tested inside the Wokwi simulator.
//  Testing the UI with real hardware is required.

static constexpr auto LONG_PRESS_DURATION = 300;

struct Button {
    Bounce2::Button bb{};
    bool long_press{false};
    uint8_t pin;

    explicit Button(uint8_t pin) : pin(pin) {}

    void begin() {
        bb.attach(pin, INPUT);
        bb.interval(5);
        bb.setPressedState(HIGH);
    }

    // return 0 if not pressed, 1 if pressed short, 2 if pressed long
    uint8_t read() {
        if (bb.isPressed() && !long_press && bb.currentDuration() > LONG_PRESS_DURATION) {
            long_press = true;
            return 2;
        }
        if (bb.released()) {
            long_press = false;
            if (bb.previousDuration() < LONG_PRESS_DURATION) {
                return 1;
            }
        }
        return 0;
    }

    [[nodiscard]] const char *name() const {
        switch (pin) {
            case pins::MIDDLE_BUTTON:
                return "Middle";
            case pins::LEFT_BUTTON:
                return "Left";
            case pins::RIGHT_BUTTON:
                return "Right";
            default:
                return "Unknown";
        }
    }
};

static void drawBootScreen(const char *, uint8_t);
static ui::ie_t readInputEvent();
static void handleInputEvent();
static void leaveForm();

static Button middleButton{pins::MIDDLE_BUTTON};
static Button leftButton{pins::LEFT_BUTTON};
static Button rightButton{pins::RIGHT_BUTTON};
#ifndef WOKWI
static U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI u8g2(U8G2_R0, pins::DISPLAY_CS, pins::DISPLAY_DC, pins::DISPLAY_RST);
#else
static U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
#endif
static MUIU8G2 mui{};
static Ticker leaveFormTimer{};
static bool redraw = false;
static ui::ie_t inputEvent = ui::ie_t::NONE;

/*!
 * @brief Setup the UI
 */
void ui::setup() {
    middleButton.begin();
    leftButton.begin();
    rightButton.begin();
    u8g2.begin();
}

/*!
 * @brief Run the boot process. For a given list of boot process steps, display an animated progress bar
 * and execute the function associated with each step
 * @param lpList The list of boot process steps
 */
void ui::runBootProcess(std::span<ui_bp_t> lpList) {
    for (const auto &lp: lpList) {
        log_i("Running %s [%d%%]", lp.message, lp.progress);
        if (lp.func) lp.func();
        log_d("Done");
        drawBootScreen(lp.message, lp.progress);
    }
    u8g2.clearDisplay();
}

/*!
 * @brief Set the UI forms and start the UI
 * @param muifList The list of UI functions
 * @param fds The form definition string
 */
void ui::setUIForms(std::span<muif_t> muifList, fds_t *fds) {
    mui.begin(u8g2, fds, muifList.data(), muifList.size());
}

/*!
 * @brief Run the UI loop. Handles input events and redraws the display
 */
void ui::loop() {
    middleButton.bb.update();
    leftButton.bb.update();
    rightButton.bb.update();

    handleInputEvent();

    if (redraw) {
        u8g2.clearBuffer();
        if (mui.isFormActive()) mui.draw();
        u8g2.sendBuffer();
        redraw = false;
        if (mui.isFormActive()) {
            // exit form after 20 seconds of inactivity
            leaveFormTimer.once(20, leaveForm);
        }
    }
}

/*!
 * @brief Check if a form is currently active
 * @return True if a form is active, false otherwise
 */
bool ui::isFormActive() { return mui.isFormActive(); }

/*!
 * @brief Get the current input event. Only events that are not consumed by the UI are returned
 * @return The current input event
 */
ui::ie_t ui::getInputEvent() {
    auto event = inputEvent;
    inputEvent = ui::ie_t::NONE;
    return event;
}

static void drawBootScreen(const char *message, const uint8_t progress) {
    // TODO: Rework boot screen design

    static uint8_t last_progress = 0;
    // animate progress bar to next value
    char buffer[32];
    for (uint8_t i = last_progress; i <= progress; i++) {

        u8g2.clearBuffer();

        u8g2.drawFrame(11, 21, 105, 17);
        u8g2.drawBox(13, 23, i, 13);
        u8g2.setFont(fonts::helv<8>);

        sprintf(buffer, "Progress: %d%%", i);
        u8g2.drawStr(34, 50, buffer);

        u8g2.setFont(fonts::helv<8>);
        u8g2.drawStr(1, 8, message);
        u8g2.drawLine(1, 11, 126, 11);

        u8g2.sendBuffer();

        delay(5);
    }
    delay(250);
    last_progress = progress;
}

static ui::ie_t readInputEvent() {
    using
    enum ui::ie_t;
    if (auto state = middleButton.read(); state) {
        if (state == 1) {
            log_d("Middle button short press");
            return M_S;
        } else {
            log_d("Middle button long press");
            return M_L;
        }
    }
    if (auto state = leftButton.read(); state) {
        if (state == 1) {
            log_d("Left button short press");
            return L_S;
        } else {
            log_d("Left button long press");
            return L_L;
        }
    }
    if (auto state = rightButton.read(); state) {
        if (state == 1) {
            log_d("Right button short press");
            return R_S;
        } else {
            log_d("Right button long press");
            return R_L;
        }
    }
    return NONE;
}

static void handleInputEvent() {
    using
    enum ui::ie_t;
    auto ie = readInputEvent();
    if (mui.isFormActive()) {
        // handle event when a form is active
        switch (ie) {
            case M_S:
                mui.sendSelect();
                log_v("Sending select");
                redraw = true;
                break;
            case L_S:
                mui.prevField();
                log_v("Sending prev field");
                redraw = true;
                break;
            case R_S:
                mui.nextField();
                log_v("Sending next field");
                redraw = true;
                break;
            case L_L:
            case R_L:
                // either left or right long press will execute a field search
                mui.sendSelectWithExecuteOnSelectFieldSearch();
                log_v("Executing field search");
                redraw = true;
                break;
            default:
                break;
        }
    } else if (ie == M_S) {
        // when no form is active, enter form 1 on middle button short press
        log_v("Entering form 1");
        mui.gotoForm(1, 0);
        redraw = true;
    }
    if (!redraw) {
        // we had no event that was consumed by the UI,
        // so we can pass it to the main loop
        inputEvent = ie;
    }
}

static void leaveForm() {
    log_d("Leaving form");
    mui.leaveForm();
    redraw = true;
}
