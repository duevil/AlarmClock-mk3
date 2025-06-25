#ifndef UI_DISPLAY_MANAGER_H
#define UI_DISPLAY_MANAGER_H

#include <U8g2lib.h>
#include <MUIU8g2.h>
#include <util/blocking_queue.hpp>

#include "util/boot_process.hpp"
#include "util/thread.hpp"
#include "util/timer.h"


class UiDisplayManager final : BootProcess, Thread<4096>
{
public:
    UiDisplayManager(uint8_t pin_cs, uint8_t pin_dc, uint8_t pin_rst,
                     fds_t* form_definitions, std::initializer_list<muif_struct> fields);

    /**
     * Go to a specific form by the given ID
     * @param form_id The form id to go to
     */
    void goTo(uint8_t form_id);

    /**
     * Enter the UI form
     */
    void enter();

    /**
     * Exit the UI form
     */
    void exit();

    /**
     * Send UI action next, e.g., highlighting the next form field or increasing a number value
     */
    void actionNext();

    /**
     * UI action prev, e.g., highlighting the previous form fiel or decrementing a number value
     */
    void actionPrev();

    /**
     * UI action select, e.g., selecting the highlighted field or clicking a button
     */
    void actionSelect();

    /**
     * Redraw the UI
     */
    void redraw();

    /**
     * Check if the UI is active
     * @return If the UI form is currently active
     */
    bool active();

private:
    void runBootProcess() override;
    void run() override;

    const std::vector<muif_struct> m_fields;
    U8G2 m_display;
    MUIU8G2 m_ui{};
    Timer m_close_timer;
    ESPQueue<10, const char*> m_custom_draws{};
};


#endif //UI_DISPLAY_MANAGER_H
