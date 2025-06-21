#ifndef EVENTS_HPP
#define EVENTS_HPP

#include <esp_event.h>
#include <memory>
#include <unordered_set>


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


    struct HandlerInstance
    {
        esp_event_handler_instance_t esp_instance;
        void(*handler)(const Event&);
    };

    using HandlerInstancePtr = std::unique_ptr<HandlerInstance>;


    inline struct
    {
        auto& create(auto handler)
        {
            return *m_handlers.emplace(std::make_unique<HandlerInstance>(nullptr, handler)).first;
        }

        void destroy(const HandlerInstancePtr& handler_ptr)
        {
            m_handlers.erase(handler_ptr);
        }

        void operator()(const HandlerInstancePtr& instance, const Event& event) const
        {
            if (auto it = m_handlers.find(instance); it != m_handlers.end())
            {
                it->get()->handler(event);
            }
        }

    private:
        std::unordered_set<std::unique_ptr<HandlerInstance>> m_handlers{};
    } _handler_instances{};


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
            esp_event_post(m_base, m_id, m_data, m_size, 0);
        }

        void operator<<(const auto& data)
        {
            m_data = &data;
            m_size = sizeof(decltype(data));
        }

    private:
        const void* m_data{};
        size_t m_size{};
    };


    struct ListenProxy : Proxy
    {
        using Proxy::Proxy;

        auto& operator>>(auto handler) const
        {
            auto& handler_instance = _handler_instances.create(handler);
            auto err = esp_event_handler_instance_register(
                m_base, m_id,
                [](auto* handler_arg, esp_event_base_t base, int32_t id, void* data)
                {
                    auto& instance = *static_cast<const HandlerInstancePtr*>(handler_arg);
                    _handler_instances(instance, {base, id, data});
                },
                const_cast<void*>(static_cast<const void*>(&handler_instance)), &handler_instance->esp_instance
            );
            ESP_ERROR_CHECK(err);
            return handler_instance;
        }

        void unregister(const HandlerInstancePtr& handler_ptr) const
        {
            ESP_ERROR_CHECK(esp_event_handler_instance_unregister(m_base, m_id, handler_ptr->esp_instance));
            _handler_instances.destroy(handler_ptr);
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

        template <typename T>
        std::conditional_t<std::is_integral_v<T>, ListenProxy, const HandlerInstancePtr&>
        operator>>(const T& param) const
        {
            if constexpr (std::is_integral_v<T>)
            {
                return {m_base, param};
            }
            else
            {
                return ListenProxy::operator>>(param);
            }
        }
    };


    constexpr ListenProxy GLOBAL = Base{nullptr};
}


#define EVENT_DEFINE(x) constexpr events::Base x{#x}


#endif //EVENTS_HPP
