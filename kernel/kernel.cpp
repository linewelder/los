#include <stdint.h>

#include "asm.h"
#include "gdt.h"
#include "idt.h"
#include "exceptions.h"
#include "pic.h"
#include "ps2.h"
#include "keyboard.h"
#include "terminal.h"
#include "printf.h"

extern "C" void kmain() {
    terminal::clear();
    terminal::write_cstr("Los\n");

    terminal::write_cstr("Initializing the GDT...\n");
    gdt::init();

    terminal::write_cstr("Initializing the IDT...\n");
    idt::init();
    register_exception_handlers();

    terminal::write_cstr("Initializing PIC...\n");
    pic::init();

    terminal::write_cstr("Initializing the PS/2 controller...\n");
    ps2::init();

    bool keyboard_found = false;
    terminal::write_cstr("\nConnected PS/2 devices:\n");
    for (int i = 0; i < ps2::get_device_count(); i++) {
        const ps2::Device& device = ps2::get_device(i);
        printf("- %s (type: %x)",
            device.get_type_name(), device.get_type());

        if (!keyboard_found && device.get_type() == 0xab83) {
            device.enable_scanning();
            idt::register_interrupt(i == 0 ? 
                0x21 : 0x2c, keyboard::irq_handler);
            keyboard_found = true;
            terminal::write_cstr(" [Primary keyboard]");
        }

        terminal::putchar('\n');
    }
    if (!keyboard_found) kpanic("No keyboard");

    keyboard::set_callback([](keyboard::KeyEventArgs args) {
        if (!args.released && args.character) {
            terminal::putchar(args.character);
        }
    });
    pic::clear_mask(1);
    enable_interrupts();

    terminal::write_cstr("\nYou can type\n\n");
    for (;;) {
        asm volatile("hlt");
    }
}
