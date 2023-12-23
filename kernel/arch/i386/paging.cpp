/**
 * @file
 *
 * OSDev Wiki: https://wiki.osdev.org/Paging
 * Written with: https://os.phil-opp.com/paging-implementation/
 */

#include <arch/i386/paging.hpp>

#include <stdint.h>
#include <arch/i386/asm.hpp>
#include <kernel/kpanic.hpp>
#include <memory/frame_allocator.hpp>
#include <util/assert.hpp>
#include <util/array.hpp>
#include <util/bits.hpp>
#include <util/memory.hpp>

namespace paging {
    class PageTable;

    class PageTableEntry {
    public:
        PageTableEntry() = default;

        PageTableEntry(const PageTableEntry& other) = delete;
        PageTableEntry& operator=(const PageTableEntry& other) = delete;

        PageTableEntry(PageTableEntry&& other) = delete;
        PageTableEntry& operator=(PageTableEntry&& other) = delete;

        ~PageTableEntry() = default;

        void map(PhysAddr addr, bool writeEnabled) {
            uint32_t desc = addr & 0xffff'f000;
            desc = set_bit(desc, 0); // Present.
            if (writeEnabled) {
                desc = set_bit(desc, 1);
            }

            inner = desc;
        }

        void map(const PageTable& table, bool writeEnabled) {
            map(reinterpret_cast<PhysAddr>(&table), writeEnabled);
        }

        void unmap() {
            inner = 0;
        }

        bool is_unused() const {
            return !get_bit(inner, 0);
        }

    private:
        uint32_t inner = 0;
    };

    class PageTable {
    public:
        static PageTable& allocate() {
            auto maybe = frame_allocator::allocate_frame();
            if (!maybe.has_value()) {
                kpanic("Failed to allocate memory for a page table.");
            }

            PageTable* pointer = reinterpret_cast<PageTable*>(maybe.get_value());
            new (pointer) PageTable();
            return *pointer;
        }

        PageTable(const PageTable& other) = delete;
        PageTable& operator=(const PageTable& other) = delete;

        PageTable(PageTable&& other) = delete;
        PageTable& operator=(PageTable&& other) = delete;

        ~PageTable() = default;

        inline PageTableEntry& operator[](size_t index) {
            return inner[index];
        }

        inline const PageTableEntry& operator[](size_t index) const {
            return inner[index];
        }

        /**
         * Only for page table directories.
         */
        void use() {
            write_cr3(reinterpret_cast<uint32_t>(this));
        }

    private:
        PageTable() {
            ASSERT(reinterpret_cast<size_t>(this) % PAGE_SIZE == 0);
        }

        Array<PageTableEntry, 1024> inner;
    };

    static void enable_paging() {
        asm volatile(
            "mov %cr0, %eax\n\t"
            "or $0x80000000, %eax\n\t"
            "mov %eax, %cr0");
    }

    void init() {
        PageTable& page_directory = PageTable::allocate();
        PageTable& kernel_page_table = PageTable::allocate();

        // Identity map the first 4MiB.
        for (int i = 0; i < 1024; i++) {
            kernel_page_table[i].map(i * 0x1000, true);
        }
        page_directory[0].map(kernel_page_table, true);

        page_directory.use();
        enable_paging();
    }
}
