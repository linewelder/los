#include <util/memory.hpp>

#include <stdint.h>

void* memcpy(void* dst, const void* src, size_t size) {
    uint8_t* dst_bytes = reinterpret_cast<uint8_t*>(dst);
    const uint8_t* src_bytes = reinterpret_cast<const uint8_t*>(src);

    for (size_t i = 0; i < size; i++) {
        dst_bytes[i] = src_bytes[i];
    }

    return dst;
}

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
