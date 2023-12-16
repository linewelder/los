#include <kernel/print.hpp>

#include <arch/i386/terminal.hpp>
#include <util/array.hpp>

void print_value(StringView value, char) {
    for (size_t i = 0; i < value.get_size(); i++) {
        terminal::putchar(value[i]);
    }
}

void print_value(const char* value, char) {
    terminal::write_cstr(value);
}

template <typename T>
static void print_number_with_base(T value, int base) {
    if (value == 0) {
        terminal::putchar('0');
        return;
    }

    if (value < 0) {
        terminal::putchar('-');
        value = -value;
    }

    constexpr int BUF_SIZE = 18; // Enough for max uint64_t.
    Array<char, BUF_SIZE> buf;
    int buf_start = BUF_SIZE;

    while (value) {
        buf_start--;
        int digit = value % base;
        if (digit < 10) {
            buf[buf_start] = '0' + digit;
        } else {
            buf[buf_start] = 'a' + digit - 10;
        }

        value /= base;
    }

    for (int k = buf_start; k < BUF_SIZE; k++) {
        terminal::putchar(buf[k]);
    }
}

template <typename T>
static void print_number(T value, char format) {
    switch (format) {
    case 'x':
        print_number_with_base(value, 16);
        break;

    case 'p':
        print_value("0x", '\0');
        print_number_with_base(value, 16);
        break;

    case 'c':
        terminal::putchar(value);
        break;

    default:
        print_number_with_base(value, 10);
        break;
    }
}

void print_value(int value, char format) {
    print_number(value, format);
}

void print_value(uint32_t value, char format) {
    print_number(value, format);
}

void print_value(uint64_t value, char format) {
    print_number(value, format);
}

void println() {
    terminal::putchar('\n');
}
