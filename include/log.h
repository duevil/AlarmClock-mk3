#ifndef LOG_H
#define LOG_H

static const char *log_timestamp() {
    static char timestamp[25];
    struct timeval tv{};
    gettimeofday(&tv, nullptr);
    struct tm tm{};
    localtime_r(&tv.tv_sec, &tm);
    snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02dT%02d:%02d:%02d.%03ld",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec / 1000);
    return timestamp;
}

#define CUSTOM_LOG_FORMAT(letter, format) "%s " #letter \
    " [%s:%u] %s(): " format "\r\n", \
    log_timestamp(), pathToFileName(__FILE__), __LINE__, __FUNCTION__

#if CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_VERBOSE
#undef log_v
#define log_v(format, ...) esp_log_write(ESP_LOG_VERBOSE, ARDUHAL_ESP_LOG_TAG, CUSTOM_LOG_FORMAT(V, format) __VA_OPT__(,) __VA_ARGS__)
#endif

#if CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
#undef log_d
#define log_d(format, ...) esp_log_write(ESP_LOG_DEBUG, ARDUHAL_ESP_LOG_TAG, CUSTOM_LOG_FORMAT(D, format) __VA_OPT__(,) __VA_ARGS__)
#endif

#if CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
#undef log_i
#define log_i(format, ...) esp_log_write(ESP_LOG_INFO, ARDUHAL_ESP_LOG_TAG, CUSTOM_LOG_FORMAT(I, format) __VA_OPT__(,) __VA_ARGS__)
#endif

#if CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_WARN
#undef log_w
#define log_w(format, ...) esp_log_write(ESP_LOG_WARN, ARDUHAL_ESP_LOG_TAG, CUSTOM_LOG_FORMAT(W, format) __VA_OPT__(,) __VA_ARGS__)
#endif

#if CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_ERROR
#undef log_e
#define log_e(format, ...) esp_log_write(ESP_LOG_ERROR, ARDUHAL_ESP_LOG_TAG, CUSTOM_LOG_FORMAT(E, format) __VA_OPT__(,) __VA_ARGS__)
#endif

#if defined RELEASE
#define log_setup(vprintf_func) do { \
    esp_log_level_set(ARDUHAL_ESP_LOG_TAG, ESP_LOG_INFO); \
    esp_log_set_vprintf(vprintf_func); \
} while (0)
#elif defined DEBUG
#define log_setup(vprintf_func) do { \
    Serial.begin(SERIAL_BAUD_RATE); \
    esp_log_level_set(ARDUHAL_ESP_LOG_TAG, (esp_log_level_t) CORE_DEBUG_LEVEL); \
    esp_log_set_vprintf(vprintf_func); \
} while (0)
#else
#define log_setup() do {} while (0)
#endif


#endif //LOG_H
