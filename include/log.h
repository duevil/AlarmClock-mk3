#ifndef LOG_H
#define LOG_H

#define CUSTOM_LOG_FORMAT(letter, format) ARDUHAL_LOG_COLOR_ ## letter "%s " #letter \
    " [%s:%u] %s(): " format ARDUHAL_LOG_RESET_COLOR "\r\n", \
    esp_log_system_timestamp(), pathToFileName(__FILE__), __LINE__, __FUNCTION__

#if CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_VERBOSE
#undef log_v
#define log_v(format, ...) esp_log_write(ESP_LOG_VERBOSE, TAG, CUSTOM_LOG_FORMAT(V, format), ##__VA_ARGS__)
#endif

#if CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
#undef log_d
#define log_d(format, ...) esp_log_write(ESP_LOG_DEBUG, TAG, CUSTOM_LOG_FORMAT(D, format), ##__VA_ARGS__)
#endif

#if CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
#undef log_i
#define log_i(format, ...) esp_log_write(ESP_LOG_INFO, TAG, CUSTOM_LOG_FORMAT(I, format), ##__VA_ARGS__)
#endif

#if CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_WARN
#undef log_w
#define log_w(format, ...) esp_log_write(ESP_LOG_WARN, TAG, CUSTOM_LOG_FORMAT(W, format), ##__VA_ARGS__)
#endif

#if CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_ERROR
#undef log_e
#define log_e(format, ...) esp_log_write(ESP_LOG_ERROR, TAG, CUSTOM_LOG_FORMAT(E, format), ##__VA_ARGS__)
#endif

#if defined RELEASE
#define log_setup() do { esp_log_level_set(TAG, ESP_LOG_INFO); } while (0)
#elif defined DEBUG
#define log_setup() do { \
    esp_log_level_set(TAG, (esp_log_level_t) CORE_DEBUG_LEVEL); \
    Serial.begin(SERIAL_BAUD_RATE); \
} while (0)
#else
#define log_setup() do {} while (0)
#endif


#endif //LOG_H
