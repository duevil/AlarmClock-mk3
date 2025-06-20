#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "Device.hpp"
#include <memory>
#include <ranges>
#include <algorithm>

#ifndef LOG_QUEUE_LENGTH
#define LOG_QUEUE_LENGTH 32
#endif


namespace logging
{
    void _logger_task_handle(auto*);


    inline struct
    {
        void initialize()
        {
            m_queue = xQueueCreateStatic(LOG_QUEUE_LENGTH, sizeof(Entry), m_queueStack, &m_queueBuf);
            xTaskCreateStaticPinnedToCore(_logger_task_handle,
                                          "logging_task",
                                          std::size(m_taskStack),
                                          nullptr,
                                          0,
                                          m_taskStack,
                                          &m_taskBuf,
                                          APP_CPU_NUM);
        }

        template <typename TDevice> requires std::is_base_of_v<Device, TDevice>
        bool registerDevice(Level level = DEFAULT_LEVEL, int format = DEFAULT_FORMAT,
                            Device** handle = nullptr, auto... args)
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

        void log(const Entry& entry) const
        {
            xQueueSend(m_queue, &entry, 0);
        }

    private:
        friend void _logger_task_handle(auto*);
        std::vector<std::unique_ptr<Device>> m_devices{};
        QueueHandle_t m_queue{};
        StaticQueue_t m_queueBuf{};
        StackType_t m_queueStack[LOG_QUEUE_LENGTH * sizeof(Entry)]{};
        StaticTask_t m_taskBuf{};
        StackType_t m_taskStack[2048]{};
    } Logger;


    [[noreturn]] void _logger_task_handle(auto*)
    {
        auto entry = new Entry{};
        while (true)
        {
            if (auto received = xQueueReceive(Logger.m_queue, entry, portMAX_DELAY); received == pdTRUE)
            {
                for (const auto& device : Logger.m_devices)
                {
                    device->write(*entry);
                }
            }
        }
    }
}


#endif //LOGGER_HPP
