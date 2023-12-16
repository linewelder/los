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
#include <memory/frame_allocator.hpp>

extern "C" [[noreturn]]
void kmain(multiboot_info_t* multiboot_info, uint32_t magic) {
    terminal::clear();

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        kpanic("Invalid Multiboot magic number");
    }

    gdt::init();
    idt::init();
    register_exception_handlers();

    frame_allocator::init(multiboot_info);
    println("Los ({}MiB RAM Available)",
        frame_allocator::get_total_memory() / 1024 / 1024);

    LOG_INFO("Initializing PIC...");
    pic::init();

    LOG_INFO("Initializing the PS/2 controller...");
    ps2::init();

    println("Connected PS/2 devices:");
    for (const auto& device : ps2::get_devices()) {
        println("  - {} (type: {:x})",
            device.get_type_name(), device.get_type());
    }

    LOG_INFO("Detecting connected PCI devices...");
    pci::init();

    terminal::write_cstr("Connected PCI devices:\n");
    for (const auto& func : pci::get_functions()) {
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
    for (const auto& disk : ide::get_disks()) {
        auto get_interface_name = [](ide::InterfaceType interface) {
            switch (interface) {
                case ide::InterfaceType::ATA: return "ATA";
                case ide::InterfaceType::ATAPI: return "ATAPI";
                default: return "<unkown>";
            }
        };

        println("  - {} ({} Kb) Inteface: {}",
            disk.model, disk.size / 2,
            get_interface_name(disk.interface));
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
