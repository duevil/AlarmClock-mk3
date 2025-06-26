#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "util/boot_process.hpp"
#include "util/thread.hpp"
#include <Adafruit_SHT4x.h>
#include <hp_BH1750.h>
#include <Arduino.h>
#include <cmath>


/**
 * Class for managing external temperatur, humidity and light sensors,
 * averaging the values and emitting events when new readings are available
 */
class SensorManager final : BootProcess, Thread<>
{
public:
    SensorManager();

    /**
     * Get the current average temperature in °C
     */
    [[nodiscard]] float temperature() const;

    /**
     * Get the current average humidity in %
     */
    [[nodiscard]] float humidity() const;

    /**
     * Get the current average light value in lux
     */
    [[nodiscard]] float light() const;

private:
    void runBootProcess() override;
    void run() override;

    template <size_t size>
    struct AveragingValue : private std::array<float, size>
    {
        AveragingValue() : std::array<float, size>({})
        {
            this->fill(NAN);
        }

        AveragingValue& operator<<(float value)
        {
            if (!isnan(value))
            {
                average = 0;
                (*this)[index] = value;
                index = (index + 1) % size;
                for (float f : *this)
                {
                    if (isnan(f)) f = value;
                    average += f;
                }
                average /= size;
            }
            return *this;
        }

        float average = NAN;
        size_t index = 0;
    };

    AveragingValue<32> m_temperature{};
    AveragingValue<32> m_humidity{};
    AveragingValue<32> m_light{};
#ifndef WOKWI
    Adafruit_SHT4x m_sht4x{};
    hp_BH1750 m_bh1750{};
#endif
};


#endif //SENSOR_MANAGER_H
