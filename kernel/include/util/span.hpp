#pragma once

#include <stdint.h>
#include <stddef.h>
#include <util/assert.hpp>
#include <util/util.hpp>

template <typename T>
struct Span {
public:
    constexpr const T& operator[](size_t index) const {
        ASSERT(index < size);
        return start[index];
    }

    constexpr T& operator[](size_t index) {
        ASSERT(index < size);
        return start[index];
    }

    constexpr size_t get_size() const {
        return size;
    }

    constexpr T* begin() const { return start; }

    constexpr T* end() const { return start + size; }

    constexpr operator Span<const T>() const requires(!IsConst<T>) {
        return { start, size };
    }

    T* start;
    size_t size;
};
