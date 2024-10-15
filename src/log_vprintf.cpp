#include "alarm_clock.h"

static inline constexpr const size_t MAX_LOG_FILE_SIZE = 10 * 1024 * 1024; // 10 MiB
static inline constexpr const uint8_t MAX_FILE_COUNT = 100;
static inline constexpr const size_t LOG_MSG_SIZE = 256;
static NVSValue<uint8_t> logFileIndex{"log_file_index", 0};
static File log_file;
static QueueHandle_t logQueue;
static StaticTask_t logTaskHandle;
static StackType_t logTaskStack[8192];

static void rotate_log_file() {
    char filename[32];
    if (log_file) {
        // We already opened a log file; close it and move to the next
        log_file.close();
        logFileIndex = (*logFileIndex + 1) % MAX_FILE_COUNT;
    }
    if (!SD.exists("/logs")) {
        SD.mkdir("/logs");
    }
    snprintf(filename, sizeof(filename), "/logs/%02d.log", *logFileIndex);
    log_file = SD.open(filename, FILE_APPEND);
    if (log_file && log_file.size() > MAX_LOG_FILE_SIZE) {
        // The newly opened log file is already too large; truncate it
        log_file.close();
        log_file = SD.open(filename, FILE_WRITE);
        log_w("Log file %s is too large; deleted and re-created", filename);
    }
}

static void __attribute((noreturn)) print_log(void *) {
    char buffer[LOG_MSG_SIZE];
    // infinite loop while waiting for log messages to be received
    while (true) {
        auto xHigherPriorityTaskWoken = pdFALSE;
        // wait for a log message to be received
        // using ISR version to allow log messages from ISR context
        auto received = xQueueReceiveFromISR(logQueue, buffer, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            // if the task was woken, yield to it
            portYIELD_FROM_ISR();
        }
        if (!received) {
            // no log message received; wait for the next one
            continue;
        }
        // a new log message was received; print it
#ifdef DEBUG
#ifdef DEBUG_SERIAL_COLORS
        char level;
        if (buffer[0] >= '0' && buffer[0] <= '9') {
            // buffer is of custom format, extract the log level after timestamp
            level = buffer[24];
        } else {
            // buffer is of default format, log level is the first character
            level = buffer[0];
        }
        Serial.print("\033["); // color sequence start
        switch (level) {
            case 'V':
                Serial.print("0;90"); // grey
                break;
            case 'D':
                Serial.print("0;36"); // cyan
                break;
            case 'I':
                Serial.print("0;32"); // green
                break;
            case 'W':
                Serial.print("1;33"); // yellow
                break;
            case 'E':
                Serial.print("1;31"); // red
                break;
            default:
                Serial.print("37"); // white
                break;
        }
        Serial.print("m"); // color sequence end
#endif
        Serial.print(buffer);
#ifdef DEBUG_SERIAL_COLORS
        Serial.print("\033[0m"); // reset color
#endif
#endif
        if (!global::sdAvailable) {
            continue;
        }
        if (!log_file || log_file.size() > MAX_LOG_FILE_SIZE) {
            rotate_log_file();
            log_w("Log file not opened or current log file reached size limit; rotated to %02d.log",
                  *logFileIndex);
        }
        if (!log_file) {
            log_e("Failed to open log file");
            continue;
        }
        log_file.print(buffer);
        log_file.flush();
    }
}

/*!
 * @brief Custom vprintf function to replace the default logging output. Uses a queue and a task pinned to core 1
 * (APP_CPU) to print log messages asynchronously and avoid blocking the main loop or tasks running on core 0 (PRO_CPU).
 * The log messages are written to a file on the SD card if available, rotating the log file when it reaches a
 * specified size limit. The log messages are also printed to the Serial port if DEBUG is defined.
 * @param format Log message format
 * @param args Log message arguments
 * @return Log message length
 */
int log_vprintf(const char *format, va_list args) {
    auto xHigherPriorityTaskWoken = pdFALSE;
    char buffer[LOG_MSG_SIZE];
    if (static bool initialized = false; !initialized) {
        // if log queue and task are not yet created, create them
        // the queue is created using PSRAM memory allocation to not use up the main heap
        logQueue = xQueueCreateWithCaps(30, sizeof(buffer), MALLOC_CAP_SPIRAM);
        // create the log task pinned to core 1 (APP_CPU) and statically to avoid heap allocation
        xTaskCreateStaticPinnedToCore(print_log,
                                      "print_log",
                                      sizeof(logTaskStack) / sizeof(logTaskStack[0]),
                                      nullptr,
                                      1,
                                      logTaskStack,
                                      &logTaskHandle,
                                      1);
        initialized = true;
    }
    vsnprintf(buffer, sizeof(buffer), format, args);
    // send the log message to the queue to be printed
    // using ISR version to allow log messages from ISR context
    xQueueSendFromISR(logQueue, buffer, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        // if the task was woken, yield to it
        portYIELD_FROM_ISR();
    }
    return static_cast<int>(strlen(buffer));
}