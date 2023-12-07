#pragma once

#include <arch/i386/asm.h>
#include <arch/i386/terminal.h>
#include <kernel/print.h>

template <typename... Args>
[[noreturn]]
void kpanic(FormatString<Args...> format, const Args&... args) {
    terminal::set_color(terminal::Color::WHITE, terminal::Color::RED);
    terminal::write_cstr("Kernel panic! ");
    println(format, args...);
    disable_interrupts();
    for (;;) hlt();
}
