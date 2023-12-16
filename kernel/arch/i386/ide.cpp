#include <arch/i386/ide.hpp>

#include <arch/i386/asm.hpp>
#include <kernel/log.hpp>
#include <util/array.hpp>
#include <util/bits.hpp>
#include <util/inplace_vector.hpp>
#include <util/math.hpp>

namespace ide {
    // Flags in the Status register.
    constexpr uint8_t STATUS_BUSY                = 0x80;
    constexpr uint8_t STATUS_DRIVE_READY         = 0x40;
    constexpr uint8_t STATUS_DRIVE_WRITE_FAULT   = 0x20;
    constexpr uint8_t STATUS_DRIVE_SEEK_COMPLETE = 0x10;
    constexpr uint8_t STATUS_REQUEST_READY       = 0x08;
    constexpr uint8_t STATUS_CORRECTED_DATA      = 0x04;
    constexpr uint8_t STATUS_INDEX               = 0x02;
    constexpr uint8_t STATUS_ERROR               = 0x01;

    // Flags in the Error register.
    constexpr uint8_t ERROR_BAD_BLOCK            = 0x80;
    constexpr uint8_t ERROR_UNCORRECTABLE        = 0x40;
    constexpr uint8_t ERROR_MEDIA_CHANGED        = 0x20;
    constexpr uint8_t ERROR_ID_MASK_NOT_FOUND    = 0x10;
    constexpr uint8_t ERROR_MEDIA_CHANGE_REQUEST = 0x08;
    constexpr uint8_t ERROR_ABORTED              = 0x04;
    constexpr uint8_t ERROR_TRACK_0_NOT_FOUND    = 0x02;
    constexpr uint8_t ERROR_NO_ADDRE_MARK        = 0x01;

    // Offsets in the identification space (in uint16_t's).
    constexpr size_t IDENT_DEVICE_TYPE  = 0;
    constexpr size_t IDENT_MODEL        = 27;
    constexpr size_t IDENT_FEATURES     = 49;
    constexpr size_t IDENT_MAX_LBA      = 60;
    constexpr size_t IDENT_COMMAND_SETS = 82;
    constexpr size_t IDENT_MAX_LBA_EXT  = 100;

    constexpr uint16_t FEATURES_SUPPORTS_LBA = 1 << 9;

    constexpr uint32_t COMMAND_SETS_USES_48_BIT = 1 << 26;

    /*
    Base IO port is:
    - BAR0 for the primary channel;
    - BAR2 for the secondary channel.
    Control base IO port is:
    - BAR1 for the primary channel;
    - BAR3 for the secondary channel.
    Bus master IDE base IO port is:
    - BAR4 for the primary channel;
    - BAR4 + 8 for the secondary channel.
    */
    enum class Register {
        // Upper 4 bits are used to distinguish different
        // kinds of registers.

        // Offsets from the base IO port.
        DATA          = 0x00,
        ERROR         = 0x01, // Read only.
        FEATURES      = 0x01, // Write only.
        SECTOR_COUNT  = 0x02,
        LBA0          = 0x03,
        LBA1          = 0x04,
        LBA2          = 0x05,
        DRIVE_SELECT  = 0x06, // Select drive in the channel.
        STATUS        = 0x07, // Read only.
        COMMAND       = 0x07, // Write only.

        // Offsets from the base IO port (written before their lower counterparts).
        SECTOR_COUNT1 = 0x12,
        LBA3          = 0x13,
        LBA4          = 0x14,
        LBA5          = 0x15,

        // Offsets from the the control base IO port.
        ALT_STATUS    = 0x22, // Read only.
        CONTROL       = 0x22, // Write only.
    };

    enum class Command {
        READ_PIO        = 0x20,
        READ_PIO_EXT    = 0x24,
        WRITE_PIO       = 0x30,
        WRITE_PIO_EXT   = 0x34,
        IDENTIFY        = 0xec,
        IDENTIFY_PACKET = 0xa1,
        CACHE_FLUSH     = 0xe7,
        CACHE_FLUSH_EXT = 0xea,
    };

