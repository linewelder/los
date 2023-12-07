#include <stdint.h>

#include <arch/i386/asm.hpp>
#include <arch/i386/gdt.hpp>
#include <arch/i386/idt.hpp>
#include <arch/i386/exceptions.hpp>
#include <arch/i386/pic.hpp>
#include <arch/i386/ps2.hpp>
#include <arch/i386/keyboard.hpp>
#include <arch/i386/terminal.hpp>
#include <arch/i386/pci.hpp>
#include <arch/i386/ide.hpp>
#include <kernel/log.hpp>
#include <kernel/print.hpp>
#include <kernel/kpanic.hpp>
#include <kernel/multiboot.h>
#include <util/bits.hpp>

static uint32_t detect_available_ram(multiboot_info_t* multiboot_info) {
    bool mmap_valid = get_bit(multiboot_info->flags, 6);
    if (!mmap_valid) {
        kpanic("Invalid memory map");
    }

    uint32_t ram_available = 0;
    for (
        uint32_t offset = 0;
        offset < multiboot_info->mmap_length;
        offset += sizeof(multiboot_memory_map_t))
    {
        multiboot_memory_map_t* block =
            reinterpret_cast<multiboot_memory_map_t*>(
                multiboot_info->mmap_addr + offset);

        // Available RAM above 1M.
        if (block->type == MULTIBOOT_MEMORY_AVAILABLE && block->addr >= 0x10'0000) {
            ram_available += static_cast<uint32_t>(block->len);
        }
    }

    return ram_available;
}

extern "C" [[noreturn]]
void kmain(multiboot_info_t* multiboot_info, uint32_t magic) {
    terminal::clear();

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        kpanic("Invalid Multiboot magic number");
    }

    uint32_t ram_available = detect_available_ram(multiboot_info);
    println("Los ({}MB RAM Available)", ram_available / 1024 / 1024);

    gdt::init();
    idt::init();
    register_exception_handlers();

    LOG_INFO("Initializing PIC...");
    pic::init();

    LOG_INFO("Initializing the PS/2 controller...");
    ps2::init();

    println("Connected PS/2 devices:");
    for (size_t i = 0; i < ps2::get_device_count(); i++) {
        const ps2::Device& device = ps2::get_device(i);
        println("  - {} (type: {:x})",
            device.get_type_name(), device.get_type());
    }

    LOG_INFO("Detecting connected PCI devices...");
    pci::init();

    terminal::write_cstr("Connected PCI devices:\n");
    for (size_t i = 0; i < pci::get_function_count(); i++) {
        const pci::Function& func = pci::get_function(i);
        println(
            "  {}:{}.{} Class: {:x} Vendor: {:x} Device: {:x}",
            func.get_bus(), func.get_device(), func.get_function(),
            func.get_full_class(),
            func.get_vendor(), func.get_device_id());
    }

    Option<const pci::Function&> ide_controller =
        pci::find_function_with_class(0x0101);
    if (ide_controller.has_value()) {
        ide::init(ide_controller.get_value());
    } else {
        LOG_ERROR("No IDE controller");
    }

    println("Connected disks:");
    for (size_t i = 0; i < ide::get_disk_count(); i++) {
        const ide::Device& disk = ide::get_disk(i);
        println("  - {} ({} Kb) Inteface: {}",
            disk.model, disk.size / 2,
            (const char*[]){ "ATA", "ATAPI" }[static_cast<int>(disk.interface)]);
    };

    Option<const ps2::Device&> keyboard = ps2::find_device_with_type(0xab83);
    if (keyboard.has_value()) {
        keyboard->set_interrupt_handler(keyboard::irq_handler);
        keyboard::set_callback([](keyboard::KeyEventArgs args) {
            if (!args.released && args.character) {
                terminal::putchar(args.character);
            }
        });

        keyboard->enable_scanning();
        pic::clear_mask(1);
        enable_interrupts();

        println("\nYou can type\n");
    } else {
        println("\nNo keyboard.");
    }
    for (;;) hlt();
}
