#include "lights_controller.h"

#include "event_definitions.h"


uint8_t LightsController::getCurrentValue() const
{
    constexpr auto max = static_cast<float>((1LU << m_cfg.resolution) - 1);
    auto corrected = min(static_cast<float>(m_current), max) / max;
    auto normalized = pow(corrected, 1.0f / m_cfg.gamma);
    return static_cast<uint8_t>(normalized * 100.0f);
}

void LightsController::runBootProcess()
{
    ledcAttach(m_cfg.pin, m_cfg.freq, m_cfg.resolution);

    LIGHTS_EVENT >> SET_MAX >> [this](auto) { max(); };
    LIGHTS_EVENT >> SET_OFF >> [this](auto) { off(); };
    LIGHTS_EVENT >> SET_VALUE >> [this](const Event_t& e) { set(e.data<uint8_t>()); };

#ifndef WOKWI
    // WOKWI doesn't like this timer :(
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
        ledcWrite(m_cfg.pin, m_current);
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
    m_target = (1LU << m_cfg.resolution) - 1;

    LOG_D("Setting lights to maximum brightness");
    m_autoOffTimer.reset();
    resume();
}

void LightsController::off()
{
    m_target = 0;

    LOG_D("Setting lights to off");
    m_autoOffTimer.stop();
    resume();
}

void LightsController::set(uint8_t value)
{
    constexpr auto max = static_cast<float>((1LU << m_cfg.resolution) - 1);
    auto normalized = _min(value, 100) / 100.f;
    auto corrected = pow(normalized, m_cfg.gamma);
    m_target = static_cast<uint32_t>(min(corrected * max, max));

    LOG_D("Setting lights to %u %% (%lu)", value, m_target);
    m_autoOffTimer.reset();
    resume();
}

auto& LightsController::getDuration()
{
    return m_autoOffDuration;
}
