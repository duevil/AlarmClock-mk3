#include "sensor_manager.h"

#include "event_definitions.h"


SensorManager::SensorManager() : BootProcess("Sensors initialized"),
                                 Thread({.name = "sensors", .priority = 0, .coreId = PRO_CPU_NUM}) {}


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
    // TODO: initialize sensors
#endif
}

void SensorManager::run()
{
#ifdef WOKWI
    m_temperature << static_cast<float>(random(18, 25));
    m_humidity << static_cast<float>(random(60, 80));
    m_light << static_cast<float>(random(0, 10000));
    SENSOR_EVENT << TEMPERATURE << m_temperature.average;
    SENSOR_EVENT << HUMIDITY << m_humidity.average;
    SENSOR_EVENT << LIGHT << m_light.average;
    delay(1000);
#else
    // TODO: read sensors
#endif
}
