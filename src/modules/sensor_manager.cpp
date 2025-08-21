#include "sensor_manager.h"

#include "log.h"
#include "event_definitions.h"


SensorManager::SensorManager(uint8_t ldr_pin)
    : BootProcess("Sensors initialized"), Thread({.name = "sensors"}),
      m_ldr_pin(ldr_pin) {}

float SensorManager::temperature() const { return m_temperature; }

float SensorManager::humidity() const { return m_humidity; }

float SensorManager::light() const { return m_light * -15.f / 4095.f + 15.f; }

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
#endif
}

void SensorManager::run()
{
    m_light << static_cast<float>(analogRead(m_ldr_pin));
    SENSOR_EVENT << LIGHT << light();

#ifndef WOKWI
    if (m_sht4x.hasEvent())
    {
        sensors_event_t temp;
        sensors_event_t humidity;
        m_sht4x.fillEvent(&humidity, &temp);
        m_temperature << temp.temperature;
        m_humidity << humidity.relative_humidity;
        SENSOR_EVENT << TEMPERATURE << m_temperature.get();
        SENSOR_EVENT << HUMIDITY << m_humidity.get();
        m_sht4x.startEvent();
    }
#else
    if (static auto last = millis(); millis() - last > 1000)
    {
        m_temperature << static_cast<float>(random(18, 25));
        m_humidity << static_cast<float>(random(60, 80));
        SENSOR_EVENT << TEMPERATURE << m_temperature.get();
        SENSOR_EVENT << HUMIDITY << m_humidity.get();
        last = millis();
    }
#endif

    delay(50);
}
