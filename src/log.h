#ifndef LOG_H
#define LOG_H

#include "logging/logger.hpp"
#include "logging/serial.hpp"

#define LOG_ENTRY(level, format, ...) logging::Entry{level, __FILE__, __LINE__, __func__, format __VA_OPT__(,) __VA_ARGS__}

#if defined USE_CORE_DEBUG_LEVEL && CORE_DEBUG_LEVEL < 1
#define LOG_F(format, ...) do {] while (0)
#define LOG_E(format, ...) do {] while (0)
#else
#define LOG_F(format, ...) logging::Logger.log(LOG_ENTRY(logging::Level::FATAL, format __VA_OPT__(,) __VA_ARGS__))
#define LOG_E(format, ...) logging::Logger.log(LOG_ENTRY(logging::Level::ERROR, format __VA_OPT__(,) __VA_ARGS__))
#endif

#if defined USE_CORE_DEBUG_LEVEL && CORE_DEBUG_LEVEL < 2
#define LOG_W(format, ...) do {] while (0)
#define LOG_N(format, ...) do {] while (0)
#else
#define LOG_W(format, ...) logging::Logger.log(LOG_ENTRY(logging::Level::WARN, format __VA_OPT__(,) __VA_ARGS__))
#define LOG_N(format, ...) logging::Logger.log(LOG_ENTRY(logging::Level::NOTICE, format __VA_OPT__(,) __VA_ARGS__))
#endif

#if defined USE_CORE_DEBUG_LEVEL && CORE_DEBUG_LEVEL < 3
#define LOG_I(format, ...) do {] while (0)
#else
#define LOG_I(format, ...) logging::Logger.log(LOG_ENTRY(logging::Level::INFO, format __VA_OPT__(,) __VA_ARGS__))
#endif

#if defined USE_CORE_DEBUG_LEVEL && CORE_DEBUG_LEVEL < 4
#define LOG_D(format, ...) do {] while (0)
#else
#define LOG_D(format, ...) logging::Logger.log(LOG_ENTRY(logging::Level::DEBUG, format __VA_OPT__(,) __VA_ARGS__))
#endif

#if defined USE_CORE_DEBUG_LEVEL && CORE_DEBUG_LEVEL < 5
#define LOG_T(format, ...) do {] while (0)
#define LOG_V(format, ...) do {] while (0)
#else
#define LOG_T(format, ...) logging::Logger.log(LOG_ENTRY(logging::Level::TRACE, format __VA_OPT__(,) __VA_ARGS__))
#define LOG_V(format, ...) logging::Logger.log(LOG_ENTRY(logging::Level::VERBOSE, format __VA_OPT__(,) __VA_ARGS__))
#endif

#define LOG_A(format, ...) logging::Logger.log(LOG_ENTRY(logging::Level::ALWAYS, format __VA_OPT__(,) __VA_ARGS__))
#define LOG(format, ...) LOG_A(format __VA_OPT__(,) __VA_ARGS__)

#endif //LOG_H
