/**
 * @file
 * Type-safe formatted printing.
 */

#pragma once

#include <util/string_view.h>
#include <util/util.h>

namespace detail {
    template <typename... Args>
    struct FormatString;

    template <>
    struct FormatString<> {
        consteval FormatString(StringView fmt)
            : inner(fmt)
        {
            for (size_t i = 0; i + 1 < fmt.get_size(); i++) {
                if (fmt[i] == '{' && fmt[i + 1] == '}') {
                    compiletime_fail("Given less arguments than expected by the format string.");
                }
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
        consteval FormatString(StringView fmt)
            : inner(fmt)
        {
            for (size_t i = 0; i + 1 < fmt.get_size(); i++) {
                if (fmt[i] == '{' && fmt[i + 1] == '}') {
                    inner =  fmt.substring(0, i);
                    rest = FormatString<Args...>(fmt.substring(i + 2, fmt.get_size() - i - 2));
                    return;
                }
            }

            compiletime_fail("Given more arguments than expected by the format string.");
        }

        template <size_t N>
        consteval FormatString(const char (&fmt)[N])
            : FormatString(StringView(fmt)) {}

        consteval FormatString() {}

        StringView inner;
        FormatString<Args...> rest;
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
 * Print a single StringView.
 *
 * Not really meant to be used directly.
 * The reason it is not in `detail` is because we can define other
 * overloads for our own values outside.
 */
void print_value(StringView value);

/**
 * Print a single const char*.
 *
 * Not really meant to be used directly.
 * The reason it is not in `detail` is because we can define other
 * overloads for our own values outside.
 */
void print_value(const char* value);

/**
 * Print a single int.
 *
 * Not really meant to be used directly.
 * The reason it is not in `detail` is because we can define other
 * overloads for our own values outside.
 */
void print_value(int value);

/**
 * Print formatted text.
 *
 * Print the format string substituting `{}` with the respective argument.
 */
inline void print(FormatString<> format) {
    print_value(format.inner);
}

/**
 * Print formatted text.
 *
 * Print the format string substituting `{}` with the respective argument.
 */
template <typename FirstArg, typename... Args>
void print(FormatString<FirstArg, Args...> format, FirstArg const& first, const Args&... args) {
    print_value(format.inner);
    print_value(first);
    print(format.rest, args...);
}

template <typename... Args>
void println(FormatString<Args...> format, const Args&... args) {
    print(format, args...);
    print_value("\n");
}
