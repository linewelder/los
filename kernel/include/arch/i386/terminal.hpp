#pragma once

#include <stdint.h>
#include <stddef.h>

namespace terminal {
    enum class Color {
        BLACK = 0,
        BLUE = 1,
        GREEN = 2,
        CYAN = 3,
        RED = 4,
        MAGENTA = 5,
        BROWN = 6,
        LIGHT_GREY = 7,
        DARK_GREY = 8,
        LIGHT_BLUE = 9,
        LIGHT_GREEN = 10,
        LIGHT_CYAN = 11,
        LIGHT_RED = 12,
        LIGHT_MAGENTA = 13,
        LIGHT_BROWN = 14,
        WHITE = 15,
    };

    constexpr const size_t WIDTH = 80;
    constexpr const size_t HEIGHT = 25;

    void put_entry_at(char ch, Color fg, Color bg, size_t x, size_t y);

    void clear();

    void set_color(Color fg, Color bg);

    void putchar(char ch);

    void write(const char* str, size_t len);

    void write_cstr(const char* str);
}
