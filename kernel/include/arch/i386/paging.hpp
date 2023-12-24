#pragma once

#include <stddef.h>
#include <util/option.hpp>

namespace paging {
    static constexpr size_t PAGE_SIZE = 4096;

    using PhysAddr = size_t;

    using VirtAddr = size_t;

    void init();

    struct PageFlags {
        bool writable;
    };

    /**
     * Map the page containing `page` to the frame containing `frame`.
     * Return true on success, false on failure.
     */
    bool map(VirtAddr page, PhysAddr frame, PageFlags flags);

    /**
     * Unmap the page containing `page`.
     */
    void unmap(VirtAddr page);

    /**
     * Return true if the page containing the address is mapped.
     */
    bool is_mapped(VirtAddr address);

    Option<PhysAddr> translate(VirtAddr address);
}
