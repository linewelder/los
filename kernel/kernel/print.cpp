#include <kernel/print.hpp>

#include <arch/i386/terminal.hpp>

static void print_number(int value, int base) {
    if (value == 0) {
        terminal::putchar('0');
    }

    if (value < 0) {
        terminal::putchar('-');
        value = -value;
    }

    constexpr int BUF_SIZE = 12;
    char buf[BUF_SIZE];
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

void print_value(StringView value, char) {
    for (size_t i = 0; i < value.get_size(); i++) {
        terminal::putchar(value[i]);
    }
}

void print_value(const char* value, char) {
    terminal::write_cstr(value);
}

void print_value(int value, char format) {
    if (format == 'x') {
        print_number(value, 16);
    } else {
        print_number(value, 10);
    }
}

void println() {
    terminal::putchar('\n');
}
