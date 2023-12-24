#pragma once

#include <stddef.h>
#include <arch/i386/paging.hpp>
#include <kernel/multiboot.h>
#include <util/option.hpp>

namespace frame_allocator {
    void init(const multiboot_info_t& multiboot_info);

    Option<paging::PhysAddr> allocate_frame();

    size_t get_total_memory();

    size_t get_available_memory();
}
