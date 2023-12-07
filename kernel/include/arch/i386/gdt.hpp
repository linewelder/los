#pragma once

#include <stdint.h>

namespace gdt {
    /// Must be set for valid segments.
    constexpr uint8_t ACCESS_VALID = 1 << 7;
    constexpr uint8_t ACCESS_RING(uint8_t level) {
        return level << 5;
    }
    /// As opposed to a system segment.
    constexpr uint8_t ACCESS_CODE_DATA = 1 << 4;
    constexpr uint8_t ACCESS_EXECUTABLE = 1 << 3;
    /// For data segments only.
    constexpr uint8_t ACCESS_GROWS_DOWN = 1 << 2;
    /// For code segments only. Allow far jumps from higher-ring segments.
    constexpr uint8_t ACCESS_CONFORMING = 1 << 2;
    /// For code segments only.
    constexpr uint8_t ACCESS_READABLE = 1 << 1;
    // For data segments only.
    constexpr uint8_t ACCESS_WRITABLE = 1 << 1; 
    constexpr uint8_t ACCESS_ACCESSED = 1 << 0;

    /// As opposed to limit in bytes.
    constexpr uint8_t FLAG_LIMIT_IN_PAGES = 1 << 3;
    /// As opposed to 16-bit protected mode.
    constexpr uint8_t FLAG_32_BIT = 1 << 2;
    /// If set, FLAG_32_BIT must be clear.
    constexpr uint8_t FLAG_LONG_MODE = 1 << 1;

    struct EncodedGdtEntry {
        uint8_t data[8];
    };

    struct GdtEntry {
        uint32_t base;
        uint32_t limit; // 20-bit available.
        uint8_t access_byte;
        uint8_t flags;
    };

    void init();
}
