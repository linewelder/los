#pragma once

#include <stdint.h>
#include <stddef.h>

#include <arch/i386/pci.hpp>
#include <disk/disk.hpp>
#include <util/array.hpp>
#include <util/span.hpp>
#include <util/string_view.hpp>

namespace ide {
    enum class ChannelType {
        PRIMARY,
        SECONDARY,
    };

    enum class DriveType {
        MASTER,
        SLAVE,
    };

    enum class InterfaceType {
        ATA,
        ATAPI,
    };

    enum class IdentifyResultStatus {
        Success,
        NoDevice,
        UnknownDeviceType,
        RequestError, // Details are in error_byte.
    };

    struct IdentifyResult {
        IdentifyResultStatus status;
        uint8_t error_byte; // Errors read from the error register.
    };

    enum class Direction {
        READ,
        WRITE,
    };

    enum class PollingResult {
        SUCCESS,
        ERROR,
        DRIVE_WRITE_FAULT,
        REQUEST_NOT_READY,
    };

    class Device : public IDisk {
    public:
        constexpr Device()
            : channel_type(ChannelType::PRIMARY), drive_type(DriveType::MASTER) {}
        constexpr Device(ChannelType channel_type, DriveType drive_type)
            : channel_type(channel_type), drive_type(drive_type) {}

        IdentifyResult identify();

        /**
         * See IDisk::read.
         */
        bool read(uint64_t lba, Span<uint8_t> buffer) const override;

        /**
         * See IDisk::write.
         */
        bool write(uint64_t lba, Span<uint8_t> buffer) const override;

        /**
         * Return size in sectors.
         */
        size_t get_size() const;

        /**
         * Return device interface type.
         */
        InterfaceType get_interface_type() const;

        /**
         * Return model name.
         */
        StringView get_model() const;
    
    private:
        ChannelType channel_type;
        DriveType drive_type;
        InterfaceType interface = InterfaceType::ATA;
        uint16_t signature = 0; // Drive signature.
        uint16_t features = 0;
        uint32_t command_sets = 0; // Command sets supported.
        uint32_t size = 0; // Size in sectors.
        Array<char, 41> model;

        /**
         * Access the drive (read or write).
         * Sector count is the whole number of sectors that
         * can fit in `buffer`.
         */
        PollingResult access(
            Direction direction,
            uint64_t lba,
            Span<uint8_t> buffer) const;
    };

    void init(const pci::Function& func);

    Span<const ide::Device> get_disks();
}
