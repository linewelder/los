#pragma once

#include <stdint.h>

inline void outl(uint16_t address, uint32_t data) {
    asm volatile("outl %1, %0"
        :
        : "d" (address), "a" (data));
}

inline void outw(uint16_t address, uint16_t data) {
    asm volatile("outw %1, %0"
        :
        : "d" (address), "a" (data));
}

inline void outb(uint16_t address, uint8_t data) {
    asm volatile("outb %1, %0"
        :
        : "d" (address), "a" (data));
}

inline uint32_t inl(uint16_t address) {
    uint32_t result;
    asm volatile("inl %1, %0"
        : "=a" (result)
        : "d" (address));
    return result;
}

inline uint16_t inw(uint16_t address) {
    uint16_t result;
    asm volatile("inw %1, %0"
        : "=a" (result)
        : "d" (address));
    return result;
}

inline uint8_t inb(uint16_t address) {
    uint8_t result;
    asm volatile("inb %1, %0"
        : "=a" (result)
        : "d" (address));
    return result;
}

inline void io_wait() {
    outb(0x80, 0);
}

inline void disable_interrupts() {
    asm volatile("cli");
}

inline void enable_interrupts() {
    asm volatile("sti");
}

inline void hlt() {
    asm volatile("hlt");
}

/**
 * To be used in polling loops.
 */
inline void tiny_delay() {
    asm volatile(
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t");
}
