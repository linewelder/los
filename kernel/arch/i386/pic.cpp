#include <arch/i386/pic.h>

#include <stdint.h>
#include <arch/i386/asm.h>
#include <util/bits.h>

namespace pic {
    static constexpr uint8_t PIC1_VECTOR_OFFSET = 0x20;
    static constexpr uint8_t PIC2_VECTOR_OFFSET = 0x28;

    static constexpr uint16_t PIC1_COMMAND_PORT = 0x20;
    static constexpr uint16_t PIC1_DATA_PORT = 0x21;
    static constexpr uint16_t PIC2_COMMAND_PORT = 0xa0;
    static constexpr uint16_t PIC2_DATA_PORT = 0xa1;

    static constexpr uint8_t ICW1_ICW4 = 0x01; // Start initialization with 3 more comands to come.
    static constexpr uint8_t ICW1_SINGLE = 0x02; // Single (cascade) mode.
    static constexpr uint8_t ICW1_INTERVAL4 = 0x04; // Call address interval 4 (8).
    static constexpr uint8_t ICW1_LEVEL = 0x08; // Level triggered (edge) mode.
    static constexpr uint8_t ICW1_INIT = 0x10; // Initialization sequence marker (must be set).

    static constexpr uint8_t ICW4_USE_8086_MODE = 0x01; // 8086/88 (MCS-80/85) mode.
    static constexpr uint8_t ICW4_EOI_AUTO = 0x02; // Auto (normal) EOI.
    static constexpr uint8_t ICW4_BUF_SLAVE = 0x08; // Buffered mode (for slave).
    static constexpr uint8_t ICW4_BUF_MASTER = 0x0C; // Buffered mode (for master).
    static constexpr uint8_t ICW4_SFNM = 0x10; // Special fully nested (not).

    static constexpr uint8_t END_OF_INTERRUPT = 0x20;

    void init() {
        // Start the initialization sequence in cascade mode.
        outb(PIC1_COMMAND_PORT, ICW1_INIT | ICW1_ICW4);
        io_wait();
        outb(PIC2_COMMAND_PORT, ICW1_INIT | ICW1_ICW4);
        io_wait();

        // ICW2: Set up vector offsets.
        outb(PIC1_DATA_PORT, PIC1_VECTOR_OFFSET);
        io_wait();
        outb(PIC2_DATA_PORT, PIC2_VECTOR_OFFSET);
        io_wait();

        // ICW3:
        // Tell Master PIC that there is a slave PIC at IRQ2.
        outb(PIC1_DATA_PORT, 1 << 2);
        io_wait();
        // Tell slave PIC its cascade identity (0000 0010)
        outb(PIC2_DATA_PORT, 2);
        io_wait();

        // ICW4: Use 8086 mode (not 8080 mode).
        outb(PIC1_DATA_PORT, ICW4_USE_8086_MODE);
        io_wait();
        outb(PIC2_DATA_PORT, ICW4_USE_8086_MODE);
        io_wait();

        // Mask all interrupts.
        outb(PIC1_DATA_PORT, 0xff);
        outb(PIC2_DATA_PORT, 0xff);
    }

    void set_mask(uint8_t irq) {
        uint16_t port;
        if(irq < 8) {
            port = PIC1_DATA_PORT;
        } else {
            port = PIC2_DATA_PORT;
            irq -= 8;
        }
        uint8_t value = set_bit(inb(port), irq);
        outb(port, value);
    }
    
    void clear_mask(uint8_t irq) {
        uint16_t port;
        if(irq < 8) {
            port = PIC1_DATA_PORT;
        } else {
            port = PIC2_DATA_PORT;
            irq -= 8;
        }
        uint8_t value = unset_bit(inb(port), irq);
        outb(port, value);
    }

    void send_eoi(uint8_t irq) {
        if (irq >= 8) {
            outb(PIC2_COMMAND_PORT, END_OF_INTERRUPT);
        }

        outb(PIC1_COMMAND_PORT, END_OF_INTERRUPT);
    }

    uint8_t get_interrupt_vector(uint8_t irq) {
        if (irq < 8) {
            return PIC1_VECTOR_OFFSET + irq;
        } else {
            return PIC2_VECTOR_OFFSET + irq - 8;
        }
    }
}
