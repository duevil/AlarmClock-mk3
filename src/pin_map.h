#ifndef PIN_MAP_H
#define PIN_MAP_H


struct PinTypeHelper
{
    gpio_num_t pin;

    // ReSharper disable once CppNonExplicitConversionOperator
    constexpr operator gpio_num_t() const { return pin; } // NOLINT(*-explicit-constructor)
    // ReSharper disable once CppNonExplicitConversionOperator
    constexpr operator uint8_t() const { return pin; } // NOLINT(*-explicit-constructor)
};


#define MAP_PIN(y, x) constexpr PinTypeHelper x{GPIO_NUM_##y};


namespace pins
{
    MAP_PIN(13, lights);
    MAP_PIN(25, matrix_cs);
    MAP_PIN(34, alarm_interrupt)
    MAP_PIN(36, button_left)
    MAP_PIN(39, button_middle)
#ifdef WOKWI
    MAP_PIN(35, button_right)
#else
    MAP_PIN(37, button_right)
#endif
    MAP_PIN(8, ui_display_cs);
    MAP_PIN(7, ui_display_dc);
    MAP_PIN(4, ui_display_rst);
    MAP_PIN(12, i2s_data);
    MAP_PIN(27, i2s_bck);
    MAP_PIN(33, i2s_lrc);
    MAP_PIN(14, ldr);
}


#endif //PIN_MAP_H
