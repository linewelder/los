#pragma once

namespace detail {
    template<typename T>
    struct IdentityType {
        using Type = T;
    };
}

/**
 * Stops the compiler from deducing type template parameters.
 */
template<typename T>
using IdentityType = typename detail::IdentityType<T>::Type;

/**
 * This function is not consteval, so calling it in compile time
 * throws an error.
 */
void compiletime_fail(const char* message);