    class Channel {
    public:
        uint16_t read_data() const {
            return inw(base_port);
        }

        void write_data(uint16_t value) const {
            outw(base_port, value);
        }

        uint16_t read_lba12() const {
            return inb(base_port + 5) << 8 |
                   inb(base_port + 4);
        }

        uint8_t read_status() const {
            return inb(base_port + 7);
        }

        uint8_t read_errors() const {
            return inb(base_port + 1);
        }

        uint8_t read_alt_status() const {
            return inb(control_base_port + 2);
        }

        void write_sector_count(uint8_t value) const {
            outb(base_port + 2, value);
        }

        void write_lba(uint8_t low, uint8_t mid, uint8_t high) const {
            outb(base_port + 3, low);
            outb(base_port + 4, mid);
            outb(base_port + 5, high);
        }

        void write_drive_select(uint8_t value) const {
            outb(base_port + 6, value);
        }

        void write_command(Command data) const {
            outb(base_port + 7, static_cast<uint8_t>(data));
        }

        void disable_irqs() const {
            outb(control_base_port + 2, 2); // Set bit 1 in the control port.
        }

        // Need to add 400ns delays before all the status registers are up to date.
        // https://wiki.osdev.org/ATA_PIO_Mode#400ns_delays
        void delay_400ns() const {
            read_alt_status();
            read_alt_status();
            read_alt_status();
            read_alt_status();
        }

        void wait_request_ready() const {
            while (!(read_status() & (STATUS_REQUEST_READY | STATUS_ERROR))) {
                tiny_delay();
            };
        }

        void wait_not_busy() const {
            while (read_status() & STATUS_BUSY) {
                tiny_delay();
            };
        }

        PollingResult poll(bool advanced_check) const {
            delay_400ns();
            wait_not_busy();

            if (advanced_check) {
                uint8_t status = read_status();
                if (status & STATUS_ERROR) {
                    return PollingResult::ERROR;
                }

                if (status & STATUS_DRIVE_WRITE_FAULT) {
                    return PollingResult::DRIVE_WRITE_FAULT;
                }

                // No errors, but request is not ready.
                if ((status & STATUS_REQUEST_READY) == 0) {
                    return PollingResult::REQUEST_NOT_READY;
                }
            }

            return PollingResult::SUCCESS;
        }

        PollingResult read_sectors(uint8_t sector_count, Span<uint8_t> buffer) const {
            for (uint8_t i = 0; i < sector_count; i++) {
                PollingResult result = poll(true);
                if (result != PollingResult::SUCCESS) {
                    return result;
                }

                for (int j = 0; j < 256; j++) {
                    uint16_t word = read_data();
                    buffer[2 * j] = get_bit_range(word, 0, 8);
                    buffer[2 * j + 1] = get_bit_range(word, 8, 8);
                }
            }

            return PollingResult::SUCCESS;
        }

        PollingResult write_sectors(uint8_t sector_count, Span<uint8_t> buffer) const {
            for (uint8_t i = 0; i < sector_count; i++) {
                poll(false);
                for (int j = 0; j < 256; j++) {
                    write_data(buffer[2 * j] |
                        (buffer[2 * j + 1] << 8));
                }
            }

            return PollingResult::SUCCESS;
        }

        uint16_t base_port;
        uint16_t control_base_port;
        uint16_t bus_master_port;
    };

    Array<Channel, 2> channels = {{
        { 0x1f0, 0x3f6, 0 },
        { 0x170, 0x376, 0 },
    }};

