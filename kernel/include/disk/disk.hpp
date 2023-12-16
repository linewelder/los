#pragma once

#include <stdint.h>
#include <util/span.hpp>

/**
 * Generic disk.
 */
class IDisk {
public:
    /**
     * Read sectors to the buffer until we are left without
     * space for a whole sector.
     * Maximum 255 sectors.
     */
    virtual bool read(uint64_t lba, Span<uint8_t> buffer) const = 0;

    /**
     * Write whole sectors from the buffer (if the buffer
     * has extra bytes in the end they are ignored).
     * Maximum 255 sectors.
     */
    virtual bool write(uint64_t lba, Span<uint8_t> buffer) const = 0;
};
