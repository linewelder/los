#pragma once

#include <stdint.h>
#include <stddef.h>

/**
 * Wrapper around an array.
 */
template <typename T, size_t SIZE>
struct Array {
public:
    constexpr const T& operator[](size_t index) const {
        return data[index];
    }

    constexpr T& operator[](size_t index) {
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
