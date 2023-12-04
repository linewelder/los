#include <arch/i386/gdt.h>

#include <util/bits.h>

namespace gdt {
    static EncodedGdtEntry gdt[3];

    static void encode_gdt_entry(EncodedGdtEntry& target, GdtEntry desc) {
        target.data[0] = get_bit_range(desc.limit, 0, 8);
        target.data[1] = get_bit_range(desc.limit, 8, 8);
        target.data[6] = get_bit_range(desc.limit, 16, 4);

        target.data[2] = get_bit_range(desc.base, 0, 8);
        target.data[3] = get_bit_range(desc.base, 8, 8);
        target.data[4] = get_bit_range(desc.base, 16, 8);
        target.data[7] = get_bit_range(desc.base, 24, 8);

        target.data[5] = desc.access_byte;

        target.data[6] |= (desc.flags << 4);
    }

    static void fill_gdt() {
        encode_gdt_entry(gdt[0], {}); // Null descriptor.
        encode_gdt_entry(gdt[1], { // Kernel code segment.
            .base = 0x0,
            .limit = 0xfffff,
            .access_byte = ACCESS_VALID | ACCESS_CODE_DATA |
                           ACCESS_EXECUTABLE | ACCESS_READABLE,
            .flags = FLAG_LIMIT_IN_PAGES | FLAG_32_BIT,
        });
        encode_gdt_entry(gdt[2], { // Kernel data segment.
            .base = 0x0,
            .limit = 0xfffff,
            .access_byte = ACCESS_VALID | ACCESS_CODE_DATA |
                           ACCESS_WRITABLE,
            .flags = FLAG_LIMIT_IN_PAGES | FLAG_32_BIT,
        });
    }

    static struct __attribute__((packed)) {
        uint16_t size;
        uint32_t base;
    } gdtr;

    void init() {
        fill_gdt();

        gdtr.size = sizeof(gdt);
        gdtr.base = reinterpret_cast<uint32_t>(&gdt);
        asm volatile("lgdt %0" : : "m"(gdtr));

        // Reload segment registers.
        asm volatile(
            "jmp $0x08, $.reload_cs\n"
            ".reload_cs:\n\t"
            "mov $0x10, %ax\n\t"
            "mov %ax, %ds\n\t"
            "mov %ax, %es\n\t"
            "mov %ax, %fs\n\t"
            "mov %ax, %gs\n\t"
            "mov %ax, %ss");
    }
}
