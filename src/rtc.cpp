#include "rtc.h"

#ifndef WOKWI
using RTC_TYPE = RTC_DS3231;
#else
using RTC_TYPE = RTC_DS1307;
#endif
static RTC_TYPE rtcObj{};
static DateTime nowObj{};
static bool rtcAvailable = false;
static Ticker updateNowTimer;
static Ticker updateInternetRTCTimer;

static DateTime time_tToDateTime(time_t time);
static void ntpCallback(struct timeval *);
static void updateNow();
static void updateInternetRTC();

/**
 * @brief Setup the RTC and enable NTP synchronization.
 */
void rtc::setup() {
    rtcAvailable = rtcObj.begin();
    if (rtcAvailable) {
#ifndef WOKWI
        if (!rtcObj.lostPower()) {
#else
        if (!rtcObj.isrunning()) {
#endif
            log_e("RTC is NOT running!");
            rtcObj.adjust(DateTime(__DATE__, __TIME__));
            rtcAvailable = false;
        } else {
            log_i("RTC is running");
            rtcAvailable = true;
        }
    } else {
        log_e("Couldn't find RTC");
    }

    configTzTime(global::timeZone.c_str(), "pool.ntp.org", "time.nist.gov", "time.google.com");
    sntp_set_time_sync_notification_cb(ntpCallback);
    updateNowTimer.attach_ms(100, updateNow);
    updateInternetRTCTimer.attach(60 * 60 * 1000, updateInternetRTC);
    updateInternetRTC();
}

/**
 * @brief Get the current time.
 * @return The current time as a DateTime object.
 */
const DateTime &rtc::now() { return nowObj; }

/**
 * @brief Update the system timezone using the global::timeZone variable.
 */
void rtc::updateTimeZone() {
    setenv("TZ", global::timeZone.c_str(), 1);
    tzset();
}

static DateTime time_tToDateTime(time_t time) {
    struct tm tm{};
    localtime_r(&time, &tm);
    return {
            static_cast<uint16_t>(tm.tm_year + 1900),
            static_cast<uint8_t>(tm.tm_mon + 1),
            static_cast<uint8_t>(tm.tm_mday),
            static_cast<uint8_t>(tm.tm_hour),
            static_cast<uint8_t>(tm.tm_min),
            static_cast<uint8_t>(tm.tm_sec)
    };
}

static void ntpCallback(struct timeval *tv) {
    rtcObj.adjust(time_tToDateTime(tv->tv_sec));
    log_i("Time synchronized from NTP: %s", rtcObj.now().timestamp().c_str());
}

static void updateNow() {
    nowObj = time_tToDateTime(time(nullptr));
    log_v("Updated now: %s", nowObj.timestamp().c_str());
}

static void updateInternetRTC() {
    auto _now = rtcObj.now();
    struct tm tm{
            _now.second(),
            _now.minute(),
            _now.hour(),
            _now.day(),
            _now.month() - 1,
            _now.year() - 1900,
    };
    time_t t = mktime(&tm);
    if (t != -1) {
        struct timeval tv{.tv_sec = t};
        const int res = settimeofday(&tv, nullptr);
        if (res == 0) {
            log_i("Internal RTC adjusted from RTC");
        } else {
            log_e("Could not set time of day");
        }
    }
}
