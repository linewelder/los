#include <kernel/print.h>

#include <kernel/printf.h>

void print_value(StringView value, char) {
    for (size_t i = 0; i < value.get_size(); i++) {
        printf("%c", value[i]);
    }
}

void print_value(const char* value, char) {
    printf("%s", value);
}

void print_value(int value, char format) {
    if (format == 'x') {
        printf("%x", value);
    } else {
        printf("%d", value);
    }
}