    IdentifyResult Device::identify() {
        Channel& channel = channels[static_cast<int>(channel_type)];

        switch (drive_type) {
        case DriveType::MASTER:
            channel.write_drive_select(0xa0);
            break;
        case DriveType::SLAVE:
            channel.write_drive_select(0xa0 | (1 << 4));
            break;
        }
        io_wait();

        channel.write_command(Command::IDENTIFY);
        io_wait();

        channel.delay_400ns();
        if (channel.read_status() == 0) {
            return { IdentifyResultStatus::NoDevice, 0 };
        }

        interface = InterfaceType::ATA;
        bool error = false;
        for (;;) {
            uint8_t status = channel.read_status();
            if (status & STATUS_ERROR) {
                error = true;
                break;
            }
            if (!(status & STATUS_BUSY) && (status & STATUS_REQUEST_READY)) {
                break;
            }
        }

        if (error) {
            uint16_t device_type = channel.read_lba12();
            if (device_type == 0xeb14 || device_type == 0x9669) {
                interface = InterfaceType::ATAPI;
            } else {
                return { IdentifyResultStatus::UnknownDeviceType, 0 };
            }
 
            channel.write_command(Command::IDENTIFY_PACKET);
            io_wait();
        }

        channel.wait_request_ready();
        if ((channel.read_status() & STATUS_ERROR)) {
            uint8_t error_byte = channel.read_errors();
            return { IdentifyResultStatus::RequestError, error_byte };
        };

        Array<uint16_t, 256> identification;
        for (int i = 0; i < 256; i++) {
            identification[i] = channel.read_data();
        }

        auto get_identification_uint32 = [&](size_t offset) {
            return (identification[offset + 1] << 16) |
                identification[offset];
        };

        signature = identification[IDENT_DEVICE_TYPE];
        features = identification[IDENT_FEATURES];
        command_sets = get_identification_uint32(IDENT_COMMAND_SETS);

        if (command_sets & COMMAND_SETS_USES_48_BIT) {
            size = get_identification_uint32(IDENT_MAX_LBA_EXT);
        } else { // CHS or 28-bit addressing.
            size = get_identification_uint32(IDENT_MAX_LBA);
        }

        // I have no idea why we have to swap the characters.
        int last_nonspace_index = 0;
        for (int i = 0; i < 20; i ++) {
            char a = get_bit_range(identification[IDENT_MODEL + i], 8, 8);
            char b = get_bit_range(identification[IDENT_MODEL + i], 0, 8);

            if (b != ' ') last_nonspace_index = 2 * i + 1;
            else if (a != ' ') last_nonspace_index = 2 * i;

            model[2 * i] = a;
            model[2 * i + 1] = b;
        }
        model[last_nonspace_index + 1] = '\0';

        return { IdentifyResultStatus::Success, 0 };
    }

