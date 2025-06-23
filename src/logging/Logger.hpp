#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "Device.hpp"
#include "util/BlockingQueue.hpp"
#include <util/Thread.hpp>
#include <memory>
#include <ranges>
#include <algorithm>

#ifndef LOG_QUEUE_LENGTH
#define LOG_QUEUE_LENGTH 32
#endif


namespace logging
{
    inline struct Logger final : Thread<>
    {
        Logger() : Thread({.name = "logging", .coreId = PRO_CPU_NUM}) {}

        template <typename TDevice> requires std::is_base_of_v<Device, TDevice>
        bool registerDevice(Level level = DEFAULT_LEVEL, int format = DEFAULT_FORMAT,
                            Device** handle = nullptr, auto&&... args)
        {
            auto device = std::make_unique<TDevice>(level, format, std::forward<decltype(args)>(args)...);
            if (device != nullptr && device->initialize())
            {
                m_devices.push_back(std::move(device));
                if (handle != nullptr)
                {
                    *handle = m_devices.back().get();
                }
                return true;
            }
            return false;
        }

        void unregisterDevice(Device* device)
        {
            if (device != nullptr)
            {
                std::erase_if(m_devices, [device](const auto& ptr) { return ptr.get() == device; });
            }
        }

        void log(const Entry& entry)
        {
            m_queue.offer(entry);
        }

    private:
        std::vector<std::unique_ptr<Device>> m_devices{};
        ESPQueue<LOG_QUEUE_LENGTH, Entry> m_queue{};
        Entry m_entry_buf{};

        void run() override
        {
            m_queue.take(m_entry_buf);
            for (const auto& device : m_devices)
            {
                device->write(m_entry_buf);
            }
        }
    } Logger;
}


#endif //LOGGER_HPP
