#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>
#include <unordered_map>
#include <functional>


/**
 * Timer class for calling an action after a given period, either once or repeating
 */
class Timer
{
    using callback_t = std::function<void()>;

public:
    /**
     * Creates a non-repeating detached timer instance that will delete itself after execution
     * @param seconds The seconds after which the timed action should occur
     * @param callback The callback to run on timer execution
     */
    static bool detached(uint16_t seconds, const callback_t& callback);

    /**
     * Deletes the timer
     */
    ~Timer();

    /**
     * Assigns a new one-shot callback to this timer
     * @param seconds The seconds after which the timed action should occur
     * @param callback The callback to run on timer execution
     * @param start Whether the timer should be started instantly
     * @return true if the timer was created successfully
     */
    bool once(uint16_t seconds, const callback_t& callback, bool start = false);

    /**
     * Assigns a new repeating callback to this timer
     * @param seconds The seconds after which the timed action should occur
     * @param callback The callback to run on timer execution
     * @param start Whether the timer should be started instantly
     * @return true if the timer was created successfully
     */
    bool always(uint16_t seconds, const callback_t& callback, bool start = false);

    /**
     * Stops the timer (can be restarted)
     */
    void stop() const;

    /**
     * Starts the timer
     */
    void start() const;

    /**
     * Resets the timer, starting it if not running or resetting the elapsed duration
     */
    void reset() const;

    /**
     * Change the period of this timer
     * @param seconds The period in seconds
     */
    void changePeriod(uint16_t seconds) const;

    /**
     * Change the timer type
     * @param reload Whether this timer should stop after execution or keep repeating
     */
    void setReload(BaseType_t reload) const;

private:
    inline static std::unordered_map<TimerHandle_t, callback_t> s_callbacks{};
    TimerHandle_t m_timer{};

    static void s_callback(TimerHandle_t timer);
    bool create(uint16_t seconds, const callback_t& callback, BaseType_t reload, bool start);
};


#endif //TIMER_H
