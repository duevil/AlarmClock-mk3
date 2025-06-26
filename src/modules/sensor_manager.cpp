#include "sensor_manager.h"

#include "log.h"
#include "event_definitions.h"


SensorManager::SensorManager() : BootProcess("Sensors initialized"), Thread({.name = "sensors"}) {}

float SensorManager::temperature() const
{
    return m_temperature.average;
}

float SensorManager::humidity() const
{
    return m_humidity.average;
}

float SensorManager::light() const
{
    return m_light.average;
}

void SensorManager::runBootProcess()
{
#ifndef WOKWI

    if (!m_sht4x.begin())
    {
        LOG_E("SHT4X initialization failed");
    }
    else
    {
        m_sht4x.setPrecision(SHT4X_LOW_PRECISION);
        m_sht4x.setHeater(SHT4X_NO_HEATER);
        m_sht4x.startEvent();
    }

    if (!m_bh1750.begin(BH1750_TO_GROUND))
    {
        LOG_E("SHT4X initialization failed");
    }
    else
    {
        m_bh1750.setQuality(BH1750_QUALITY_HIGH2);
        m_bh1750.calibrateTiming();
        m_bh1750.start();
    }

#endif
}

void SensorManager::run()
{
#ifndef WOKWI

    if (m_sht4x.hasEvent())
    {
        sensors_event_t temp;
        sensors_event_t humidity;
        m_sht4x.fillEvent(&humidity, &temp);
        m_temperature << temp.temperature;
        m_humidity << humidity.relative_humidity;
        SENSOR_EVENT << TEMPERATURE << m_temperature.average;
        SENSOR_EVENT << HUMIDITY << m_humidity.average;
        m_sht4x.startEvent();
    }

    if (m_bh1750.hasValue())
    {
        m_light << m_bh1750.getLux();
        SENSOR_EVENT << LIGHT << m_light.average;
        m_bh1750.start();
    }

#else
    m_temperature << static_cast<float>(random(18, 25));
    m_humidity << static_cast<float>(random(60, 80));
    m_light << static_cast<float>(random(0, 10000));
    SENSOR_EVENT << TEMPERATURE << m_temperature.average;
    SENSOR_EVENT << HUMIDITY << m_humidity.average;
    SENSOR_EVENT << LIGHT << m_light.average;
    delay(1000);
#endif
}
