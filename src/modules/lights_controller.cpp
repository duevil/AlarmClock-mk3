#include "lights_controller.h"

#include "event_definitions.h"
#include "pin_map.h"


struct
{
    uint8_t pin = pins::lights;
    uint32_t freq = 5000;
    uint8_t resolution = 13;
    uint16_t fade_time = 250;
    float gamma = 2.2f;
    uint32_t max_duty = (1LU << resolution) - 1;
} constexpr c_config{};


uint8_t LightsController::getCurrentValue() const
{
    auto corrected = static_cast<float>(_min(m_current, c_config.max_duty)) / c_config.max_duty;
    auto normalized = pow(corrected, 1.0f / c_config.gamma);
    return static_cast<uint8_t>(normalized * 100.0f);
}

void LightsController::runBootProcess()
{
    ledcAttach(c_config.pin, c_config.freq, c_config.resolution);

    LIGHTS_EVENT >> SET_MAX >> [this](auto) { max(); };
    LIGHTS_EVENT >> SET_OFF >> [this](auto) { off(); };
    LIGHTS_EVENT >> SET_VALUE >> [this](const Event_t& e) { set(e.data<uint8_t>()); };

#ifndef WOKWI
           m_autoOffTimer.once(*m_autoOffDuration * 60, [] { LIGHTS_EVENT << SET_OFF; });
#endif
    m_autoOffDuration.observe([this](uint8_t duration) { m_autoOffTimer.changePeriod(duration * 60); });
}

void LightsController::run()
{
    if (m_current != m_target)
    {
#ifdef WOKWI
        // WOKWI simulation can't handle fading
        m_current = m_target;
        ledcWrite(c_config.pin, m_current);
#else
                ledcFade(c_config.pin, m_current, m_target, c_config.fade_time);
                m_current = m_target;
                //delayMicroseconds(c_config.fade_time * 1000 / _abs(static_cast<int64_t>(m_target) - m_current));
                //m_current < m_target ? ++m_current : --m_current;
#endif
    }
    else
    {
        LIGHTS_EVENT << FADE_DONE;
        suspend();
    }
}

void LightsController::max()
{
    LOG_D("Setting lights to maximum brightness");
    m_target = c_config.max_duty;
    m_autoOffTimer.reset();
    resume();
}

void LightsController::off()
{
    LOG_D("Setting lights to off");
    m_target = 0;
    m_autoOffTimer.stop();
    resume();
}

void LightsController::set(uint8_t value)
{
    auto normalized = _min(value, 100) / 100.f;
    auto corrected = pow(normalized, c_config.gamma);
    m_target = static_cast<uint32_t>(min(corrected * c_config.max_duty, c_config.max_duty * 1.f));
    LOG_D("Setting lights to %u %% (%lu)", value, m_target);
    m_autoOffTimer.reset();
    resume();
}

auto& LightsController::getDuration()
{
    return m_autoOffDuration;
}
