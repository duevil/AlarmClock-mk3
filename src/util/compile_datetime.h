#ifndef COMPILE_DATETIME_H
#define COMPILE_DATETIME_H


/**
 * Set the internal rtc from the compilation datetime given by <code>__DATE__</code> and <code>__TIME__</code>
 * @note The time is handled without any timezone information and thus treated as being in
 * whatever timezone the system is currently set to
 */
inline void set_internal_rtc_from_compile_datetime()
{
    tm tm{.tm_isdst = -1};
    strptime(__DATE__ " " __TIME__, "%b %d %Y %T", &tm);
    timeval tv{.tv_sec = (mktime(&tm))};
    settimeofday(&tv, nullptr);
}


#endif //COMPILE_DATETIME_H