    PollingResult Device::access(
        Direction direction,
        uint64_t lba,
        Span<uint8_t> buffer) const
    {
        uint8_t sector_count{ static_cast<uint8_t>(
            min(buffer.get_size() / 512, 255)) };

        enum class AddressMode {
            CHS,
            LBA28,
            LBA48,
        } address_mode;
        Array<uint8_t, 6> lba_io;
        uint8_t head;

        if (lba >= 0x1000'0000) {
            address_mode = AddressMode::LBA48;
            for (int i = 0; i < 6; i++) {
                lba_io[i] = get_bit_range(lba, i * 8, 8);
            }
            head      = 0;
        } else if (features & FEATURES_SUPPORTS_LBA) {
            address_mode = AddressMode::LBA28;
            uint32_t lba_low = static_cast<uint32_t>(lba);
            lba_io[0] = get_bit_range(lba_low, 0, 8);
            lba_io[1] = get_bit_range(lba_low, 8, 8);
            lba_io[2] = get_bit_range(lba_low, 16, 8);
            lba_io[3] = 0;
            lba_io[4] = 0;
            lba_io[5] = 0;
            head      = get_bit_range(lba_low, 24, 4);
        } else {
            address_mode = AddressMode::CHS;

            uint32_t lba_low = static_cast<uint32_t>(lba);
            uint8_t sector = (lba_low % 63) + 1;
            uint16_t cylinder = (lba_low + 1 - sector) / (16 * 63);
            head = (lba_low + 1 - sector) % (16 * 63) / 63;

            lba_io[0] = sector;
            lba_io[1] = get_bit_range(cylinder, 0, 8);
            lba_io[2] = get_bit_range(cylinder, 8, 8);
            lba_io[3] = 0;
            lba_io[4] = 0;
            lba_io[5] = 0;
        }

        const Channel& channel = channels[static_cast<int>(channel_type)];
        channel.wait_not_busy();

        uint8_t drive_select_value = 0xa0;
        if (address_mode != AddressMode::CHS) {
            drive_select_value = set_bit(drive_select_value, 6);
        }
        if (drive_type == DriveType::SLAVE) {
            drive_select_value = set_bit(drive_select_value, 4);
        }
        drive_select_value |= head;
        channel.write_drive_select(drive_select_value);

        if (address_mode == AddressMode::LBA48) {
            channel.write_sector_count(0);
            channel.write_lba(lba_io[3], lba_io[4], lba_io[5]);
        }
        channel.write_sector_count(sector_count);
        channel.write_lba(lba_io[0], lba_io[1], lba_io[2]);

        if (direction == Direction::READ) {
            channel.write_command(
                address_mode == AddressMode::LBA48
                    ? Command::READ_PIO_EXT
                    : Command::READ_PIO);
            return channel.read_sectors(sector_count, buffer);
        } else {
            channel.write_command(
                address_mode == AddressMode::LBA48
                    ? Command::WRITE_PIO_EXT
                    : Command::WRITE_PIO);

            PollingResult result = channel.write_sectors(sector_count, buffer);
            if (result != PollingResult::SUCCESS) return result;

            channel.write_command(
                address_mode == AddressMode::LBA48
                    ? Command::CACHE_FLUSH_EXT
                    : Command::CACHE_FLUSH);
            return channel.poll(false);
        }
    }

    bool Device::read(uint64_t lba, Span<uint8_t> buffer) const {
        return access(Direction::READ, lba, buffer) == PollingResult::SUCCESS;
    }

    bool Device::write(uint64_t lba, Span<uint8_t> buffer) const {
        access(Direction::WRITE, lba, buffer);
        return true;
    }

    size_t Device::get_size() const {
        return size;
    }

    InterfaceType Device::get_interface_type() const {
        return interface;
    }

    StringView Device::get_model() const {
        size_t length = 0;
        while (length < model.get_size() && model[length] != '\0') {
            length++;
        }

        return { model.data, length };
    }

    static InplaceVector<ide::Device, 4> disks;

    Span<const ide::Device> get_disks() {
        return disks;
    }

    void init(const pci::Function& func) {
        Array<uint16_t, 5> bars;
        for (uint8_t i = 0; i < 5; i++) {
            bars[i] = func.get_bar_io(i);
        }

        if (bars[0]) channels[0].base_port = bars[0];
        if (bars[1]) channels[0].control_base_port = bars[1];
        if (bars[2]) channels[1].base_port = bars[2];
        if (bars[3]) channels[1].control_base_port = bars[3];
        if (bars[4]) {
            channels[0].bus_master_port = bars[4];
            channels[1].bus_master_port = bars[4] + 8;
        }

        channels[0].disable_irqs();
        channels[1].disable_irqs();

        for (int channel = 0; channel < 2; channel++) {
            for (int drive_type = 0; drive_type < 2; drive_type++) {
                int id = channel * 2 + drive_type;
                ide::Device disk(
                    static_cast<ide::ChannelType>(channel),
                    static_cast<ide::DriveType>(drive_type));
                IdentifyResult result = disk.identify();

                switch (result.status) {
                case IdentifyResultStatus::Success:
                    disks.push_back(disk);
                    break;
                case IdentifyResultStatus::NoDevice:
                    break;
                case IdentifyResultStatus::UnknownDeviceType:
                    LOG_ERROR("Error identifying disk {}: Unknown device type\n", id);
                    break;
                case IdentifyResultStatus::RequestError:
                    LOG_ERROR("Error identifying disk {}: Device returned error code {:x}\n",
                        id, result.error_byte);
                    break;
                }
            }
        }
    }
}
