#ifndef UI_H
#define UI_H

#include "alarm_clock.h"

namespace ui {
    //! @brief Enum for button input events.
    enum class ie_t {
        NONE, //!< No input event.
        M_S, //!< Middle button short press.
        M_L, //!< Middle button long press.
        L_S, //!< Left button short press.
        L_L, //!< Left button long press.
        R_S, //!< Right button short press.
        R_L, //!< Right button long press.
    };

    void setup();
    void runBootProcess(std::span<ui_bp_t>);
    void setUIForms(std::span<muif_t>, fds_t *);
    void loop();
    bool isFormActive();
    ie_t getInputEvent();
}

#endif //UI_H
