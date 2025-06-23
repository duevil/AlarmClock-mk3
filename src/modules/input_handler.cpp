#include "input_handler.h"

#include "event_definitions.h"
#include <esp32-hal-gpio.h>


struct BtnEvt
{
    int32_t gpio;
    INPUT_EVENT_ID evt;
};

void assign_events(auto e_in, auto... be)
{
    events::Base{EBTN_EVENTS} >> e_in >> [be...](const Event_t& e)
    {
        auto gpio = e.data<EventMsg>().gpio;
        ((be.gpio == gpio ? INPUT_EVENT << be.evt, 0 : 0), ...);
    };
}


InputHandler::InputHandler(gpio_num_t pin_l, gpio_num_t pin_m, gpio_num_t pin_r)
    : BootProcess("Inputs initialized"),
      m_btn_l(pin_l, HIGH), m_btn_m(pin_m, HIGH), m_btn_r(pin_r, HIGH) {}

void InputHandler::runBootProcess()
{
    using enum ESPButton::event_t;
    for (auto btn : {&m_btn_l, &m_btn_m, &m_btn_r})
    {
        btn->enableEvent(press, false);
        btn->enableEvent(release, false);
        btn->enableEvent(click);
        btn->enableEvent(longPress);
        btn->enableEvent(longRelease);
        btn->enable();
    }
    assign_events(click,
                  BtnEvt{m_btn_l.getGPIO(), CLICK_LEFT},
                  BtnEvt{m_btn_m.getGPIO(), CLICK_MIDDLE},
                  BtnEvt{m_btn_r.getGPIO(), CLICK_RIGHT});
    assign_events(longPress,
                  BtnEvt{m_btn_l.getGPIO(), LONG_PRESS_LEFT},
                  BtnEvt{m_btn_m.getGPIO(), LONG_PRESS_MIDDLE},
                  BtnEvt{m_btn_r.getGPIO(), LONG_PRESS_RIGHT});
    assign_events(longRelease,
                  BtnEvt{m_btn_l.getGPIO(), LONG_RELEASE_LEFT},
                  BtnEvt{m_btn_m.getGPIO(), LONG_RELEASE_MIDDLE},
                  BtnEvt{m_btn_r.getGPIO(), LONG_RELEASE_RIGHT});
}
