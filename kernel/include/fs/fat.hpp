#pragma once

#include <disk/disk.hpp>
#include <util/option.hpp>

namespace fat {
    bool try_read(const IDisk& disk);
}
