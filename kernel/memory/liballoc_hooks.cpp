/**
 * @file
 * Heap page management (liballoc hooks' implementation).
 */

#include <arch/i386/paging.hpp>
#include <kernel/log.hpp>
#include <memory/frame_allocator.hpp>

/** This function is supposed to lock the memory data structures. It
 * could be as simple as disabling interrupts or acquiring a spinlock.
 * It's up to you to decide. 
 *
 * \return 0 if the lock was acquired successfully. Anything else is
 * failure.
 */
extern "C" int liballoc_lock() {
    return 0;
}

/** This function unlocks what was previously locked by the liballoc_lock
 * function.  If it disabled interrupts, it enables interrupts. If it
 * had acquiried a spinlock, it releases the spinlock. etc.
 *
 * \return 0 if the lock was successfully released.
 */
extern "C" int liballoc_unlock() {
    return 0;
}

constexpr paging::VirtAddr heap_start = 0x40'0000;
constexpr paging::VirtAddr heap_end = 0x50'0000;

static Option<paging::VirtAddr> find_unused_region(size_t pages) {
    size_t region_size = pages * paging::PAGE_SIZE;
    size_t contiguous_free_pages = 0;

    for (paging::VirtAddr addr = heap_start;
         addr < heap_end;
         addr += paging::PAGE_SIZE)
    {
        if (paging::is_mapped(addr)) {
            contiguous_free_pages = 0;
            continue;
        }

        contiguous_free_pages++;
        if (contiguous_free_pages == pages) {
            return addr + paging::PAGE_SIZE - region_size;
        }
    }

    return {};
}

/** This is the hook into the local system which allocates pages. It
 * accepts an integer parameter which is the number of pages
 * required.  The page size was set up in the liballoc_init function.
 *
 * \return NULL if the pages were not allocated.
 * \return A pointer to the allocated memory.
 */
extern "C" void* liballoc_alloc(size_t pages) {
    auto region_size = pages * paging::PAGE_SIZE;

    auto maybe_region = find_unused_region(pages);
    if (!maybe_region.has_value()) [[unlikely]] {
        LOG_ERROR("Failed to allocate {} pages for the heap.", pages);
        return nullptr;
    }
    auto region = maybe_region.get_value();

    for (size_t offset = 0; offset < region_size; offset += paging::PAGE_SIZE) {
        auto maybe_frame = frame_allocator::allocate_frame();
        if (!maybe_frame.has_value()) [[unlikely]] {
            LOG_ERROR("Failed to allocate {} page frames for the heap (managed to do {}).",
                pages, offset / paging::PAGE_SIZE);
            return nullptr;
        }

        paging::map(
            region + offset,
            maybe_frame.get_value(),
            paging::PageFlags{ .writable = true });
    }

    return reinterpret_cast<void*>(region);
}

/** This frees previously allocated memory. The void* parameter passed
 * to the function is the exact same value returned from a previous
 * liballoc_alloc call.
 *
 * The integer value is the number of pages to free.
 *
 * \return 0 if the memory was successfully freed.
 */
extern "C" int liballoc_free(void* start, size_t pages) {
    auto start_addr = reinterpret_cast<paging::VirtAddr>(start);
    for (size_t i = 0; i < pages; i++) {
        paging::unmap(start_addr + i * paging::PAGE_SIZE);
    }

    return 0;
}
