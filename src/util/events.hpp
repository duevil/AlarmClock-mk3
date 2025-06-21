#ifndef EVENTS_HPP
#define EVENTS_HPP

#include <esp_event.h>
#include <memory>
#include <unordered_set>


static constexpr struct
{} FROM_ISR;

namespace events
{
    inline void init()
    {
        if (auto err = esp_event_loop_create_default(); err != ESP_OK && err != ESP_ERR_INVALID_STATE)
        {
            ESP_ERROR_CHECK(err);
        }
    }


    struct Event
    {
        esp_event_base_t base;
        int32_t id;
        void* data;
    };

    using handler_t = void (*)(const Event&);


    struct Proxy
    {
    protected:
        friend struct Base;

        constexpr Proxy(esp_event_base_t base, int32_t id) : m_base(base), m_id(id) {}

        esp_event_base_t m_base;
        int32_t m_id;
    };


    struct PostProxy : Proxy
    {
        using Proxy::Proxy;

        ~PostProxy()
        {
            if (m_isr)
            {
                esp_event_isr_post(m_base, m_id, m_data, m_size, nullptr);
            }
            else
            {
                esp_event_post(m_base, m_id, m_data, m_size, pdMS_TO_TICKS(10));
            }
        }

        auto& operator<<(const decltype(FROM_ISR)&)
        {
            m_isr = true;
            return *this;
        }

        auto& operator<<(const auto& data)
        {
            m_data = &data;
            m_size = sizeof(decltype(data));
            return *this;
        }

    private:
        const void* m_data{};
        size_t m_size{};
        bool m_isr = false;
    };


    struct ListenProxy : Proxy
    {
        using Proxy::Proxy;

        struct Instance
        {
            esp_event_handler_instance_t esp_instance;
            handler_t handler;
        };

        using InstancePtr = std::unique_ptr<Instance>;

        auto& operator>>(handler_t handler) const
        {
            auto& hi = *s_handlerInstances.emplace(std::make_unique<Instance>(nullptr, handler)).first;
            auto handler_arg = const_cast<void*>(static_cast<const void*>(&hi));
            auto err = esp_event_handler_instance_register(m_base, m_id, s_handler, handler_arg, &hi->esp_instance);
            ESP_ERROR_CHECK(err);
            return hi;
        }

        void unregister(const InstancePtr& instance) const
        {
            ESP_ERROR_CHECK(esp_event_handler_instance_unregister(m_base, m_id, instance->esp_instance));
            if (auto it = s_handlerInstances.find(instance); it != s_handlerInstances.end())
            {
                s_handlerInstances.erase(it);
            }
        }

    private:
        inline static std::unordered_set<InstancePtr> s_handlerInstances{};

        static void s_handler(void* handler_arg, esp_event_base_t base, int32_t id, void* data)
        {
            auto& hi = *static_cast<const InstancePtr*>(handler_arg);
            if (auto it = s_handlerInstances.find(hi); it != s_handlerInstances.end())
            {
                it->get()->handler({base, id, data});
            }
        }
    };


    struct Base : ListenProxy
    {
        // ReSharper disable once CppDFAConstantParameter
        constexpr explicit Base(esp_event_base_t base) : ListenProxy(base, ESP_EVENT_ANY_ID) {}

        PostProxy operator<<(int32_t id) const
        {
            return {m_base, id};
        }

        ListenProxy operator>>(int32_t id) const
        {
            return {m_base, id};
        }

        auto& operator>>(handler_t handler) const
        {
            return ListenProxy::operator>>(handler);
        }
    };


    constexpr ListenProxy GLOBAL = Base{nullptr};
}

using EventHandlerPtr = events::ListenProxy::InstancePtr;


#define EVENT_DEFINE(x) constexpr events::Base x{#x}


#endif //EVENTS_HPP
