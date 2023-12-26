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
 * true for types qualified as cont.
 */

template <typename T>
inline constexpr bool IsConst = false;

template <typename T>
inline constexpr bool IsConst<T const> = true;

/**
 * Type traits for container types.
 */

template<typename T>
inline constexpr bool IsTriviallyCopyConstructible = __is_trivially_constructible(T, const T&);

template<typename T>
inline constexpr bool IsTriviallyCopyAssignable = __is_trivially_assignable(T&, const T&);

template<typename T>
inline constexpr bool IsTriviallyMoveConstructible = __is_trivially_constructible(T, T&&);

template<typename T>
inline constexpr bool IsTriviallyMoveAssignable = __is_trivially_assignable(T&, T&&);

template <typename T>
inline constexpr bool IsTriviallyDestructible = __has_trivial_destructor(T);

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
