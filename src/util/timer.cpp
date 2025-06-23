#include "timer.h"


bool Timer::detached(uint16_t seconds, const callback_t& callback)
{
    auto timer = xTimerCreate(nullptr, pdMS_TO_TICKS(seconds * 1000), pdFALSE, reinterpret_cast<void*>(true),
                              s_callback);
    s_callbacks.try_emplace(timer, callback);
    if (timer)
    {
        return xTimerStart(timer, 0) == pdTRUE;
    }
    return false;
}

Timer::~Timer()
{
    if (m_timer)
    {
        xTimerDelete(m_timer, 0);
    }
}

bool Timer::once(uint16_t seconds, const callback_t& callback, bool start)
{
    return create(seconds, callback, pdFALSE, start);
}

bool Timer::always(uint16_t seconds, const callback_t& callback, bool start)
{
    return create(seconds, callback, pdTRUE, start);
}

void Timer::stop() const
{
    if (m_timer)
    {
        xTimerStop(m_timer, 0);
    }
}

void Timer::start() const
{
    if (m_timer)
    {
        xTimerStart(m_timer, 0);
    }
}

void Timer::reset() const
{
    if (m_timer)
    {
        xTimerReset(m_timer, 0);
    }
}

void Timer::changePeriod(uint16_t seconds) const
{
    if (m_timer)
    {
        xTimerChangePeriod(m_timer, pdMS_TO_TICKS(seconds * 1000), 0);
    }
}

void Timer::setReload(BaseType_t reload) const
{
    if (m_timer)
    {
        vTimerSetReloadMode(m_timer, reload);
    }
}

void Timer::s_callback(TimerHandle_t timer)
{
    s_callbacks.at(timer)();
    if (static_cast<bool>(pvTimerGetTimerID(timer)))
    {
        xTimerDelete(timer, 0);
        s_callbacks.erase(timer);
    }
}

bool Timer::create(uint16_t seconds, const callback_t& callback, BaseType_t reload, bool start)
{
    if (m_timer)
    {
        xTimerDelete(m_timer, 0);
    }
    m_timer = xTimerCreate(nullptr, pdMS_TO_TICKS(seconds * 1000), reload, nullptr, s_callback);
    s_callbacks.try_emplace(m_timer, callback);
    if (start && m_timer)
    {
        xTimerStart(m_timer, 0);
    }
    return m_timer;
}
