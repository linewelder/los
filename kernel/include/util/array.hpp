#pragma once

#include <stdint.h>
#include <stddef.h>
#include <util/assert.hpp>
#include <util/span.hpp>

/**
 * Wrapper around an array.
 */
template <typename T, size_t SIZE>
struct Array {
public:
    constexpr const T& operator[](size_t index) const {
        ASSERT(index < SIZE);
        return data[index];
    }

    constexpr T& operator[](size_t index) {
        ASSERT(index < SIZE);
        return data[index];
    }

    constexpr size_t get_size() const {
        return SIZE;
    }

    T data[SIZE];
};

// Helps the compiler deduce the size based on the initializer list.
template<typename First, typename... Rest>
Array(First, Rest...) -> Array<First, sizeof...(Rest) + 1>;
