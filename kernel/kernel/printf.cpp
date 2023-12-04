#include <limits.h>
#include <stdarg.h>

#include <arch/i386/terminal.h>

static int print_number(int value, int base, int max_length) {
    int written = 0;

    if (value == 0) {
        if (written == max_length) return -1;
        written++;
        terminal::putchar('0');
    }

    if (value < 0) {
        if (written == max_length) return -1;
        written++;
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
        if (written == max_length) return -1;
        written++;
        terminal::putchar(buf[k]);
    }

    return written;
}

/** Supports only %d %x %c %s formatters without precision and width. */
int vprintf(const char* format, va_list args) {
    int written = 0;
    while (*format != '\0') {
        size_t remained = INT_MAX - written;

        if (format[0] == '%') {
            switch (format[1]) {
                case 'd': {
                    int i = va_arg(args, int);
                    int result = print_number(i, 10, remained);
                    if (result < 0) {
                        return -1;
                    }
                    break;
                }

                case 'x': {
                    int i = va_arg(args, int);
                    int result = print_number(i, 16, remained);
                    if (result < 0) {
                        return -1;
                    }
                    break;
                }

                case 'c': {
                    char ch = static_cast<char>(va_arg(args, int)); // char promotes to int.
                    if (remained == 0) return -1;
                    terminal::putchar(ch);
                    written++;
                    break;
                }

                case 's': {
                    const char* str = va_arg(args, const char*);
                    while (str[0]) {
                        if (remained == 0) return -1;
                        terminal::putchar(str[0]);
                        written++;
                        str++;
                    }
                    break;
                }

                case '%': {
                    if (remained == 0) return -1;
                    written++;
                    terminal::putchar('%');
                    break;
                }

                default: {
                    if (remained == 0) return -1;
                    written++;
                    terminal::putchar('%');
                    format++;
                    continue;
                }
            }

            format += 2;
        } else {
            terminal::putchar(format[0]);
            format++;
        }
    }

    return written;
}

/** Supports only %d %x %c %s formatters without precision and width. */
int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    int written = vprintf(format, args);

    va_end(args);
    return written;
}
