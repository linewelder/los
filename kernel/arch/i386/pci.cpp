#include <arch/i386/pci.hpp>

#include <arch/i386/asm.hpp>
#include <kernel/log.hpp>
#include <util/bits.hpp>
#include <util/inplace_vector.hpp>

namespace pci {
    static constexpr uint16_t CONFIG_ADDRESS_PORT = 0xcf8;
    static constexpr uint16_t CONFIG_DATA_PORT = 0xcfc;

    /**
     * bus      - 8 bits available (up to 0xff).
     * device   - 5 bits available (up to 0x20).
     * function - 3 bits available (up to 0x08).
     * offset   - 8 bits available (up to 0xff),
     *            has to be aligned to 4 bytes.
     */
    static uint32_t config_read_u32(
        uint8_t bus, uint8_t device,
        uint8_t function, uint8_t offset)
    {
        uint32_t address =
            0x80000000 |
            (bus << 16) |
            (device << 11) |
            (function << 8) |
            offset;

        outl(CONFIG_ADDRESS_PORT, address);
        return inl(CONFIG_DATA_PORT);
    }

    /**
     * bus      - 8 bits available (up to 0xff).
     * device   - 5 bits available (up to 0x20).
     * function - 3 bits available (up to 0x08).
     * offset   - 8 bits available (up to 0xff),
     *            has to be aligned to 2 bytes.
     */
    static uint16_t config_read_u16(
        uint8_t bus, uint8_t device,
        uint8_t function, uint8_t offset)
    {
        uint32_t dword = config_read_u32(bus, device, function, offset & 0xfc);
        if ((offset & 0x03) == 0) {
            return get_bit_range(dword, 0, 16);
        } else {
            return get_bit_range(dword, 16, 16);
        }
    }

    /**
     * bus      - 8 bits available (up to 0xff).
     * device   - 5 bits available (up to 0x20).
     * function - 3 bits available (up to 0x08).
     * offset   - 8 bits available (up to 0xff).
     */
    static uint16_t config_read_u8(
        uint8_t bus, uint8_t device,
        uint8_t function, uint8_t offset)
    {
        uint32_t dword = config_read_u32(bus, device, function, offset & 0xfc);
        unsigned offset_within_dword = (offset & 0x03) * 8;
        return get_bit_range(dword, offset_within_dword, 8);
    }

    static InplaceVector<Function, 256> functions;

    const Function& get_function(size_t index) {
        return functions[index];
    }

    size_t get_function_count() {
        return functions.get_count();
    }

    Option<const Function&> find_function_with_class(uint16_t full_class) {
        for (size_t i = 0; i < functions.get_count(); i++) {
            if (functions[i].get_full_class() == full_class) {
                return functions[i];
            }
        }
        return {};
    }

    uint8_t Function::get_bus() const {
        return bus;
    }

    uint8_t Function::get_device() const {
        return device;
    }

    uint8_t Function::get_function() const {
        return function;
    }

    uint16_t Function::get_vendor() const {
        return config_read_u16(bus, device, function, 0x00);
    }

    uint16_t Function::get_device_id() const {
        return config_read_u16(bus, device, function, 0x02);
    }

    uint16_t Function::get_full_class() const {
        return config_read_u16(bus, device, function, 0x0a);
    }

    /// Programming interface byte.
    uint8_t Function::get_prog_if() const {
        return config_read_u8(bus, device, function, 0x09);
    }

    bool Function::has_multiple_functions() const {
        return get_bit(config_read_u8(bus, device, function, 0x0e), 7);
    }

    /**
     * Read I/O port written in a Base Address Regiter.
     * If the BAR contains a memory address, the returned value is undefined.
     * Does not work for PCI-to-CardBus.
     *
     * offset - 0..1 for PCI-to-PCI bridges,
     *          0..5 for everything else.
     */
    uint32_t Function::get_bar_io(uint8_t offset) const {
        return config_read_u32(bus, device, function, 0x10 + 4 * offset) & 0xffff'fffc;
    }

    /// For PCI-to-PCI bridges only.
    uint8_t Function::get_secondary_bus() const {
        return config_read_u8(bus, device, function, 0x19);
    }

    static void check_bus(uint8_t bus);

    static void check_function(Function func) {
        if (!functions.push_back(func)) {
            LOG_ERROR("Two many connected PCI device functions. The kernel supports up to {}.",
                functions.get_capacity());
        }

        if (func.get_full_class() == 0x0604) {
            uint8_t secondary_bus = func.get_secondary_bus();
            check_bus(secondary_bus);
        }
    }

    static void check_device(uint8_t bus, uint8_t device) {
        Function dev(bus, device, 0);
        if (dev.get_vendor() == 0xffff) return;

        check_function(dev);
        if (dev.has_multiple_functions()) {
            for (uint8_t function = 1; function < 8; function++) {
                Function func(bus, device, function);
                if (func.get_vendor() != 0xffff) {
                    check_function(func);
                }
            }
        }
    }

    static void check_bus(uint8_t bus) {
        for (uint8_t device = 0; device < 32; device++) {
            check_device(bus, device);
        }
    }

    void init() {
        Function host_bridge(0, 0, 0);
        if (host_bridge.has_multiple_functions()) {
            for (uint8_t function = 0; function < 8; function++) {
                if (Function(0, 0, function).get_vendor() != 0xffff) {
                    break;
                }
                check_bus(function);
            }
        } else {
            check_bus(0);
        }
    }
}
