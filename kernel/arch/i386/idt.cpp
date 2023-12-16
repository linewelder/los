#include <arch/i386/idt.hpp>

#include <util/array.hpp>
#include <util/bits.hpp>

namespace idt {
    static Array<EncodedIdtEntry, 256> idt;

    static void register_gate(uint8_t vector, uint32_t address, uint8_t attribute_type) {
        idt[vector].offset_1 = get_bit_range(address, 0, 16);
        idt[vector].offset_2 = get_bit_range(address, 16, 16);
        idt[vector].segment = 0x08;
        idt[vector].zero = 0;
        idt[vector].attributes = ATTR_VALID | ATTR_RING(3) | attribute_type;
    }

    void register_interrupt(uint8_t vector, Handler handler) {
        register_gate(vector, reinterpret_cast<uint32_t>(handler), ATTR_32_BIT_INTERRUPT);
    }

    void register_trap(uint8_t vector, Handler handler) {
        register_gate(vector, reinterpret_cast<uint32_t>(handler), ATTR_32_BIT_TRAP);
    }

    void register_trap(uint8_t vector, ErrorCodeHandler handler) {
        register_gate(vector, reinterpret_cast<uint32_t>(handler), ATTR_32_BIT_TRAP);
    }

    struct __attribute__((packed)) {
        uint16_t size;
        uint32_t base;
    } idtr;

    __attribute__((interrupt))
    static void void_handler(idt::InterruptFrame*) {}

    void init() {
        for (int i = 0; i < 256; i++) {
            register_interrupt(i, void_handler);
        }

        idtr.size = sizeof(idt);
        idtr.base = reinterpret_cast<uint32_t>(&idt);
        asm volatile("lidt %0" : : "m"(idtr));
    }
}
