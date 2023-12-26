#include <util/memory.hpp>

#include <stdint.h>

int memcmp(const void* a, const void* b, size_t size) {
    const uint8_t* a_bytes = reinterpret_cast<const uint8_t*>(a);
    const uint8_t* b_bytes = reinterpret_cast<const uint8_t*>(b);

    for (size_t i = 0; i < size; i++) {
        if (a_bytes[i] < b_bytes[i]) {
            return -1;
        } else if (a_bytes[i] > b_bytes[i]) {
            return 1;
        }
    }

    return 0;
}
