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

namespace detail {
    /**
     * See RemoveReference.
     */
    template <typename T>
    struct ReferenceRemover {
        typedef T type;
    };

    /**
     * See RemoveReference.
     */
    template <typename T>
    struct ReferenceRemover<T&> {
        typedef T type;
    };

    /**
     * See RemoveReference.
     */
    template <typename T>
    struct ReferenceRemover<T&&> {
        typedef T type;
    };
}

/**
 * Strip a type of a reference.
 */
template <typename T>
using RemoveReference = typename detail::ReferenceRemover<T>::type;

/**
 * true for lvalue references, false for any other type.
 */

template <typename T>
inline constexpr bool IsLvalueRef = false;

template <typename T>
inline constexpr bool IsLvalueRef<T&> = true;

/**
 * This function is not consteval, so calling it in compile time
 * throws an error.
 */
void compiletime_fail(const char* message);

/**
 * Perfect forwarding.
 */
template <typename T>
constexpr T&&
forward(RemoveReference<T>& value) {
    return static_cast<T&&>(value);
}

/**
 * Convert a value into an rvalue reference.
 */
template <typename T>
constexpr RemoveReference<T>&&
move(T&& value) noexcept {
    return static_cast<RemoveReference<T>&&>(value);
}

/**
 * Can be used to create uninitialized objects if put in
 * a union with the said object.
 */
namespace detail {
    struct EmptySpace {};
};
