#ifndef SERIAL_HPP
#define SERIAL_HPP

#include "device.hpp"


namespace logging
{
    struct SerialLog final : Device
    {
        using Device::Device;

        bool initialize() override
        {
            Serial.begin(SERIAL_BAUD_RATE);
            return true;
        }

        size_t write(uint8_t data) override
        {
            return Serial.write(data);
        }

        int availableForWrite() override
        {
            return Serial.availableForWrite();
        }

        void flush() override
        {
            Serial.flush();
        }
    };
}


#endif //SERIAL_HPP
