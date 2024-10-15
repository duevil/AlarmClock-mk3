#include "rtc.h"

// TODO: Add alarm setup and handling functions

static inline constexpr const auto DEFAULT_TZ = "CET-1CEST,M3.5.0,M10.5.0/3";
static inline constexpr const auto NTP_SERVER_1 = "pool.ntp.org";
static inline constexpr const auto NTP_SERVER_2 = "time.nist.gov";
static inline constexpr const auto NTP_SERVER_3 = "time.google.com";
static inline constexpr const auto TIMEZONE_API_URL = "http://ip-api.com/line/?fields=timezone";
static inline constexpr const auto NTP_SYNC_INTERVAL = 12 * 60 * 60 * 1000; // 12 hours

#ifndef WOKWI
using RTC_TYPE = RTC_DS3231;
#else
using RTC_TYPE = RTC_DS1307;
#endif
static RTC_TYPE rtcObj{};
static bool rtcAvailable = false;
static Ticker updateNowTimer;
static Ticker updateInternetRTCTimer;

static DateTime time_tToDateTime(time_t);
static void ntpCallback(struct timeval *);
static void updateNow();
static void updateInternalRTC();
static bool setTimeZone(const String &);

/*!
 * @brief Setup the RTC and enable NTP synchronization.
 */
void rtc::setup() {
    rtcAvailable = rtcObj.begin();
    if (rtcAvailable) {
#ifndef WOKWI
        if (rtcObj.lostPower()) {
#else
            if (!rtcObj.isrunning()) {
#endif
            log_w("RTC is NOT running!");
            rtcObj.adjust(DateTime(__DATE__, __TIME__));
        } else {
            log_i("RTC is running");
        }
    } else {
        log_e("Couldn't find RTC");
    }

    configTzTime(DEFAULT_TZ, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);
    esp_sntp_set_time_sync_notification_cb(ntpCallback);
    esp_sntp_set_sync_interval(NTP_SYNC_INTERVAL);
    updateNowTimer.attach_ms(100, updateNow);
    updateInternetRTCTimer.attach(60 * 60 * 1000, updateInternalRTC);
    updateInternalRTC();
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
    auto rtcOld = rtcObj.now();
    auto rtcNew = time_tToDateTime(tv->tv_sec);
    if (auto diff = (rtcNew - rtcOld).totalseconds(); abs(diff) > 1) {
        rtcObj.adjust(rtcNew);
        rtcNew = rtcObj.now();
        log_w("RTC adjusted from NTP: %s (%+d seconds)", rtcNew.timestamp().c_str(), diff);
    } else {
        log_i("RTC in sync with NTP: %s", rtcNew.timestamp().c_str());
    }
    // Get the current timezone based on the IP address and update the system timezone
    http_request::get(TIMEZONE_API_URL, setTimeZone);
}

static void updateNow() { global::now = time_tToDateTime(time(nullptr)); }

static void updateInternalRTC() {
    if (!rtcAvailable) {
        return; // Don't update internal RTC with invalid time
    }
    auto _now = rtcObj.now();
    struct tm tm{
            .tm_sec = _now.second(),
            .tm_min = _now.minute(),
            .tm_hour = _now.hour(),
            .tm_mday = _now.day(),
            .tm_mon = _now.month() - 1,
            .tm_year = _now.year() - 1900,
            .tm_isdst = -1
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

static bool setTimeZone(const String &tzRef) {
    // roughly taken from https://github.com/mathieucarbou/MycilaNTP/blob/main/src/MycilaNTP.cpp

    auto tz = tzRef; // copy
    tz.trim();
    auto len = tz.length();
    if (len == 0) return false;

    auto withEqual = tz + "=";
    auto found = strstr(TIMEZONE_DB, withEqual.c_str());

    if (found == nullptr) {
        log_w("Timezone not found: %s", tz.c_str());
        return false;
    }

    auto start = found + len + 1;
    auto _tz = String(start, static_cast<unsigned int>(strstr(start, "\n") - start)).c_str();

    log_i("Set timezone to %s (%s)", tz.c_str(), _tz);

    auto changed = strcmp(_tz, getenv("TZ")) != 0;

    setenv("TZ", _tz, 1);
    tzset();

    if (changed) {
        log_i("Timezone changed, restarting NTP sync");
        esp_sntp_restart();
    }

    return true;
}
