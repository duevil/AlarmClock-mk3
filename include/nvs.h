#ifndef NVS_H
#define NVS_H

#include <Preferences.h>

namespace nvs {
    //! Template wrapper for the Preferences get-function.
    template<typename T> T get(Preferences &prefs, const char *key, T);
    //! Template wrapper for the Preferences put-function.
    template<typename T> void put(Preferences &, const char *, const T &);

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
     * @brief A generic class wrapping the Preferences functions around an existing variable.
     * @tparam T The type of the variable. Must be one of the types (uint8_t, char, uint16_t, int16_t,
     * uint32_t, int32_t, uint64_t, int64_t, float, double, bool, String) supported by the Preferences API.
     */
    template<typename T> requires is_nvs_type_v<T>
    struct Var {
        const char *key; //!< The key to store the variable under.
        T &var; //!< A reference to the variable.

        /*!
         * @brief Constructor that takes a reference to a variable and a key to store it under.
         * @param key The key to store the variable under.
         * @param var A reference to the variable.
         */
        Var(const char *key, T &var) : key(key), var(var) {
            assert(strlen(key) <= 15 && "Key must be 15 characters or less.");
        }
    };

    //! Template variable that is true if the type is supported by the Preferences API.
    template<typename> constexpr bool is_var_v = false;
    template<typename T> requires is_nvs_type_v<T> constexpr bool is_var_v<Var<T>> = true;

    /*!
     * @brief A class that handles the Preferences API for a set of predefined variables, allowing for easy bulk loading
     * and storing. To add variables to the handler, simply pass them as a list of nvs::Var objects to the constructor.
     * Only variables whose types are supported by the Preferences API can be added (uint8_t, char, uint16_t, int16_t,
     * uint32_t, int32_t, uint64_t, int64_t, float, double, bool, String).
     * @tparam Vars A variadic list of object conforming to the nvs::is_var_v concept.
     */
    template<typename... Vars> requires (is_var_v<Vars> && ...)

    class Handler {
        static constexpr const char *NAMESPACE = "alarm_clock";
        std::tuple<Vars...> vars;
        Preferences prefs;
    public:

        /*!
         * @brief Constructor that takes a list of nvs::Var objects.
         * @param vars A list of nvs::Var objects.
         */
        explicit Handler(Vars... vars) : vars(std::move(vars)...) {}

        /*!
         * @brief Initializes the Preferences API and loads the stored values into the variables.
         */
        void setup() {
            prefs.begin(NAMESPACE);
            std::apply([this](auto &... v) {
                ((v.var = get(prefs, v.key, v.var)), ...);
            }, vars);
        }

        /*!
         * @brief Stores the values of all the variables into the Preferences API.
         * @note Only variables whose values have changed will be stored, making this function safe to call frequently.
         */
        void store_all() {
            std::apply([this](auto &... v) {
                (put(prefs, v.key, v.var), ...);
            }, vars);
        }
    };
}

template<typename T> T nvs::get(Preferences &prefs, const char *key, T val) {
    /**/ if constexpr (std::is_same_v<T, int8_t>) val = prefs.getChar(key, val);
    else if constexpr (std::is_same_v<T, uint8_t>) val = prefs.getUChar(key, val);
    else if constexpr (std::is_same_v<T, int16_t>) val = prefs.getShort(key, val);
    else if constexpr (std::is_same_v<T, uint16_t>) val = prefs.getUShort(key, val);
    else if constexpr (std::is_same_v<T, int32_t>) val = prefs.getInt(key, val);
    else if constexpr (std::is_same_v<T, uint32_t>) val = prefs.getUInt(key, val);
    else if constexpr (std::is_same_v<T, int64_t>) val = prefs.getLong(key, val);
    else if constexpr (std::is_same_v<T, uint64_t>) val = prefs.getULong(key, val);
    else if constexpr (std::is_same_v<T, float>) val = prefs.getFloat(key, val);
    else if constexpr (std::is_same_v<T, double>) val = prefs.getDouble(key, val);
    else if constexpr (std::is_same_v<T, bool>) val = prefs.getBool(key, val);
    else if constexpr (std::is_same_v<T, String>) val = prefs.getString(key, val);
    return val;
}

template<typename T> void nvs::put(Preferences &prefs, const char *key, const T &var) {
    if (var == nvs::get(prefs, key, var)) { return; }
    /**/ if constexpr (std::is_same_v<T, int8_t>) prefs.putChar(key, var);
    else if constexpr (std::is_same_v<T, uint8_t>) prefs.putUChar(key, var);
    else if constexpr (std::is_same_v<T, int16_t>) prefs.putShort(key, var);
    else if constexpr (std::is_same_v<T, uint16_t>) prefs.putUShort(key, var);
    else if constexpr (std::is_same_v<T, int32_t>) prefs.putInt(key, var);
    else if constexpr (std::is_same_v<T, uint32_t>) prefs.putUInt(key, var);
    else if constexpr (std::is_same_v<T, int64_t>) prefs.putLong(key, var);
    else if constexpr (std::is_same_v<T, uint64_t>) prefs.putULong(key, var);
    else if constexpr (std::is_same_v<T, float>) prefs.putFloat(key, var);
    else if constexpr (std::is_same_v<T, double>) prefs.putDouble(key, var);
    else if constexpr (std::is_same_v<T, bool>) prefs.putBool(key, var);
    else if constexpr (std::is_same_v<T, String>) prefs.putString(key, var);
}

#endif //NVS_H
