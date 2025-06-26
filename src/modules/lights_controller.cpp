#include "lights_controller.h"


LightsController::LightsController(const Config& cfg)
    : BootProcess("Lights initialized"),
      Thread({.name = "lights", .priority = 5, .coreId = APP_CPU_NUM}),
      m_cfg(cfg) {}

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
    auto max = static_cast<float>((1LU << m_cfg.resolution) - 1);
    auto normalized = _min(value, 100) / 100.f;
    auto corrected = pow(normalized, m_cfg.gamma);
    m_target = static_cast<uint32_t>(min(corrected * max, max));

    LOG_D("Setting lights to %u %% (%lu)", value, m_target);
    m_autoOffTimer.reset();
    resume();
}

uint8_t LightsController::currentValue() const
{
    auto max = static_cast<float>((1LU << m_cfg.resolution) - 1);
    auto corrected = min(static_cast<float>(m_current), max) / max;
    auto normalized = pow(corrected, 1.0f / m_cfg.gamma);
    return static_cast<uint8_t>(normalized * 100.0f);
}

NVV<uint8_t>& LightsController::duration()
{
    return m_autoOffDuration;
}

void LightsController::runBootProcess()
{
    ledcAttach(m_cfg.pin, m_cfg.freq, m_cfg.resolution);

#ifndef WOKWI
    // WOKWI doesn't like this timer :(
    m_autoOffTimer.once(*m_autoOffDuration * 60, [this] { off(); });
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
                ledcFade(m_cfg.pin, m_current, m_target, m_cfg.fade_time);
                m_current = m_target;
                //delayMicroseconds(c_config.fade_time * 1000 / _abs(static_cast<int64_t>(m_target) - m_current));
                //m_current < m_target ? ++m_current : --m_current;
#endif
    }
    else
    {
        suspend();
    }
}
