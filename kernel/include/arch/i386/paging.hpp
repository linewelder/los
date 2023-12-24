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

    Option<PhysAddr> translate(VirtAddr address);
}
