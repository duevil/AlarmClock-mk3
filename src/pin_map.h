#ifndef PIN_MAP_H
#define PIN_MAP_H


struct PinTypeHelper
{
    gpio_num_t pin;

    // ReSharper disable once CppNonExplicitConversionOperator
    constexpr operator gpio_num_t() const { return pin; }; // NOLINT(*-explicit-constructor)
    // ReSharper disable once CppNonExplicitConversionOperator
    constexpr operator uint8_t() const { return pin; }; // NOLINT(*-explicit-constructor)
};


#define MAP_PIN(y, x) constexpr PinTypeHelper x{GPIO_NUM_##y};


namespace pins
{
    MAP_PIN(13, lights);
    MAP_PIN(25, matrix_cs);
    MAP_PIN(34, alarm_interrupt)
    MAP_PIN(36, button_left)
    MAP_PIN(39, button_middle)
    MAP_PIN(35, button_right)
    MAP_PIN(8, ui_display_cs);
    MAP_PIN(7, ui_display_dc);
    MAP_PIN(4, ui_display_rst);
}


#endif //PIN_MAP_H
