#include <arch/i386/keyboard.hpp>

#include <arch/i386/asm.hpp>
#include <arch/i386/pic.hpp>
#include <arch/i386/ps2.hpp>

namespace keyboard {
    static constexpr char get_key_character(uint16_t scancode) {
        switch (scancode) {
            case 0x0e: return '`';
            case 0x15: return 'q';
            case 0x16: return '1';
            case 0x1a: return 'z';
            case 0x1b: return 's';
            case 0x1c: return 'a';
            case 0x1d: return 'w';
            case 0x1e: return '2';
            case 0x21: return 'c';
            case 0x22: return 'x';
            case 0x23: return 'd';
            case 0x24: return 'e';
            case 0x25: return '4';
            case 0x26: return '3';
            case 0x29: return ' ';
            case 0x2a: return 'v';
            case 0x2b: return 'f';
            case 0x2c: return 't';
            case 0x2d: return 'r';
            case 0x2e: return '5';
            case 0x31: return 'n';
            case 0x32: return 'b';
            case 0x33: return 'h';
            case 0x34: return 'g';
            case 0x35: return 'y';
            case 0x36: return '6';
            case 0x3a: return 'm';
            case 0x3b: return 'j';
            case 0x3c: return 'u';
            case 0x3d: return '7';
            case 0x3e: return '8';
            case 0x41: return ',';
            case 0x42: return 'k';
            case 0x43: return 'i';
            case 0x44: return 'o';
            case 0x45: return '0';
            case 0x46: return '9';
            case 0x49: return '.';
            case 0x4a: return '/';
            case 0x4b: return 'l';
            case 0x4c: return ';';
            case 0x4d: return 'p';
            case 0x4e: return '-';
            case 0x52: return '\'';
            case 0x54: return '[';
            case 0x55: return '=';
            case 0x5a: return '\n';
            case 0x5b: return ']';
            case 0x5d: return '\\';
            case 0x66: return '\b';
            default:   return '\0';
        }
    }

    static constexpr char get_key_character_shifted(uint16_t scancode) {
        switch (scancode) {
            case 0x0e: return '~';
            case 0x15: return 'Q';
            case 0x16: return '!';
            case 0x1a: return 'Z';
            case 0x1b: return 'S';
            case 0x1c: return 'A';
            case 0x1d: return 'W';
            case 0x1e: return '@';
            case 0x21: return 'C';
            case 0x22: return 'X';
            case 0x23: return 'D';
            case 0x24: return 'E';
            case 0x25: return '$';
            case 0x26: return '#';
            case 0x29: return ' ';
            case 0x2a: return 'V';
            case 0x2b: return 'F';
            case 0x2c: return 'T';
            case 0x2d: return 'R';
            case 0x2e: return '%';
            case 0x31: return 'N';
            case 0x32: return 'B';
            case 0x33: return 'H';
            case 0x34: return 'G';
            case 0x35: return 'Y';
            case 0x36: return '^';
            case 0x3a: return 'M';
            case 0x3b: return 'J';
            case 0x3c: return 'U';
            case 0x3d: return '&';
            case 0x3e: return '*';
            case 0x41: return '<';
            case 0x42: return 'K';
            case 0x43: return 'I';
            case 0x44: return 'O';
            case 0x45: return ')';
            case 0x46: return '(';
            case 0x49: return '>';
            case 0x4a: return '?';
            case 0x4b: return 'L';
            case 0x4c: return ':';
            case 0x4d: return 'P';
            case 0x4e: return '_';
            case 0x52: return '"';
            case 0x54: return '{';
            case 0x55: return '+';
            case 0x5a: return '\n';
            case 0x5b: return '}';
            case 0x5d: return '|';
            case 0x66: return '\b';
            default:   return '\0';
        }
    }

    static KeyEventCallback callback = 0;

    void set_callback(KeyEventCallback func) {
        callback = func;
    }

    static bool received_e0 = false;
    static bool releasing = false;
    static uint8_t modifiers = 0;

    __attribute__((interrupt))
    void irq_handler(idt::InterruptFrame*) {
        uint8_t received = ps2::read_input();
        switch (received) {
            case 0xe0:
                received_e0 = true;
                break;

            case 0xf0:
                releasing = true;
                break;

            default: {
                uint16_t scancode = received;
                if (received_e0) scancode |= 0xe000;
                Key key = static_cast<Key>(scancode);

                if (releasing) {
                    switch (key) {
                    case Key::LEFT_SHIFT:
                    case Key::RIGHT_SHIFT:
                        modifiers &= ~MODIFIER_SHIFT;
                        break;
                    case Key::LEFT_ALT:
                    case Key::RIGHT_ALT:
                        modifiers &= ~MODIFIER_ALT;
                        break;
                    case Key::LEFT_CTRL:
                    case Key::RIGHT_CTRL:
                        modifiers &= ~MODIFIER_CTRL;
                        break;
                    case Key::LEFT_SUPER:
                    case Key::RIGHT_SUPER:
                        modifiers &= ~MODIFIER_SUPER;
                        break;
                    default: break;
                    }
                } else {
                    switch (key) {
                    case Key::LEFT_SHIFT:
                    case Key::RIGHT_SHIFT:
                        modifiers |= MODIFIER_SHIFT;
                        break;
                    case Key::LEFT_ALT:
                    case Key::RIGHT_ALT:
                        modifiers |= MODIFIER_ALT;
                        break;
                    case Key::LEFT_CTRL:
                    case Key::RIGHT_CTRL:
                        modifiers |= MODIFIER_CTRL;
                        break;
                    case Key::LEFT_SUPER:
                    case Key::RIGHT_SUPER:
                        modifiers |= MODIFIER_SUPER;
                        break;
                    default: break;
                    }
                }

                // Scan code 0xe07c only appears as the second half of the Print Screen
                // scancode, so we ignore it and use the first half to identify
                // Print Screen.
                if (scancode != 0xe07c && callback) {
                    char character = modifiers & MODIFIER_SHIFT
                        ? get_key_character_shifted(scancode)
                        : get_key_character(scancode);

                    callback({
                        key,
                        releasing,
                        character,
                        modifiers,
                    });
                }

                releasing = false;
                received_e0 = false;
                break;
            }
        }

        pic::send_eoi(1);
    }
}
