#include <stdint.h>

#include <arch/i386/asm.h>
#include <arch/i386/gdt.h>
#include <arch/i386/idt.h>
#include <arch/i386/exceptions.h>
#include <arch/i386/pic.h>
#include <arch/i386/ps2.h>
#include <arch/i386/keyboard.h>
#include <arch/i386/terminal.h>
#include <arch/i386/pci.h>
#include <arch/i386/ide.h>
#include <kernel/log.h>
#include <kernel/printf.h>
#include <kernel/kpanic.h>
#include <kernel/multiboot.h>
#include <util/bits.h>

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
            (multiboot_memory_map_t*)(multiboot_info->mmap_addr + offset);

        // Available RAM above 1M.
        if (block->type == MULTIBOOT_MEMORY_AVAILABLE && block->addr >= 0x10'0000) {
            ram_available += (uint32_t)block->len;
        }
    }

    return ram_available;
}

extern "C"
void kmain(multiboot_info_t* multiboot_info, uint32_t magic) {
    terminal::clear();

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        kpanic("Invalid Multiboot magic number");
    }

    uint32_t ram_available = detect_available_ram(multiboot_info);
    printf("Los (%dMB RAM Available)\n", ram_available / 1024 / 1024);

    gdt::init();
    idt::init();
    register_exception_handlers();

    LOG_INFO("Initializing PIC...");
    pic::init();

    LOG_INFO("Initializing the PS/2 controller...");
    ps2::init();

    terminal::write_cstr("Connected PS/2 devices:\n");
    for (size_t i = 0; i < ps2::get_device_count(); i++) {
        const ps2::Device& device = ps2::get_device(i);
        printf("  - %s (type: %x)\n",
            device.get_type_name(), device.get_type());
    }

    LOG_INFO("Detecting connected PCI devices...");
    pci::init();

    terminal::write_cstr("Connected PCI devices:\n");
    for (size_t i = 0; i < pci::get_function_count(); i++) {
        const pci::Function& func = pci::get_function(i);
        printf(
            "  %d:%d.%d Class: %x Vendor: %x Device: %x\n",
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

    terminal::write_cstr("Connected disks:\n");
    for (size_t i = 0; i < ide::get_disk_count(); i++) {
        const ide::Device& disk = ide::get_disk(i);
        printf("  - %s (%d Kb) Inteface: %s\n",
            disk.model, disk.size / 2,
            (const char*[]){ "ATA", "ATAPI" }[(int)disk.interface]);
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

        terminal::write_cstr("\nYou can type\n\n");
    } else {
        terminal::write_cstr("\nNo keyboard.");
    }
    for (;;) hlt();
}
