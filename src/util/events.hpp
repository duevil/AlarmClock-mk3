#ifndef EVENTS_HPP
#define EVENTS_HPP

#include <esp_event.h>
#include <memory>
#include <unordered_set>
#include <functional>


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
        void* data_untyped;

        template <typename T>
        [[nodiscard]] T data() const
        {
            return data_untyped ? *static_cast<const T*>(data_untyped) : T{};
        }

        [[nodiscard]] bool hasData() const
        {
            return data_untyped;
        }
    };

    using handler_t = std::function<void(const Event&)>;


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
                esp_event_isr_post(m_base, m_id, m_data->get(), m_data->size(), nullptr);
            }
            else
            {
                esp_event_post(m_base, m_id, m_data->get(), m_data->size(), pdMS_TO_TICKS(10));
            }
        }

        auto& operator<<(const decltype(FROM_ISR)&)
        {
            m_isr = true;
            return *this;
        }

        template <typename T>
        auto& operator<<(const T& data)
        {
            m_data = std::make_unique<TypedData<T>>(data);
            return *this;
        }

    private:
        struct BaseData
        {
            virtual ~BaseData() = default;
            [[nodiscard]] virtual const void* get() const = 0;
            [[nodiscard]] virtual size_t size() const = 0;
        };

        struct EmptyData final : BaseData
        {
            [[nodiscard]] const void* get() const override { return nullptr; }
            [[nodiscard]] size_t size() const override { return 0; }
        };

        template <typename T>
        struct TypedData final : BaseData
        {
            T value;

            explicit TypedData(const T& v) : value(v) {}
            [[nodiscard]] const void* get() const override { return static_cast<const void*>(&value); }
            [[nodiscard]] size_t size() const override { return sizeof(T); }
        };

        std::unique_ptr<BaseData> m_data = std::make_unique<EmptyData>();
        int m_int = 0;
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

        auto& operator>>(const handler_t& handler) const
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
            if (handler_arg != nullptr)
            {
                auto& hi = *static_cast<const InstancePtr*>(handler_arg);
                if (auto it = s_handlerInstances.find(hi); it != s_handlerInstances.end() && (*it)->handler)
                {
                    (*it)->handler({base, id, data});
                }
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

        auto& operator>>(const handler_t& handler) const
        {
            return ListenProxy::operator>>(handler);
        }
    };


    constexpr ListenProxy GLOBAL = Base{nullptr};
}

using EventHandlerPtr = events::ListenProxy::InstancePtr;
using Event_t = events::Event;


#define EVENT_DEFINE(x) constexpr events::Base x{#x}


#endif //EVENTS_HPP
