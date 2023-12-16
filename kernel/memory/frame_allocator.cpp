#include <memory/frame_allocator.hpp>

#include <kernel/kpanic.hpp>
#include <kernel/log.hpp>
#include <memory/kernel_location.hpp>
#include <util/bits.hpp>
#include <util/span.hpp>
#include <util/math.hpp>

namespace frame_allocator {
    struct MemoryRegion {
        paging::PhysAddr start;
        size_t length;

        /**
         * Replace this with the intersection between this and bounds.
         */
        void clamp(MemoryRegion bounds) {
            paging::PhysAddr new_end = min(
                start + length,
                bounds.start + bounds.length);
            start = max(start, bounds.start);
            length = new_end > start
                ? new_end - start
                : 0;
        }
    };

    // We are not in Long Mode.
    constexpr paging::PhysAddr MAX_ADDRESS = 0xffff'ffff;

    static paging::PhysAddr first_frame;
    static size_t frame_capacity;
    static size_t frame_count;

    void init(multiboot_info_t* info) {
        bool mmap_valid = get_bit(info->flags, 6);
        if (!mmap_valid) {
            kpanic("Invalid memory map");
        }

        paging::PhysAddr kernel_end_addr =
            reinterpret_cast<paging::PhysAddr>(&kernel_end);
        MemoryRegion available { kernel_end_addr, MAX_ADDRESS - kernel_end_addr };

        Span<const multiboot_memory_map_t> mmap{
            .start = reinterpret_cast<const multiboot_memory_map_t*>(info->mmap_addr),
            .size = info->mmap_length / sizeof(multiboot_memory_map_t),
        };

        for (const auto& region : mmap) {
            if (region.type != MULTIBOOT_MEMORY_AVAILABLE) {
                continue;
            }

            if (region.addr > MAX_ADDRESS) {
                LOG_WARN("Available memory above 4GiB detected. It will not be used.");
                continue;
            }
            size_t region_start = static_cast<size_t>(region.addr);

            uint64_t region_end = region.addr + region.len;
            size_t region_len = region.len;
            if (region_end > MAX_ADDRESS) {
                LOG_WARN("Available memory above 4GiB detected. It will not be used.");
                region_len = MAX_ADDRESS - region.addr;
            }

            MemoryRegion current { region_start, region_len };

            current.clamp(available);
            if (current.length == 0) {
                continue;
            }

            first_frame = current.start;
            frame_capacity = current.length / paging::PAGE_SIZE;
            frame_count = 0;
            return;
        }

        kpanic("No available memory");
    }

    size_t get_total_memory() {
        return frame_capacity * paging::PAGE_SIZE;
    }

    size_t get_available_memory() {
        return (frame_capacity - frame_count) * paging::PAGE_SIZE;
    }

    Option<paging::PhysAddr> allocate_frame() {
        if (frame_count >= frame_capacity) {
            return {};
        }

        paging::PhysAddr frame = first_frame + frame_count * paging::PAGE_SIZE;
        frame_count++;
        return frame;
    }
}
