#ifndef PIN_MAP_H
#define PIN_MAP_H

#define MAP_PIN(y, x) constexpr uint8_t x = GPIO_NUM_##y;


namespace pins
{
    MAP_PIN(13, LIGHTS);
}


#endif //PIN_MAP_H
