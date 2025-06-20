#ifndef ENTRY_HPP
#define ENTRY_HPP

#include "common.h"


namespace logging
{
    constexpr auto MAX_MSG_LEN = 228;

    struct Entry
    {
        time_t timestamp;
        Level level;
        const char* file;
        uint32_t line;
        const char* function;
        TaskHandle_t task;
        char message[MAX_MSG_LEN];

        Entry(): timestamp(0), level(), file(nullptr), line(0), function(nullptr), task(nullptr), message() {}

        Entry(Level level, const char* file, uint32_t line, const char* function, const char* format, auto... args)
            : timestamp(time(nullptr)), level(level), file(file),
              line(line), function(function), task(xTaskGetCurrentTaskHandle()),
              message()
        {
            snprintf(message, MAX_MSG_LEN, format, args...);
        }
    };
}


#endif //ENTRY_HPP
