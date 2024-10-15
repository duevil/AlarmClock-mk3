#include "sensors.h"

template<size_t N>
class SelfAveragingValue {
    float values[N]{};
    float average = NAN;
    size_t index = 0;
public:
    SelfAveragingValue() { for (auto &v: values) v = NAN; }

    friend auto operator<<(SelfAveragingValue &sav, auto value) {
        sav.values[sav.index] = value;
        sav.index = (sav.index + 1) % N;
        sav.average = 0;
        auto count = 0;
        for (auto v: sav.values) {
            if (isnan(v)) continue;
            sav.average += v;
            count++;
        }
        sav.average /= static_cast<decltype(sav.average)>(count);
        return sav.average;
    }

    auto operator*() const { return average; }
};

static constexpr inline auto BH1750_ADDRESS = 0x23;
static constexpr inline auto BH1750_ADJUSTMENT_PERCENT = 75;
static Adafruit_SHT4x sht4x{};
static hp_BH1750 bh1750{};
static SelfAveragingValue<10> temperature{};
static SelfAveragingValue<10> humidity{};
static SelfAveragingValue<5> light{};
static bool sht4x_available = false;
static bool bh1750_available = false;


/*!
 * @brief Setup the sensors.
 */
void sensors::setup() {
    if (!(sht4x_available = sht4x.begin())) {
        log_e("Could not find a valid SHT4x sensor, check wiring!");
    } else {
        sht4x.setPrecision(SHT4X_HIGH_PRECISION);
        sht4x.setHeater(SHT4X_NO_HEATER);
        sht4x.startEvent(); // start reading sensor data
        log_d("SHT4x sensor setup complete");
    }
    if (!(bh1750_available = bh1750.begin(BH1750_ADDRESS))) {
        log_e("Could not find a valid BH1750 sensor, check wiring!");
    } else {
        bh1750.setQuality(BH1750_QUALITY_HIGH2);
        bh1750.start();
        log_d("BH1750 sensor setup complete");
    }
}

/*!
 * @brief Loop function for the sensors. Reads the measurements from the sensors if available.
 */
void sensors::loop() {
    static uint32_t lastSht4xRead = 0;
    if (bh1750_available && bh1750.hasValue()) {
        ::light << bh1750.getLux();
        log_v("Light: %.2f lux", *::light);
        bh1750.adjustSettings(BH1750_ADJUSTMENT_PERCENT);
        bh1750.start();
    }
    // TODO: remove forced measurement interval
    if (sht4x_available && millis() - lastSht4xRead > 2000 && sht4x.hasEvent()) {
        ::temperature << sht4x.getTemperature();
        ::humidity << sht4x.getHumidity();
        log_v("Temperature: %.2f°C, Humidity: %.2f%%", *::temperature, *::humidity);
        sht4x.startEvent();
        lastSht4xRead = millis();
    }
}

/*!
 * @brief Get the current temperature
 * @return The current temperature
 */
float sensors::temperature() { return *::temperature; }

/*!
 * @brief Get the current humidity
 * @return The current humidity
 */
float sensors::humidity() { return *::humidity; }

/*!
 * @brief Get the current light level
 * @return The current light level
 */
float sensors::light() { return *::light; }
