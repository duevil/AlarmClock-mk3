#ifndef COMPILE_DATETIME_H
#define COMPILE_DATETIME_H


/**
 * Set the internal rtc from the compilation datetime given by <code>__DATE__</code> and <code>__TIME__</code>
 * @note The time is handled without any timezone information and thus treated as being UTC
 */
inline void set_internal_rtc_from_compile_datetime()
{
    tm tmFw{};
    strptime(__DATE__ " " __TIME__, "%b %d %Y %T", &tmFw);
    timeval tv{.tv_sec = (mktime(&tmFw))};
    settimeofday(&tv, nullptr);
}


#endif //COMPILE_DATETIME_H
