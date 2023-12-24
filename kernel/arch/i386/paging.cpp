/**
 * @file
 *
 * We assume that all page tables and directories are identity mapped.
 *
 * OSDev Wiki: https://wiki.osdev.org/Paging
 * Written with: https://os.phil-opp.com/paging-implementation/
 */

#include <arch/i386/paging.hpp>

#include <stdint.h>
#include <arch/i386/asm.hpp>
#include <kernel/kpanic.hpp>
#include <kernel/log.hpp>
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

        void map(PhysAddr addr, PageFlags flags) {
            uint32_t desc = addr & 0xffff'f000;
            desc = set_bit(desc, 0); // Present.
            if (flags.writable) {
                desc = set_bit(desc, 1);
            }

            inner = desc;
        }

        void map(const PageTable& table, PageFlags flags) {
            map(reinterpret_cast<PhysAddr>(&table), flags);
        }

        void unmap() {
            inner = 0;
        }

        Option<PhysAddr> get_addr() const {
            if (is_unused()) return {};
            return inner & 0xffff'f000;
        }

        /**
         * Only for page table directory entries.
         */
        Option<PageTable&> get_table() const {
            if (is_unused()) return {};
            return *reinterpret_cast<PageTable*>(inner & 0xffff'f000);
        }

        bool is_unused() const {
            return !get_bit(inner, 0);
        }

    private:
        uint32_t inner = 0;
    };

    class PageTable {
    public:
        static Option<PageTable&> try_allocate() {
            auto maybe = frame_allocator::allocate_frame();
            if (!maybe.has_value()) {
                return {};
            }

            PageTable* pointer = reinterpret_cast<PageTable*>(maybe.get_value());
            new (pointer) PageTable();
            return *pointer;
        }

        static PageTable& allocate() {
            auto maybe = try_allocate();
            if (!maybe.has_value()) {
                kpanic("Failed to allocate memory for a page table.");
            }
            return maybe.get_value();
        }

        static PageTable& get_active_page_dir() {
            return *reinterpret_cast<PageTable*>(read_cr3());
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
            kernel_page_table[i].map(i * 0x1000, PageFlags{ .writable = true });
        }
        page_directory[0].map(kernel_page_table, PageFlags{ .writable = true });

        page_directory.use();
        enable_paging();
    }

    Option<PhysAddr> translate(VirtAddr address) {
        auto& page_directory = PageTable::get_active_page_dir();

        auto dir_index = get_bit_range(address, 22, 10);
        auto page_table = page_directory[dir_index].get_table();
        if (!page_table.has_value()) {
            LOG_INFO("Table {} not maped", dir_index);
            return {};
        }

        auto table_index = get_bit_range(address, 12, 10);
        auto frame_start = page_table.get_value()[table_index].get_addr();
        if (!frame_start.has_value()) {
            LOG_INFO("Page {} not maped", table_index);
            return {};
        }

        auto offset = get_bit_range(address, 0, 12);
        return frame_start.get_value() + offset;
    }

    bool map(VirtAddr page, PhysAddr frame, PageFlags flags) {
        auto& page_directory = PageTable::get_active_page_dir();

        auto dir_index = get_bit_range(page, 22, 10);
        auto page_table = page_directory[dir_index].get_table();
        if (!page_table.has_value()) {
            LOG_INFO(
                "Mapping page table {}.",
                dir_index);

            page_table = PageTable::try_allocate();
            if (!page_table.has_value()) return false;

            page_directory[dir_index]
                .map(page_table.get_value(), PageFlags{ .writable = true });
        }

        auto table_index = get_bit_range(page, 12, 10);
        auto& entry = page_table.get_value()[table_index];
        if (!entry.is_unused()) {
            LOG_WARN("Mapping page {:p} that is already mapped to {:p}.",
                page & 0xffff'f000, entry.get_addr().get_value());
        }

        entry.map(frame, flags);
        return true;
    }
}
