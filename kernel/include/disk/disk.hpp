#pragma once

#include <stdint.h>
#include <util/span.hpp>

/**
 * Generic disk.
 */
class IDisk {
public:
    /**
     * Buffer must be at least 512 * sector_count bytes long.
     */
    virtual bool read(uint64_t lba, uint8_t sector_count, Span<uint8_t> buffer) const = 0;

    /**
     * Buffer must be at least 512 * sector_count bytes long.
     */
    virtual bool write(uint64_t lba, uint8_t sector_count, Span<uint8_t> buffer) const = 0;
};
