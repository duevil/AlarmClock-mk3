#ifndef DEVICE_HPP
#define DEVICE_HPP

#include "entry.hpp"


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

            auto level = static_cast<std::underlying_type_t<Level>>(entry.level) - 1;
            if ((m_format & LEVEL_FULL) == LEVEL_FULL)
            {
                *this << '[' << LEVEL_STR_FULL[level] << "] ";
            }
            else if (m_format & LEVEL_LETTER)
            {
                *this << '[' << LEVEL_STR_LETTER[level] << "] ";
            }
            else if (m_format & LEVEL_SHORT)
            {
                *this << '[' << LEVEL_STR_SHORT[level] << "] ";
            }

            if ((m_format & TIMESTAMP_FULL) == TIMESTAMP_FULL)
            {
                // print timestamp as YYYY-MM-DD HH:MM:SS.sss
                tm tm{};
                gmtime_r(&entry.timestamp.tv_sec, &tm);
                char buf[32]{};
                char* tmp;
                asprintf(&tmp, "%%F %%T.%03lld ", entry.timestamp.tv_usec / 1000LL);
                strftime(buf, sizeof(buf), tmp, &tm);
                free(tmp);
                *this << buf;
            }
            else if (m_format & TIMESTAMP_SIMPLE)
            {
                // print timestamp in [seconds].[milliseconds]
                printf("%010lld.%03lld ", entry.timestamp.tv_sec, entry.timestamp.tv_usec / 1000LL);
            }
            else if (m_format & TIMESTAMP_SHORT)
            {
                // print timestamp as HH:MM:SS
                tm tm{};
                gmtime_r(&entry.timestamp.tv_sec, &tm);
                char buf[10]{};
                strftime(buf, sizeof(buf), "%T ", &tm);
                *this << buf;
            }

            if (m_format & FILE_TRACE || m_format & FUNCTION_TRACE)
            {
                *this << '[';
                if (m_format & FILE_TRACE)
                {
                    *this << entry.file << ':' << entry.line;
                }
                if (m_format & FILE_TRACE && m_format & FUNCTION_TRACE)
                {
                    *this << ' ';
                }
                if (m_format & FUNCTION_TRACE)
                {
                    *this << entry.function;
                }
                *this << "] ";
            }

            if (m_format & TASK_TRACE)
            {
                auto name = pcTaskGetName(entry.task);
                *this << "[task: " << (name ? name : "<null>") << "] ";
            }

            if (m_format)
            {
                *this << "- ";
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

        Device& operator<<(auto x)
        {
            print(x);
            return *this;
        }
    };
}


#endif //DEVICE_HPP
