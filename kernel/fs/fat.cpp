#include <fs/fat.hpp>

#include <stdint.h>
#include <disk/disk.hpp>
#include <kernel/log.hpp>
#include <util/array.hpp>
#include <util/enum_flags.hpp>

namespace fat {
    struct [[gnu::packed]] FatHeader16 {
        uint8_t drive_number; //< 0x00 for floppy disks, 0x80 for hard disks.
        uint8_t reserved_0; //< Windows NT flags.
        uint8_t signature; //< Must be 0x28 or 0x29.
        uint32_t volume_id;
        Array<char, 11> volume_label; //< Padded with spaces.
        Array<char, 8> fat_type_label; //< System identifier string, not trustable.
    };
    static_assert(sizeof(FatHeader16) == 26);

    struct [[gnu::packed]] FatHeader32 {
        uint32_t sectors_per_fat;
        uint16_t flags;
        uint16_t version;
        uint32_t root_cluster;
        uint16_t fat_info_sector;
        uint16_t backup_boot_sector;
        Array<uint8_t, 12> reserved_0; //< Should be zero.
        uint8_t drive_number; //< 0x00 for floppy disks, 0x80 for hard disks.
        uint8_t reserved_1; //< Windows NT flags.
        uint8_t signature; //< Must be 0x28 or 0x29.
        uint32_t volume_id;
        Array<char, 11> volume_label; //< Padded with spaces.
        Array<char, 8> fat_type_label; //< Always "FAT32   ", not trustable.
    };
    static_assert(sizeof(FatHeader32) == 54);

    struct [[gnu::packed]] FatBootRecord {
        Array<uint8_t, 3> boot_jump;
        Array<char, 8> oem_name; //< The DOS version.
        uint16_t bytes_per_sector;
        uint8_t sectors_per_cluster;
        uint16_t reserved_sector_count;
        uint8_t fat_count; //< Number of File Allocation Tables.
        uint16_t root_entry_count; //< Must be set so that the root directory occupies entire sectors.
        uint16_t total_sectors_16; //< If zero, number of sectors is > 65535.
        uint8_t media_type; //< https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system#BPB20_OFS_0Ah
        uint16_t sectors_per_fat; //< FAT12/FAT16 only.
        uint16_t sectors_per_track;
        uint16_t head_side_count;
        uint32_t hidden_sector_count; //< i.e. the LBA of the beginning of the partition.
        uint32_t total_sectors_32; //< Set if total_sectors_16 == 0.
        union {
            FatHeader16 fat16;
            FatHeader32 fat32;
        };
    };
    static_assert(sizeof(FatBootRecord) == 36 + sizeof(FatHeader32));

    enum class FileAttr : uint8_t {
        READ_ONLY = 0x01,
        HIDDEN = 0x02,
        SYSTEM = 0x04,
        VOLUME_ID = 0x08,
        DIRECTORY = 0x10,
        ARCHIVE = 0x20,

        /** The entry is a LongNameEntry*/
        LONG_NAME = 0x0f
    };
    ENUM_FLAGS(FileAttr)

    struct TimeStamp {
        uint16_t value : 5;
        uint16_t minutes : 6;
        uint16_t seconds : 5;
    };

    struct DateStamp {
        uint16_t year : 7;
        uint16_t month : 4;
        uint16_t day : 5;
    };

    struct [[gnu::packed]] FileEntry {
        Array<char, 8> name;
        Array<char, 3> extension;
        FileAttr attributes;
        uint8_t reserved; // For Windows NT.
        uint8_t creation_time_tenths_of_sec; //< Range 0..199 inclusive.
        TimeStamp creation_time;
        DateStamp creation_date;
        DateStamp last_accessed_date;
        uint16_t first_cluster_high;
        TimeStamp last_modified_time;
        DateStamp last_modified_date;
        uint16_t first_cluster_low;
        uint32_t file_size; //< In bytes.
    };
    static_assert(sizeof(FileEntry) == 32);

    struct [[gnu::packed]] LongNameEntry {
        uint8_t order;
        Array<uint16_t, 5> name_0;
        FileAttr attributes; //< Always FileAttr::LONG_NAME.
        uint8_t long_entry_type; //< Zero.
        uint8_t checksum;
        Array<uint16_t, 6> name_1;
        uint16_t zero;
        Array<uint16_t, 2> name_2;
    };
    static_assert(sizeof(LongNameEntry) == 32);

    Option<FatFS> FatFS::try_read(const IDisk& disk) {
        Array<uint8_t, 512> boot_sector;
        if (!disk.read(0, boot_sector)) {
            LOG_ERROR("Failed to read boot sector.");
            return {};
        }

        const auto& header = *reinterpret_cast<const FatBootRecord*>(
            boot_sector.begin());

        auto total_sectors = header.total_sectors_16 == 0
            ? header.total_sectors_32
            : header.total_sectors_16;

        auto fat_size = header.sectors_per_fat == 0
            ? header.fat32.sectors_per_fat
            : header.sectors_per_fat;

        auto root_dir_sectors =
            ((header.root_entry_count * 32) + (header.bytes_per_sector - 1))
                / header.bytes_per_sector;

        auto first_data_sector = header.reserved_sector_count +
            (header.fat_count * fat_size) + root_dir_sectors;

        auto data_sectors = total_sectors - first_data_sector;

        auto total_clusters = data_sectors / header.sectors_per_cluster;

        FatType type;
        if (total_clusters < 4085) {
            type = FatType::FAT12;
        } else if (total_clusters < 65525) {
            type = FatType::FAT16;
        } else {
            type = FatType::FAT32;
        }

        FatFS fs(disk);
        fs.type = type;
        fs.first_data_sector = first_data_sector;
        fs.sectors_per_cluster = header.sectors_per_cluster;

        fs.first_root_sector = type == FatType::FAT32
            ? fs.first_sector_of(header.fat32.root_cluster)
            : first_data_sector - root_dir_sectors;

        return fs;
    }

    Option<Vector<DirEntry>> FatFS::list_root() const {
        Vector<DirEntry> list;
        if (!read_files_from_sector(first_root_sector, list)) {
            return {};
        }
        return list;
    }

    bool FatFS::read_files_from_sector(
        uint32_t sector, Vector<DirEntry>& list) const
    {
        Array<uint8_t, 512> data;
        if (!disk.read(sector, data)) return false;

        Span<const FileEntry> entries{
            reinterpret_cast<const FileEntry*>(data.begin()),
            16
        };

        struct LongFileNameBuffer {
            
        };

        for (const auto& entry : entries) {
            if (entry.name[0] == '\0') break; // Out of entries.
            if (entry.name[0] == '\xe5') continue; // Entry not used.

            if (has_flag(entry.attributes, FileAttr::VOLUME_ID)) {
                continue;
            }

            // We do not support long file names yet.
            if (entry.attributes == FileAttr::LONG_NAME) {
                continue;
            }

            DirEntry file;
            file.is_directory = has_flag(entry.attributes, FileAttr::DIRECTORY);

            for (auto ch : entry.name) {
                if (ch == ' ') break;
                file.name.push_back(ch);
            }

            if (entry.extension[0] != ' ') {
                file.name.push_back('.');
                for (auto ch : entry.extension) {
                    if (ch == ' ') break;
                    file.name.push_back(ch);
                }
            }

            list.push_back(move(file));
        }
        return true;
    }

    FatFS::FatFS(const IDisk& disk) : disk(disk) {}

    uint32_t FatFS::first_sector_of(uint32_t cluster) const {
        return ((cluster - 2) * sectors_per_cluster) + first_data_sector;
    }
}