#pragma once

#include <stdint.h>

struct IDiskVmt {
    bool (*read)(const void* ptr, uint64_t lba, uint8_t sector_count, void* buffer);
    bool (*write)(const void* ptr, uint64_t lba, uint8_t sector_count, void* buffer);
};

/**
 * Generic disk.
 *
 * Since virtual methods (and thus interfaces) require RTTI in C++ and
 * RTTI requires libsupc++, manually reinventing the wheel allows us to
 * avoid all of that.
 */
struct IDisk {
public:
    constexpr IDisk(const void* ptr, const IDiskVmt* vmt) : ptr(ptr), vmt(vmt) {}

    inline bool read(uint64_t lba, uint8_t sector_count, void* buffer) const {
        return vmt->read(ptr, lba, sector_count, buffer);
    }

    inline bool write(uint64_t lba, uint8_t sector_count, void* buffer) const {
        return vmt->write(ptr, lba, sector_count, buffer);
    }

private:
    const void* ptr;
    const IDiskVmt* vmt;
};
