#ifndef PIN_MAP_H
#define PIN_MAP_H

#define MAP_PIN(y, x) constexpr uint8_t x = GPIO_NUM_##y;


namespace pins
{
    MAP_PIN(13, lights);
    MAP_PIN(25, matrix_cs);
    MAP_PIN(34, alarm_interrupt)
}


#endif //PIN_MAP_H
