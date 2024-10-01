#include "alarm_clock.h"

// TODO:
//  Currently, the file logging was only tested with limited possibilities inside the WOKWI simulation.
//  To ensure proper functionality, testing with real hardware is required.

static constexpr const size_t MAX_LOG_FILE_SIZE = 1024 * 1024; // 1 MB
static constexpr const uint8_t MAX_FILE_COUNT = 100;
static File log_file;

static void rotate_log_file() {
    char filename[32];
    if (log_file) {
        // We already opened a log file; close it and move to the next
        log_file.close();
        global::logFileIndex = (*global::logFileIndex + 1) % MAX_FILE_COUNT;
    }
    if (!SD.exists("/logs")) {
        SD.mkdir("/logs");
    }
    snprintf(filename, sizeof(filename), "/logs/%02d.log", *global::logFileIndex);
    log_file = SD.open(filename, FILE_WRITE);
    if (log_file && log_file.size() > MAX_LOG_FILE_SIZE) {
        // The newly opened log file is already too large; truncate it by deleting and re-creating it
        log_file.close();
        SD.remove(filename);
        log_file = SD.open(filename, FILE_WRITE);
        log_w("Log file %s is too large; deleted and re-created", filename);
    }
}

/*!
 * @brief Custom vprintf function to replace the default logging output. Writes the log to a file on the SD card,
 *        rotating the file when it reaches a certain size. If debug mode is enabled, also prints the log to the
 *        Serial output.
 * @param format Log message format
 * @param args Log message arguments
 * @return Log message length
 */
int log_vprintf(const char *format, va_list args) {
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
#ifdef DEBUG
    Serial.print(buffer);
#endif
    if (!global::sdAvailable) {
        return -1;
    }
    if (!log_file || log_file.size() > MAX_LOG_FILE_SIZE) {
        rotate_log_file();
        log_i("Log file not opened or current log file reached size limit; rotating");
    }
    if (!log_file) {
        log_e("Failed to open log file");
        return -1;
    }
    log_file.print(buffer);
    log_file.flush();
    return static_cast<int>(strlen(buffer));
}