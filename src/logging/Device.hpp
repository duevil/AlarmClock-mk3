#ifndef DEVICE_HPP
#define DEVICE_HPP

#include "Entry.hpp"


namespace logging
{
    struct Device : Print
    {
        ~Device() override = default;
        virtual bool initialize() = 0;

        Device(Level level, int format) : m_level(level), m_format(format) {}
        void setLevel(Level level) { this->m_level = level; }
        void setFormat(Format format) { this->m_format = format; }

        void write(const Entry& entry)
        {
            if (m_level < entry.level)
            {
                return;
            }

            writeStart(entry);

            if ((m_format & TIMESTAMP_FULL) == TIMESTAMP_FULL)
            {
                // print timestamp as YYYY-MM-DD HH:MM:SS
                auto tm = localtime(&entry.timestamp);
                printf("%04u-%02u-%02u %02u:%02u:%02u ",
                       tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                       tm->tm_hour, tm->tm_min, tm->tm_sec);
            }
            else if (m_format & TIMESTAMP_SIMPLE)
            {
                // print timestamp in seconds
                printf("%lli ", entry.timestamp);
            }
            else if (m_format & TIMESTAMP_SHORT)
            {
                // print timestamp as HH:MM:SS
                auto tm = localtime(&entry.timestamp);
                printf("%02u:%02u:%02u ", tm->tm_hour, tm->tm_min, tm->tm_sec);
            }

            auto level = static_cast<std::underlying_type_t<Level>>(entry.level);
            if ((m_format & LEVEL_FULL) == LEVEL_FULL)
            {
                printf("[%s] ", LEVEL_STR_FULL[level]);
            }
            else if (m_format & LEVEL_LETTER)
            {
                printf("[%c] ", LEVEL_STR_LETTER[level]);
            }
            else if (m_format & LEVEL_SHORT)
            {
                printf("[%-.*s] ", 3, &LEVEL_STR_SHORT[level * 3]);
            }

            if (m_format & FILE_TRACE && m_format & FUNCTION_TRACE)
            {
                printf("[%s:%u %s] ", entry.file, entry.line, entry.function);
            }
            else if (m_format & FILE_TRACE)
            {
                printf("[%s:%u] ", entry.file, entry.line);
            }
            else if (m_format & FUNCTION_TRACE)
            {
                printf("[%s] ", entry.function);
            }

            if (m_format & TASK_TRACE && entry.task != nullptr)
            {
                auto name = pcTaskGetName(entry.task);
                printf("[task: %s] ", name ? name : "(null)");
            }

            println(entry.message);
            flush();

            writeEnd(entry);
        }

    protected:
        virtual void writeStart(const Entry&) {}
        virtual void writeEnd(const Entry&) {}

    private:
        Level m_level;
        int m_format;
    };
}


#endif //DEVICE_HPP
