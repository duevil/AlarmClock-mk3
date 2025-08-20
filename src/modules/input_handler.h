#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <espasyncbutton.hpp>
#include "util/boot_process.hpp"


/**
 * Class for handling input events; observes button interactions and emits either
 * <code>CLICK_[LEFT|MIDDLE|RIGHT]</code> for all buttons,
 * <code>LONG_PRESS_MIDDLE</code> when long pressing the middle button
 * or <code>REPEATING_[LEFT|RIGHT]</code> repeatedly when holding the left or right button
 */
class InputHandler final : BootProcess
{
public:
    /**
     * Creates GPIO input handler for the given button pins
     * @param pin_l Left button pin
     * @param pin_m Middle button pin
     * @param pin_r Right button pin
     */
    InputHandler(gpio_num_t pin_l, gpio_num_t pin_m, gpio_num_t pin_r);

    // delete copy constructor and assignment operator

    InputHandler(const InputHandler&) = delete;
    InputHandler& operator=(const InputHandler&) = delete;

private:
    void runBootProcess() override;

    enum button_id { left, middle, right };

    template <button_id>
    struct EventPolicy
    {
        void event(ESPButton::event_t e, EventMsg* msg);
    };

    GPIOButton<EventPolicy<left>> m_btn_l;
    GPIOButton<EventPolicy<middle>> m_btn_m;
    GPIOButton<EventPolicy<right>> m_btn_r;
};


#endif //INPUT_HANDLER_H
