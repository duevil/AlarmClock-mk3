#ifndef FONTS_H
#define FONTS_H

#include <U8g2lib.h>

namespace fonts {
    template<auto s, auto b = false>
    constexpr auto cour = [] {
        static_assert(s == 8 || s == 10 || s == 12 || s == 14 || s == 18 || s == 24, "Invalid font size");
        if constexpr (b) {
            if constexpr (s == 8) return u8g2_font_courB08_tf;
            if constexpr (s == 10) return u8g2_font_courB10_tf;
            if constexpr (s == 12) return u8g2_font_courB12_tf;
            if constexpr (s == 14) return u8g2_font_courB14_tf;
            if constexpr (s == 18) return u8g2_font_courB18_tf;
            if constexpr (s == 24) return u8g2_font_courB24_tf;
        } else {
            if constexpr (s == 8) return u8g2_font_courR08_tf;
            if constexpr (s == 10) return u8g2_font_courR10_tf;
            if constexpr (s == 12) return u8g2_font_courR12_tf;
            if constexpr (s == 14) return u8g2_font_courR14_tf;
            if constexpr (s == 18) return u8g2_font_courR18_tf;
            if constexpr (s == 24) return u8g2_font_courR24_tf;
        }
    }();
    template<auto s, auto b = false>
    constexpr auto helv = [] {
        static_assert(s == 8 || s == 10 || s == 12 || s == 14 || s == 18 || s == 24, "Invalid font size");
        if constexpr (b) {
            if constexpr (s == 8) return u8g2_font_helvB08_tf;
            if constexpr (s == 10) return u8g2_font_helvB10_tf;
            if constexpr (s == 12) return u8g2_font_helvB12_tf;
            if constexpr (s == 14) return u8g2_font_helvB14_tf;
            if constexpr (s == 18) return u8g2_font_helvB18_tf;
            if constexpr (s == 24) return u8g2_font_helvB24_tf;
        } else {
            if constexpr (s == 8) return u8g2_font_helvR08_tf;
            if constexpr (s == 10) return u8g2_font_helvR10_tf;
            if constexpr (s == 12) return u8g2_font_helvR12_tf;
            if constexpr (s == 14) return u8g2_font_helvR14_tf;
            if constexpr (s == 18) return u8g2_font_helvR18_tf;
            if constexpr (s == 24) return u8g2_font_helvR24_tf;
        }
    }();
}

#endif //FONTS_H
