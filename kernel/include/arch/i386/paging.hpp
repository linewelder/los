#pragma once

#include <stddef.h>

namespace paging {
    static constexpr size_t PAGE_SIZE = 4096;

    using PhysAddr = size_t;

    using VirtAddr = size_t;
}
