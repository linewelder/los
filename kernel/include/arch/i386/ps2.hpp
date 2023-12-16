#pragma once

#include <stdint.h>
#include <stddef.h>
#include <arch/i386/idt.hpp>
#include <util/option.hpp>
#include <util/span.hpp>

namespace ps2 {
    class Device {
    public:
        constexpr Device() : id(0), type(0xffff) {}
        constexpr Device(uint16_t id) : id(id), type(0xffff) {}

        void send(uint8_t data) const;
        void disable_scanning() const;
        void enable_scanning() const;

        uint16_t get_type() const;
        const char* get_type_name() const;
        void identify();

        void set_interrupt_handler(idt::Handler handler) const;

    private:
        uint16_t id; /* 0 or 1 */
        uint16_t type; /** 0xffff if the device returns an empty response. */
    };

    void init();

    Span<const Device> get_devices();

    Option<const Device&> find_device_with_type(uint16_t type);

    Option<uint8_t> try_poll(int max_cycles = 500);

    uint8_t read_input();

    uint8_t read_config_byte();

    void write_config_byte(uint8_t value);
}
