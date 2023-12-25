/**
 * @file
 * Macro for defining bitwise operations for an enum type.
 */

#pragma once

#define _DEFINE_OPERATOR(Enum, assignment, op) \
    [[nodiscard]] \
    constexpr Enum operator op(Enum a, Enum b) { \
        using Type = UnderlyingType<Enum>; \
        return static_cast<Enum>(static_cast<Type>(a) op static_cast<Type>(b)); \
    } \
    constexpr Enum& operator assignment(Enum& a, Enum b) { \
        using Type = UnderlyingType<Enum>; \
        return a = static_cast<Enum>(static_cast<Type>(a) op static_cast<Type>(b)); \
    }

/**
 * Define bitwise operations for the given enum type.
 */
#define ENUM_FLAGS(Enum) \
    _DEFINE_OPERATOR(Enum, &=, &) \
    _DEFINE_OPERATOR(Enum, |=, |) \
    _DEFINE_OPERATOR(Enum, ^=, ^) \
    constexpr bool has_flag(FileAttr value, FileAttr mask) { \
        using Type = UnderlyingType<FileAttr>; \
        return static_cast<Type>(value & mask) == static_cast<Type>(mask); \
    }
