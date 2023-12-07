#pragma once

#include <stdint.h>

namespace idt {
    struct EncodedIdtEntry {
        uint16_t offset_1;
        uint16_t segment;
        uint8_t zero;
        uint8_t attributes;
        uint16_t offset_2;
    };

    constexpr uint8_t ATTR_TASK_GATE = 5;
    constexpr uint8_t ATTR_16_BIT_INTERRUPT = 6;
    constexpr uint8_t ATTR_16_BIT_TRAP = 7;
    constexpr uint8_t ATTR_32_BIT_INTERRUPT = 14;
    constexpr uint8_t ATTR_32_BIT_TRAP = 15;

    /// Must be set.
    constexpr uint8_t ATTR_VALID = 1 << 7;
    /// Required level to access via int.
    constexpr uint8_t ATTR_RING(uint8_t level) {
        return level << 5;
    }

    struct InterruptFrame {
        uint32_t ip;
        uint16_t cs;
        uint32_t flags;
    };

    typedef void (*Handler)(InterruptFrame*);
    typedef void (*ErrorCodeHandler)(InterruptFrame*, int);

    void register_interrupt(int vector, Handler handler);
    void register_trap(int vector, Handler handler);
    void register_trap(int vector, ErrorCodeHandler handler);

    void init();
};
