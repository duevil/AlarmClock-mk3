#ifndef NVS_VALUE_HPP
#define NVS_VALUE_HPP

#include "alarm_clock.h"

/*!
 * @brief Abstract class for storing values in the non-volatile storage (NVS) using the Preferences API.
 * Provides a static method to load all the values from the NVS.
 */
class NVS {
    inline static std::unordered_set<NVS *> instances{};

protected:
    inline static Preferences prefs{};
    virtual void load() = 0;

public:
    /*!
     * @brief Constructor. Stores a pointer to the instance for static access
     */
    NVS() { instances.insert(this); }

    /*!
     * @brief Destructor. Removes the pointer to the instance
     */
    ~NVS() { instances.erase(this); }

    /*!
     * @brief Loads all the values from the NVS
     */
    static void setup() {
        prefs.begin("alarm_clock", false);
        for (auto &instance: instances) {
            if (!instance) {
                log_e("Found dangling pointer in the NVS instances");
                continue;
            }
            instance->load();
        }
    }
};

//! Template variable that is true if the type is supported by the Preferences API.
template<typename T> requires std::is_same_v<T, int8_t> || std::is_same_v<T, uint8_t> ||
                              std::is_same_v<T, int16_t> || std::is_same_v<T, uint16_t> ||
                              std::is_same_v<T, int32_t> || std::is_same_v<T, uint32_t> ||
                              std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t> ||
                              std::is_same_v<T, float> ||
                              std::is_same_v<T, double> ||
                              std::is_same_v<T, bool> ||
                              std::is_same_v<T, String>
constexpr bool is_nvs_type_v = true;

/*!
 * @brief Class for storing a value in the non-volatile storage (NVS) using the Preferences API
 * @tparam T The type of the value to be stored
 * @note Values will not be stored automatically. To store a value to the NVS, either assign a new value to the object
 * using the assignment operator or call the store method
 */
template<typename T> requires is_nvs_type_v<T>
class NVSValue : NVS {
    T value{};
    const char *name;

    inline T get();
    inline void put(const T &val);

public:
    /*!
     * @brief Constructor
     * @param name The name of the value to be stored in the NVS. Must be 15 characters or less
     * @param value The default value to be stored in the NVS
     */
    explicit NVSValue(const char *name, T value = {}) : NVS(), name(name), value(value) {
        assert(strlen(name) <= 15 && "The name must be 15 characters or less");
    }

    /*!
     * @brief Loads the value from the preferences if or, if the value is not present, creates a new entry
     */
    void load() override {
        if (prefs.isKey(name)) {
            log_d("Loading value for %s", name);
            value = get();
        } else {
            log_i("No value for %s found, creating a new entry with the default value", name);
            put(value);
        }
    }

    /*!
     * @brief Stores the value in the preferences only if it has changed
     */
    void store() {
        if (value == get()) {
            log_d("Value for %s is the same, not storing", name);
            return;
        }
        put(value);
    }

    /*!
     * @brief Sets and stores the value
     * @param val The value to be stored
     * @return A reference to this object
     */
    NVSValue &operator=(T val) {
        value = val;
        store();
        return *this;
    }

    /*!
     * @brief Gets a reference to the stored value
     */
    inline T &operator*() { return value; }

    /*!
     * @brief Gets a pointer to the stored value
     */
    inline T *operator->() { return &value; }

    /*!
     * @brief Explicit conversion to the stored value type
     * @return The stored value
     */
    inline explicit operator T() const { return value; }
};

template<typename T> requires is_nvs_type_v<T>
T NVSValue<T>::get() {
    if /**/ constexpr (std::is_same_v<T, int8_t>) return prefs.getChar(name);
    else if constexpr (std::is_same_v<T, uint8_t>) return prefs.getUChar(name);
    else if constexpr (std::is_same_v<T, int16_t>) return prefs.getShort(name);
    else if constexpr (std::is_same_v<T, uint16_t>) return prefs.getUShort(name);
    else if constexpr (std::is_same_v<T, int32_t>) return prefs.getInt(name);
    else if constexpr (std::is_same_v<T, uint32_t>) return prefs.getUInt(name);
    else if constexpr (std::is_same_v<T, int64_t>) return prefs.getLong(name);
    else if constexpr (std::is_same_v<T, uint64_t>) return prefs.getULong(name);
    else if constexpr (std::is_same_v<T, float>) return prefs.getFloat(name);
    else if constexpr (std::is_same_v<T, double>) return prefs.getDouble(name);
    else if constexpr (std::is_same_v<T, bool>) return prefs.getBool(name);
    else if constexpr (std::is_same_v<T, String>) return prefs.getString(name);
}

template<typename T> requires is_nvs_type_v<T>
void NVSValue<T>::put(const T &val) {
    if /**/ constexpr (std::is_same_v<T, int8_t>) prefs.putChar(name, val);
    else if constexpr (std::is_same_v<T, uint8_t>) prefs.putUChar(name, val);
    else if constexpr (std::is_same_v<T, int16_t>) prefs.putShort(name, val);
    else if constexpr (std::is_same_v<T, uint16_t>) prefs.putUShort(name, val);
    else if constexpr (std::is_same_v<T, int32_t>) prefs.putInt(name, val);
    else if constexpr (std::is_same_v<T, uint32_t>) prefs.putUInt(name, val);
    else if constexpr (std::is_same_v<T, int64_t>) prefs.putLong(name, val);
    else if constexpr (std::is_same_v<T, uint64_t>) prefs.putULong(name, val);
    else if constexpr (std::is_same_v<T, float>) prefs.putFloat(name, val);
    else if constexpr (std::is_same_v<T, double>) prefs.putDouble(name, val);
    else if constexpr (std::is_same_v<T, bool>) prefs.putBool(name, val);
    else if constexpr (std::is_same_v<T, String>) prefs.putString(name, val);
}

#endif //NVS_VALUE_HPP
