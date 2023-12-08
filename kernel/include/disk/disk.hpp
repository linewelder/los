#pragma once

#include <stdint.h>

/**
 * Generic disk.
 */
class IDisk {
public:
    virtual bool read(uint64_t lba, uint8_t sector_count, void* buffer) const = 0;
    virtual bool write(uint64_t lba, uint8_t sector_count, void* buffer) const = 0;
};
