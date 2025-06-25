#ifndef NVS_VALUE_HPP
#define NVS_VALUE_HPP

#include <Preferences.h>
#include "log.h"
#include <unordered_set>


/*!
 * @brief Abstract class for storing values in the non-volatile storage (NVS) using the Preferences API
 *
 * Provides a static method to load all the values from the NVS
 */
struct NVS
{
    /*!
     * @brief Stores a pointer to the instance for static access
     */
    NVS() { s_instances.insert(this); }

    /*!
     * @brief Destructor. Removes the pointer to the instance
     */
    virtual ~NVS() { s_instances.erase(this); }

    /*!
     * @brief Loads all the values from the NVS
     * @param name The namespace for the NVS
     */
    static void begin(const char* name)
    {
        s_prefs.begin(name, false);
        for (auto& instance : s_instances)
        {
            assert(instance && "Found invalid NVS instance");
            if (instance)
                instance->load();
        }
    }

protected:
    inline static Preferences s_prefs{};
    virtual void load() = 0;

private:
    inline static std::unordered_set<NVS*> s_instances{};
};


//! Template variable that is true if the type is supported by the Preferences API
template <typename T> requires std::is_same_v<T, int8_t> || std::is_same_v<T, uint8_t> ||
    std::is_same_v<T, int16_t> || std::is_same_v<T, uint16_t> ||
    std::is_same_v<T, int32_t> || std::is_same_v<T, uint32_t> ||
    std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t> ||
    std::is_same_v<T, float> ||
    std::is_same_v<T, double> ||
    std::is_same_v<T, bool> ||
    std::is_same_v<T, String>
constexpr bool is_nvs_type_v = true;


/*!
 * @brief Class for storing a non-volatile value (NVV) in the non-volatile storage (NVS) using the Preferences API with
 * the possibility to attach observers to be called when the value is changed
 * @tparam T The type of the value to be stored
 * @note Changing the value of the NVV using the assignment operator automatically syncs the value with the NVS.
 * When changing the value using operations on a reference of a NVV,
 * it is necessary to manually call <code>sync()</code>.
 * Observers will only be called on sync, but not on load.
 */
template <typename T> requires is_nvs_type_v<T>
struct NVV final : private NVS
{
    using Observer = std::function<void(const T&)>;

    /*!
     * @brief Constructor
     * @param name The name of the value to be stored in the NVS; must be 15 characters or fewer
     * @param value The default value to be stored in the NVS
     */
    explicit NVV(const char* name, T value = {}) : m_value(value), m_name(name)
    {
        assert(strlen(name) <= 15 && "The name must be 15 characters or less");
    }

    /*!
     * @brief Loads the value from the NVS or, if the value is not present, creates a new entry
     */
    void load() override
    {
        if (s_prefs.isKey(m_name))
        {
            m_value = get();
            LOG_D("Loaded value for %s: %s", m_name, String(m_value).c_str());
        }
        else
        {
            LOG_N("No value for %s found, creating a new entry with the default value", m_name);
            put();
        }
    }

    /*!
     * @brief Stores the value in the NVS only if it has changed
     */
    void sync()
    {
        if (m_value == get())
        {
            LOG_D("Value for %s is the same, no storing done", m_name);
            return;
        }
        LOG_D("Putting value for %s: %s", m_name, String(m_value).c_str());
        put();

        for (const auto& observer : m_observers)
        {
            observer(m_value);
        }
    }

    /*!
     * @brief Sets and stores the value
     * @param val The value to be stored
     * @return A reference to this object
     */
    NVV& operator=(const T& val)
    {
        m_value = val;
        sync();
        return *this;
    }

    /*!
     * @brief Gets a reference to the stored value
     */
    T& operator*() { return m_value; }

    /*!
     * @brief Gets a const reference to the stored value
     */
    const T& operator*() const { return m_value; }

    /*!
     * @brief Gets a pointer to the stored value
     */
    T* operator->() { return &m_value; }

    /*!
     * @brief Explicit conversion to the stored value type
     * @return The stored value
     */
    explicit operator T() const { return m_value; }

    /**
     * @brief Attach an observer to this value to be called when the value changes
     * @param observer The observer to attach to this NVV
     */
    void observe(const auto& observer)
    {
        m_observers.emplace_back(observer);
    }

    /**
     * @brief Remove an observer from the observer list of this value
     * @param observer The observer to remove from this NVV
     */
    void observerRemove(const auto& observer)
    {
        std::erase(m_observers, observer);
    }

    // delete copy constructor and assignment operator

    NVV(const NVV&) = delete;
    NVV& operator=(const NVV&) = delete;

private:
    T m_value{};
    const char* m_name;
    std::vector<Observer> m_observers{};

    T get()
    {
        if constexpr (std::is_same_v<T, int8_t>) return s_prefs.getChar(m_name);
        if constexpr (std::is_same_v<T, uint8_t>) return s_prefs.getUChar(m_name);
        if constexpr (std::is_same_v<T, int16_t>) return s_prefs.getShort(m_name);
        if constexpr (std::is_same_v<T, uint16_t>) return s_prefs.getUShort(m_name);
        if constexpr (std::is_same_v<T, int32_t>) return s_prefs.getInt(m_name);
        if constexpr (std::is_same_v<T, uint32_t>) return s_prefs.getUInt(m_name);
        if constexpr (std::is_same_v<T, int64_t>) return s_prefs.getLong(m_name);
        if constexpr (std::is_same_v<T, uint64_t>) return s_prefs.getULong(m_name);
        if constexpr (std::is_same_v<T, float>) return s_prefs.getFloat(m_name);
        if constexpr (std::is_same_v<T, double>) return s_prefs.getDouble(m_name);
        if constexpr (std::is_same_v<T, bool>) return s_prefs.getBool(m_name);
        if constexpr (std::is_same_v<T, String>) return s_prefs.getString(m_name);
        return T{};
    }

    void put()
    {
        if /**/ constexpr (std::is_same_v<T, int8_t>) s_prefs.putChar(m_name, m_value);
        else if constexpr (std::is_same_v<T, uint8_t>) s_prefs.putUChar(m_name, m_value);
        else if constexpr (std::is_same_v<T, int16_t>) s_prefs.putShort(m_name, m_value);
        else if constexpr (std::is_same_v<T, uint16_t>) s_prefs.putUShort(m_name, m_value);
        else if constexpr (std::is_same_v<T, int32_t>) s_prefs.putInt(m_name, m_value);
        else if constexpr (std::is_same_v<T, uint32_t>) s_prefs.putUInt(m_name, m_value);
        else if constexpr (std::is_same_v<T, int64_t>) s_prefs.putLong(m_name, m_value);
        else if constexpr (std::is_same_v<T, uint64_t>) s_prefs.putULong(m_name, m_value);
        else if constexpr (std::is_same_v<T, float>) s_prefs.putFloat(m_name, m_value);
        else if constexpr (std::is_same_v<T, double>) s_prefs.putDouble(m_name, m_value);
        else if constexpr (std::is_same_v<T, bool>) s_prefs.putBool(m_name, m_value);
        else if constexpr (std::is_same_v<T, String>) s_prefs.putString(m_name, m_value);
    }
};


#endif //NVS_VALUE_HPP
