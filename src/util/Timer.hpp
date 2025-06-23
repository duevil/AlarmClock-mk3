#ifndef TIMER_HPP
#define TIMER_HPP

#include <unordered_map>
#include <functional>


struct Timer
{
    using callback_t = std::function<void()>;

    static void detached(uint16_t seconds, const callback_t& callback)
    {
        auto timer = xTimerCreate(nullptr, pdMS_TO_TICKS(seconds * 1000), pdFALSE, reinterpret_cast<void*>(true),
                                  s_callback);
        s_callbacks.emplace(timer, callback);
        if (timer)
        {
            xTimerStart(timer, 0) == pdTRUE;
        }
    }

    ~Timer()
    {
        if (m_timer)
        {
            xTimerDelete(m_timer, 0);
        }
    }

    bool once(uint16_t seconds, const callback_t& callback, bool start = false)
    {
        return create(seconds, callback, pdFALSE, start);
    }

    bool always(uint16_t seconds, const callback_t& callback, bool start = false)
    {
        return create(seconds, callback, pdTRUE, start);
    }

    void stop() const
    {
        if (m_timer)
        {
            xTimerStop(m_timer, 0);
        }
    }

    void start() const
    {
        if (m_timer)
        {
            xTimerStart(m_timer, 0);
        }
    }

    void reset() const
    {
        if (m_timer)
        {
            xTimerReset(m_timer, 0);
        }
    }

    void changePeriod(uint16_t seconds) const
    {
        if (m_timer)
        {
            xTimerChangePeriod(m_timer, pdMS_TO_TICKS(seconds * 1000), 0);
        }
    }

    void setReload(BaseType_t reload) const
    {
        if (m_timer)
        {
            vTimerSetReloadMode(m_timer, reload);
        }
    }

private:
    inline static std::unordered_map<TimerHandle_t, callback_t> s_callbacks{};
    TimerHandle_t m_timer{};

    static void s_callback(TimerHandle_t timer)
    {
        s_callbacks.at(timer)();
        if (static_cast<bool>(pvTimerGetTimerID(timer)))
        {
            xTimerDelete(timer, 0);
            s_callbacks.erase(timer);
        }
    }

    bool create(uint16_t seconds, const callback_t& callback, BaseType_t reload, bool start)
    {
        if (m_timer)
        {
            xTimerDelete(m_timer, 0);
        }
        m_timer = xTimerCreate(nullptr, pdMS_TO_TICKS(seconds * 1000), reload, nullptr, s_callback);
        s_callbacks.emplace(m_timer, callback);
        if (start && m_timer)
        {
            xTimerStart(m_timer, 0);
        }
        return m_timer;
    }
};


#endif //TIMER_HPP
