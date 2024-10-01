#include "weather.h"

static inline constexpr const auto TICKER_INTERVAL = 30 * 60; // 30 minutes

static weather::data weatherData;
static Ticker weatherTicker;
static const JsonDocument filter = [] {
    static constexpr auto filter = R"({
        "weather": [{ "description": true }],
        "main": { "temp": true, "feels_like": true },
        "wind": { "speed": true, "deg": true }
    })";
    JsonDocument doc;
    deserializeJson(doc, filter);
    return doc;
}();

static String getURL();
static String getDir(float);
static void update(const JsonDocument &doc);

static void fetchWeather() {
    log_d("Fetching weather data");
    http_request::getJson(getURL().c_str(), update, filter);
}


/*!
 * @brief Setup the weather module. The weather data will be updated every 30 minutes using the OpenWeather API
 */
void weather::setup() {
    weatherTicker.attach(TICKER_INTERVAL, fetchWeather);
    fetchWeather();
}

/*!
 * @brief Get the current weather data
 * @return The current weather data
 */
const weather::data &weather::get() { return weatherData; }

/*!
 * @brief Get a string representation of the current weather data
 * @return A string representation of the current weather data
 */
String weather::dataString() {
    return weatherData.description + "\n"
           + weatherData.temp + "°C (" + weatherData.feels_like + "°C)\n"
           + weatherData.wind_speed + "m/s (" + weatherData.wind_direction + ")";
}


static String getURL() {
    auto url = String{"http://api.openweathermap.org/data/2.5/weather?units=metric&"}
               + "lat=" + String{*global::latitude, 4} + "&"
               + "lon=" + String{*global::longitude, 4} + "&"
               + "appid=" + OPEN_WEATHER_API_KEY;
    log_d("URL: %s", url.c_str());
    return url;
}

static String getDir(float deg) {
    static constexpr const char *directions[] = {
            "N", "NNE", "NE", "ENE",
            "E", "ESE", "SE", "SSE",
            "S", "SSW", "SW", "WSW",
            "W", "WNW", "NW", "NNW"
    };
    return directions[static_cast<int>((deg + 11.25) / 22.5) % 16];
}

static void update(const JsonDocument &doc) {
    weatherData.description = doc["weather"][0]["description"].as<String>();
    weatherData.temp = doc["main"]["temp"].as<float>();
    weatherData.feels_like = doc["main"]["feels_like"].as<float>();
    weatherData.wind_speed = doc["wind"]["speed"].as<float>();
    weatherData.wind_direction = getDir(doc["wind"]["deg"].as<float>());
    // turn first char of description to upper case
    weatherData.description[0] = static_cast<char>(toupper(weatherData.description[0]));
    log_i("Weather updated\n%s", weather::dataString().c_str());
}
