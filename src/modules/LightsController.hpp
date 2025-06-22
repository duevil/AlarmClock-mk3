#ifndef LIGHTS_CONTROLLER_HPP
#define LIGHTS_CONTROLLER_HPP


namespace lights
{
    struct
    {
        uint8_t pin = pins::LIGHTS;
        uint32_t freq = 5000;
        uint8_t resolution = 13;
        uint16_t fade_time = 250;
        float gamma = 2.2;
        uint32_t max_duty = (1lu << resolution) - 1;
    } constexpr c_config{};


    struct Controller final : BootProcess, Thread<1024>
    {
        Controller() : BootProcess("Lights initialized"),
                       Thread({.name = "lights", .priority = 10, .coreId = APP_CPU_NUM}) {}

        [[nodiscard]] uint8_t getCurrentValue() const
        {
            auto corrected = static_cast<float>(_min(m_current, c_config.max_duty)) / c_config.max_duty;
            auto normalized = pow(corrected, 1.0f / c_config.gamma);
            return static_cast<uint8_t>(normalized * 100.0f);
        }

        [[nodiscard]] const auto& getDuration()
        {
            return m_autoOffDuration;
        }

        void setDuration(const uint8_t duration)
        {
            m_autoOffDuration = duration;
            m_autoOffTimer.changePeriod(duration * 60);
        }

    private:
        void runBootProcess() override
        {
            ledcAttach(c_config.pin, c_config.freq, c_config.resolution);

            LIGHTS_EVENT >> SET_MAX >> [this](auto) { max(); };
            LIGHTS_EVENT >> SET_OFF >> [this](auto) { off(); };
            LIGHTS_EVENT >> SET_VALUE >> [this](const Event_t& e) { set(e.data<uint8_t>()); };

            m_autoOffTimer.once(*m_autoOffDuration * 60, [] { LIGHTS_EVENT << SET_OFF; });
        }

        void run() override
        {
            if (m_current != m_target)
            {
                delayMicroseconds(c_config.fade_time * 1000 / _abs(static_cast<int64_t>(m_target) - m_current));
                m_current < m_target ? ++m_current : --m_current;
                ledcWrite(c_config.pin, m_current);
            }
            else
            {
                LIGHTS_EVENT << FADE_DONE;
                suspend();
            }
        }

        void max()
        {
            LOG_D("Setting lights to maximum brightness");
            m_target = c_config.max_duty;
            m_autoOffTimer.reset();
            resume();
        }

        void off()
        {
            LOG_D("Setting lights to off");
            m_target = 0;
            m_autoOffTimer.stop();
            resume();
        }

        void set(uint8_t value)
        {
            auto normalized = _min(value, 100) / 100.f;
            auto corrected = pow(normalized, c_config.gamma);
            m_target = _min(corrected * c_config.max_duty, c_config.max_duty);
            LOG_D("Setting lights to %u %% (%lu)", value, m_target);
            m_autoOffTimer.reset();
            resume();
        }

        uint32_t m_target = 0;
        uint32_t m_current = 0;
        NVV<uint8_t> m_autoOffDuration{"light_duration"};
        Timer m_autoOffTimer;
    };
}

using LightsController = lights::Controller;


#endif //LIGHTS_CONTROLLER_HPP
