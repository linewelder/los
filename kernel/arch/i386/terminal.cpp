#include <arch/i386/terminal.h>

#include <stdint.h>
#include <stddef.h>

namespace terminal {
    static uint16_t* const BUFFER = reinterpret_cast<uint16_t*>(0xb8000);

    static constexpr inline uint8_t vga_entry_color(Color fg, Color bg) {
        return static_cast<uint8_t>(fg) | static_cast<uint8_t>(bg) << 4;
    }

    static constexpr inline uint16_t vga_entry(unsigned char ch, uint8_t color) {
        return static_cast<uint16_t>(ch) | static_cast<uint16_t>(color) << 8;
    }

    static size_t row = 0;
    static size_t column = 0;
    static uint8_t color = vga_entry_color(Color::LIGHT_GREY, Color::BLACK);

    static inline uint16_t& buffer_entry(size_t x, size_t y) {
        return BUFFER[y * WIDTH + x];
    }

    static void next_line() {
        if (row == HEIGHT - 1) {
            for (size_t y = 0; y < HEIGHT - 1; y++) {
                for (size_t x = 0; x < WIDTH; x++) {
                    buffer_entry(x, y) = buffer_entry(x, y + 1);
                }
            }

            for (size_t x = 0; x < WIDTH; x++) {
                put_entry_at(' ',
                    terminal::Color::LIGHT_GREY, terminal::Color::BLACK,
                    x, HEIGHT - 1);
            }
        } else {
            row++;
        }
    }

    void set_color(Color fg, Color bg) {
        color = vga_entry_color(fg, bg);
    }

    void put_entry_at(char ch, Color fg, Color bg, size_t x, size_t y) {
        buffer_entry(x, y) = vga_entry(ch, vga_entry_color(fg, bg));
    }

    void clear() {
        row = 0;
        column = 0;
        for (size_t y = 0; y < HEIGHT; y++) {
            for (size_t x = 0; x < WIDTH; x++) {
                put_entry_at(' ', terminal::Color::LIGHT_GREY, terminal::Color::BLACK, x, y);
            }
        }
    }

    void putchar(char ch) {
        switch (ch) {
            case '\b':
                if (column > 0) {
                    column--;
                    buffer_entry(column, row) = vga_entry(' ', color);
                }
                break;

            case '\n':
                next_line();
                column = 0;
                break;

            default: {
                buffer_entry(column, row) = vga_entry(ch, color);

                column++;
                if (column == WIDTH) {
                    column = 0;
                    next_line();
                }

                break;
            }
        }
    }

    void write(const char* str, size_t len) {
        for (size_t i = 0; i < len; i++) {
            putchar(str[i]);
        }
    }

    void write_cstr(const char* str) {
        while (str[0]) {
            putchar(str[0]);
            str++;
        }
    }
}
