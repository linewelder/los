/*
Bit manipulation functions.
*/

#pragma once

/**
 * Get the value of the nth bit of a number.
 * Bits are counted from 0, meaning the least significant bit, upwards.
 */
template <typename T>
constexpr bool get_bit(T flags, unsigned bit) {
    return (flags & (1 << bit)) != 0;
}

/**
 * Return the number with the nth bit set.
 * Bits are counted from 0, meaning the least significant bit, upwards.
 */
template <typename T>
constexpr T set_bit(T flags, unsigned bit) {
    return flags | (1 << bit);
}

/**
 * Return the number with the nth bit set.
 * Bits are counted from 0, meaning the least significant bit, upwards.
 */
template <typename T>
constexpr T unset_bit(T flags, unsigned bit) {
    return flags & ~(1 << bit);
}

/**
 * Return a number with n lowest bits set.
 */
template <typename T>
constexpr T get_n_ones(unsigned n) {
    T result = 0;
    for (unsigned i = 0; i < n; i++) {
        result = (result << 1) | 1;
    }
    return result;
}

/**
 * Get `length` bits starting from `start` of a number.
 * Bits are counted from 0, meaning the least significant bit, upwards.
 */
template <typename T>
constexpr T get_bit_range(T flags, unsigned start, unsigned length) {
    return (flags >> start) & get_n_ones<T>(length);
}
