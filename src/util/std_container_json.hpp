#ifndef VECTOR_JSON_HPP
#define VECTOR_JSON_HPP

#include <ArduinoJson.h>
#include <vector>
#include <unordered_set>


// see https://arduinojson.org/v7/how-to/create-converters-for-stl-containers/


namespace ArduinoJson
{
    template <typename T>
    struct Converter<std::vector<T>>
    {
        static void toJson(const std::vector<T>& src, JsonVariant dst)
        {
            auto array = dst.to<JsonArray>();
            for (T &item : src)
                array.add(item);
        }

        static std::vector<T> fromJson(JsonVariantConst src)
        {
            std::vector<T> dst;
            for (T item : src.as<JsonArrayConst>())
                dst.push_back(item);
            return dst;
        }

        static bool checkJson(JsonVariantConst src)
        {
            JsonArrayConst array = src;
            bool result = array;
            for (JsonVariantConst item : array)
                result &= item.is<T>();
            return result;
        }
    };

    template <typename T>
    struct Converter<std::unordered_set<T>>
    {
        static void toJson(const std::unordered_set<T>& src, JsonVariant dst)
        {
            auto array = dst.to<JsonArray>();
            for (auto &item : src)
                array.add(item);
        }

        static std::unordered_set<T> fromJson(JsonVariantConst src)
        {
            std::unordered_set<T> dst;
            for (T item : src.as<JsonArrayConst>())
                dst.insert(item);
            return dst;
        }

        static bool checkJson(JsonVariantConst src)
        {
            JsonArrayConst array = src;
            bool result = array;
            for (JsonVariantConst item : array)
                result &= item.is<T>();
            return result;
        }
    };
} // namespace ArduinoJson


#endif //VECTOR_JSON_HPP
