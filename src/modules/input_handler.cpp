#include "input_handler.h"

#include "event_definitions.h"
#include <esp32-hal-gpio.h>

using enum ESPButton::event_t;


InputHandler::InputHandler(gpio_num_t pin_l, gpio_num_t pin_m, gpio_num_t pin_r)
    : BootProcess("Inputs initialized"),
      m_btn_l(pin_l, HIGH), m_btn_m(pin_m, HIGH), m_btn_r(pin_r, HIGH) {}

void setup_button(auto& btn)
{
    btn.enableEvent(press, false);
    btn.enableEvent(release, false);
    btn.enableEvent(click);
    btn.enableEvent(longPress);
    btn.enableEvent(autoRepeat);
    btn.timeouts.setAutoRepeat(150);
    btn.enable();
}

void InputHandler::runBootProcess()
{
    setup_button(m_btn_l);
    setup_button(m_btn_m);
    setup_button(m_btn_r);
}

template <>
void InputHandler::EventPolicy<InputHandler::left>::event(ESPButton::event_t e, EventMsg*)
{
    switch (e)
    {
    case click:
        INPUT_EVENT << CLICK_LEFT;
        break;
    case autoRepeat:
        INPUT_EVENT << REPEATING_LEFT;
        break;
    default: break;
    }
}

template <>
void InputHandler::EventPolicy<InputHandler::middle>::event(ESPButton::event_t e, EventMsg*)
{
    switch (e)
    {
    case click:
        INPUT_EVENT << CLICK_MIDDLE;
        break;
    case longPress:
        INPUT_EVENT << LONG_PRESS_MIDDLE;
        break;
    default: break;
    }
}

template <>
void InputHandler::EventPolicy<InputHandler::right>::event(ESPButton::event_t e, EventMsg*)
{
    switch (e)
    {
    case click:
        INPUT_EVENT << CLICK_RIGHT;
        break;
    case autoRepeat:
        INPUT_EVENT << REPEATING_RIGHT;
        break;
    default: break;
    }
}
