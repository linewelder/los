#pragma once

#include <disk/disk.hpp>
#include <util/array.hpp>
#include <util/option.hpp>
#include <util/string.hpp>
#include <util/vector.hpp>

namespace fat {
    enum class FatType {
        FAT12,
        FAT16,
        FAT32
    };

    struct DirEntry {
        String name;
        bool is_directory;
    };

    class FatFS {
    public:
        static Option<FatFS> try_read(const IDisk& disk);

        Option<Vector<DirEntry>> list_root() const;

    private:
        FatFS(const IDisk& disk);

        uint32_t first_sector_of(uint32_t cluster) const;

        bool read_files_from_sector(uint32_t sector, Vector<DirEntry>& list) const;

        const IDisk& disk;
        FatType type;
        uint32_t first_data_sector;
        uint8_t sectors_per_cluster;
        uint32_t first_root_sector;
    };
}
