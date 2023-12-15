/**
 * @file
 * Type-safe formatted printing.
 *
 * EXAMPLES:
 * println("Hello, {}!", "world");
 * println("The number: {}, in hexadecimal: {:x}.", 42, 42);
 * println("A pointer: {:p}.", 0x100000);
 */

#pragma once

#include <stdint.h>
#include <util/option.hpp>
#include <util/string_view.hpp>
#include <util/util.hpp>

namespace detail {
    struct FormattingSlot {
        size_t start;
        size_t end;
        char format;
    };

    consteval Option<FormattingSlot> find_formatting_slot(StringView fmt) {
        for (size_t i = 0; i + 1 < fmt.get_size(); i++) {
            // Only look at formatting slots.
            if (fmt[i] != '{') continue;
            size_t start = i;
            i++;

            char format = '\0';
            if (fmt[i] == '}') {
                i++;
            } else if (i + 2 < fmt.get_size() &&
                       fmt[i] == ':' && fmt[i + 2] == '}')
            {
                format = fmt[i + 1];
                i += 3;
            } else {
                compiletime_fail("Malformed formatting slot.");
            }

            return FormattingSlot{
                .start = start,
                .end = i,
                .format = format,
            };
        }

        return {};
    }

    template <typename... Args>
    struct FormatString;

    template <>
    struct FormatString<> {
        consteval FormatString(StringView fmt)
            : inner(fmt)
        {
            auto slot = find_formatting_slot(fmt);
            if (slot.has_value()) {
                compiletime_fail("Given less arguments than expected by the format string.");
            }
        }

        template <size_t N>
        consteval FormatString(const char (&fmt)[N])
            : FormatString(StringView(fmt)) {}

        consteval FormatString() {}

        StringView inner;
    };

    template <typename First, typename... Args>
    struct FormatString<First, Args...> {
        consteval FormatString(StringView fmt) {
            auto slot = find_formatting_slot(fmt);
            if (!slot.has_value()) {
                compiletime_fail("Given more arguments than expected by the format string.");
            }

            inner = fmt.substring(0, slot->start);
            rest = FormatString<Args...>(fmt.substring(slot->end, fmt.get_size() - slot->end));
            format = slot->format;
        }

        template <size_t N>
        consteval FormatString(const char (&fmt)[N])
            : FormatString(StringView(fmt)) {}

        consteval FormatString() {}

        StringView inner;
        FormatString<Args...> rest;
        char format;
    };
}

/**
 * Correct format string for the given list of argument types.
 *
 * Used to check and prepare for printing format strings at compile time.
 */
template <typename... Args>
using FormatString = detail::FormatString<IdentityType<Args>...>;

/**
 * print_value - Print a single value.
 *
 * Not really meant to be used directly.
 * The reason it is not in `detail` is because we can define other
 * overloads for our own values outside.
 */

void print_value(StringView value, char format);

void print_value(const char* value, char format);

void print_value(int value, char format);

void print_value(uint32_t value, char format);

void print_value(uint64_t value, char format);

template <typename T>
void print_value(const T* pointer, char) {
    print_value(reinterpret_cast<uintptr_t>(pointer), 'p');
}

/**
 * Print formatted text.
 *
 * Print the format string substituting `{}` with the respective argument.
 */
inline void print(FormatString<> format) {
    print_value(format.inner, '\0');
}

/**
 * Print formatted text.
 *
 * Print the format string substituting `{}` with the respective argument.
 */
template <typename FirstArg, typename... Args>
void print(FormatString<FirstArg, Args...> format, FirstArg const& first, const Args&... args) {
    print_value(format.inner, '\0');
    print_value(first, format.format);
    print(format.rest, args...);
}

/**
 * Print a new line character.
 */
void println();

/**
 * Print formatted text followed by a new line character.
 */
template <typename... Args>
void println(FormatString<Args...> format, const Args&... args) {
    print(format, args...);
    println();
}
