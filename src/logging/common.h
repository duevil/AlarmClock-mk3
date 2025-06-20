#ifndef COMMON_H
#define COMMON_H

#ifndef CORE_DEBUG_LEVEL
#define CORE_DEBUG_LEVEL 1
#endif


namespace logging
{
    enum class Level
    {
        NOTHING,
        FATAL,
        ERROR,
        WARN,
        NOTICE,
        INFO,
        DEBUG,
        TRACE,
        VERBOSE,
        ALWAYS,
    };

    enum Format
    {
        LEVEL_LETTER = 0b01 << 0,
        LEVEL_SHORT = 0b10 << 0,
        LEVEL_FULL = 0b11 << 0,
        TIMESTAMP_SIMPLE = 0b01 << 2,
        TIMESTAMP_SHORT = 0b10 << 2,
        TIMESTAMP_FULL = 0b11 << 2,
        FILE_TRACE = 0b1 << 4,
        FUNCTION_TRACE = 0b1 << 5,
        TASK_TRACE = 0b1 << 6,
    };

    constexpr auto LEVEL_STR_LETTER = "FEWNIDTVA";
    constexpr auto LEVEL_STR_SHORT = "FATERRWARNOTINFDEBTRAVERALW";
    constexpr const char* LEVEL_STR_FULL[] = {
        "FATAL",
        "ERROR",
        "WARN",
        "NOTICE",
        "INFO",
        "DEBUG",
        "TRACE",
        "VERBOSE",
        "ALWAYS",
    };
    constexpr Level CORE_LEVEL_MAPPING[] = {
        Level::NOTHING,
        Level::ERROR,
        Level::WARN,
        Level::INFO,
        Level::DEBUG,
        Level::VERBOSE,
        Level::ALWAYS
    };
    constexpr auto DEFAULT_LEVEL = CORE_LEVEL_MAPPING[CORE_DEBUG_LEVEL];
    constexpr auto DEFAULT_FORMAT = LEVEL_SHORT | TIMESTAMP_FULL | FILE_TRACE | FUNCTION_TRACE | TASK_TRACE;
}


#endif //COMMON_H
