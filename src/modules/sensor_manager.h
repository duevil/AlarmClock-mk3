#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "util/boot_process.hpp"
#include "util/thread.hpp"
#include "util/averaging_value.hpp"
#include <Adafruit_SHT4x.h>


/**
 * Class for managing external temperatur, humidity and light sensors,
 * averaging the values and emitting events when new readings are available
 */
class SensorManager final : BootProcess, Thread<>
{
public:
    explicit SensorManager(uint8_t ldr_pin);

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

    uint8_t m_ldr_pin;
    AveragingValue<float, 32> m_temperature{};
    AveragingValue<float, 32> m_humidity{};
    AveragingValue<float, 32> m_light{};
#ifndef WOKWI
    Adafruit_SHT4x m_sht4x{};
#endif
};


#endif //SENSOR_MANAGER_H
