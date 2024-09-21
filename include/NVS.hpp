#ifndef NVS_HPP
#define NVS_HPP

#include <Preferences.h>
#include <vector>

/*!
 * @brief A class that handles the Preferences API for a set of predefined variables, allowing for easy bulk loading
 * and storing. To add variables to the handler, simply pass them as a list of NVS::Var objects (variable reference and
 * preferences key) to the constructor. Only variables whose types are supported by the Preferences API can be added
 * (uint8_t, char, uint16_t, int16_t, uint32_t, int32_t, uint64_t, int64_t, float, double, bool, String).
 */
class NVS {
public:
    /*!
     * @brief A generic class wrapping the Preferences functions around an existing variable.
     * @note The class works by storing two lambda functions that take a Preferences object and either store or load the
     * variable. The lambda functions are created by the constructor of the class, which takes a reference to the
     * variable and a key to store it under. The lambda functions are then called by the Handler class to store or load
     * the variable. The use of lambda functions allows the class to work with any type supported by the Preferences API
     * without having to write separate functions for each type.
     */
    class Var {
        friend class NVS; // No need for public access to the lambda functions - they are only needed by the Handler.
        const std::function<void(Preferences &p)> _store;
        const std::function<void(Preferences &p)> _load;
    public:
        /*!
         * @brief Constructor that takes a reference to a variable and a key to store it under.
         * @tparam T The type of the variable. Must be one of the types (uint8_t, char, uint16_t, int16_t,
         * uint32_t, int32_t, uint64_t, int64_t, float, double, bool, String) supported by the Preferences API.
         * @param name The key to store the variable under.
         * @param var A reference to the variable.
         */
        template<typename T, typename std::enable_if<std::is_same<T, uint8_t>::value ||
                                                     std::is_same<T, char>::value ||
                                                     std::is_same<T, uint16_t>::value ||
                                                     std::is_same<T, int16_t>::value ||
                                                     std::is_same<T, uint32_t>::value ||
                                                     std::is_same<T, int32_t>::value ||
                                                     std::is_same<T, uint64_t>::value ||
                                                     std::is_same<T, int64_t>::value ||
                                                     std::is_same<T, float>::value ||
                                                     std::is_same<T, double>::value ||
                                                     std::is_same<T, bool>::value ||
                                                     std::is_same<T, String>::value>::type...>
        Var(const char *name, T &var):
                _store([name, &var](Preferences &p) { put(p, name, var); }),
                _load([name, &var](Preferences &p) { var = get<T>(p, name); }) {}
    };

    /*!
     * @brief Constructor that takes a list of NVS::Var objects.
     * @param vars A list of NVS::Var objects.
     */
    NVS(std::initializer_list<Var> vars) : _vars(vars) {}

    /*!
     * @brief Initializes the Preferences API and loads the stored values into the variables.
     */
    void setup() {
        _prefs.begin(NAMESPACE);
        for (const Var &var: _vars) {
            var._load(_prefs);
        }
    }

    /*!
     * @brief Stores the values of all the variables into the Preferences API.
     * @note Only variables whose values have changed will be stored, making this function safe to call frequently.
     */
    void store_all() {
        for (const Var &var: _vars) {
            var._store(_prefs);
        }
    }

private:
    static constexpr const char *NAMESPACE = "alarm_clock";
    Preferences _prefs;
    const std::vector<Var> _vars;
    template<typename T>
    static T get(Preferences &p, const char *name);
    template<typename T>
    static void put(Preferences &p, const char *name, const T &value);
};

template<typename T>
T NVS::get(Preferences &p, const char *name) {
    // No need to check if the key exists, as the Preferences API will return
    // a matching default value for each type if the key does not exist.
    if /**/ constexpr (std::is_same<T, uint8_t>::value) return p.getUChar(name);
    else if constexpr (std::is_same<T, char>::value) return p.getChar(name);
    else if constexpr (std::is_same<T, uint16_t>::value) return p.getUShort(name);
    else if constexpr (std::is_same<T, int16_t>::value) return p.getShort(name);
    else if constexpr (std::is_same<T, uint32_t>::value) return p.getUInt(name);
    else if constexpr (std::is_same<T, int32_t>::value) return p.getInt(name);
    else if constexpr (std::is_same<T, uint64_t>::value) return p.getULong64(name);
    else if constexpr (std::is_same<T, int64_t>::value) return p.getLong64(name);
    else if constexpr (std::is_same<T, float>::value) return p.getFloat(name);
    else if constexpr (std::is_same<T, double>::value) return p.getDouble(name);
    else if constexpr (std::is_same<T, bool>::value) return p.getBool(name);
    else if constexpr (std::is_same<T, String>::value) return p.getString(name);
    else static_assert(std::is_same<T, T>::value, "Unsupported type");
}

template<typename T>
void NVS::put(Preferences &p, const char *name, const T &value) {
    if (value == get<T>(p, name)) return; // No need to store the same value
    if /**/ constexpr (std::is_same<T, uint8_t>::value) p.putUChar(name, value);
    else if constexpr (std::is_same<T, char>::value) p.putChar(name, value);
    else if constexpr (std::is_same<T, uint16_t>::value) p.putUShort(name, value);
    else if constexpr (std::is_same<T, int16_t>::value) p.putShort(name, value);
    else if constexpr (std::is_same<T, uint32_t>::value) p.putUInt(name, value);
    else if constexpr (std::is_same<T, int32_t>::value) p.putInt(name, value);
    else if constexpr (std::is_same<T, uint64_t>::value) p.putULong64(name, value);
    else if constexpr (std::is_same<T, int64_t>::value) p.putLong64(name, value);
    else if constexpr (std::is_same<T, float>::value) p.putFloat(name, value);
    else if constexpr (std::is_same<T, double>::value) p.putDouble(name, value);
    else if constexpr (std::is_same<T, bool>::value) p.putBool(name, value);
    else if constexpr (std::is_same<T, String>::value) p.putString(name, value);
    else static_assert(std::is_same<T, T>::value, "Unsupported type");
}

#endif //NVS_HPP
