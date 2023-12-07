#pragma once

#include <stdint.h>
#include <stddef.h>
#include <util/option.hpp>

namespace pci {
    class Function {
    public:
        constexpr Function() : bus(0), device(0), function(0) {}
        constexpr Function(uint8_t bus, uint8_t device, uint8_t function)
            : bus(bus), device(device), function(function) {}

        uint8_t get_bus() const;
        uint8_t get_device() const;
        uint8_t get_function() const;

        uint16_t get_vendor() const;
        uint16_t get_device_id() const;
        uint16_t get_full_class() const;
        /// Programming interface byte.
        uint8_t get_prog_if() const;
        bool has_multiple_functions() const;

        /**
         * Read I/O port written in a Base Address Regiter.
         * Does not work for PCI-to-CardBus.
         *
         * offset - 0..1 for PCI-to-PCI bridges,
         *          0..5 for everything else.
         */
        uint32_t get_bar_io(uint8_t offset) const;

        /// For PCI-to-PCI bridges only.
        uint8_t get_secondary_bus() const;

    private:
        uint8_t bus;
        uint8_t device;
        uint8_t function;
    };

    const Function& get_function(size_t index);

    size_t get_function_count();

    Option<const Function&> find_function_with_class(uint16_t full_class);

    void init();
}
