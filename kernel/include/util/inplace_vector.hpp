#pragma once

#include <stdint.h>
#include <stddef.h>

#include <util/assert.hpp>
#include <util/memory.hpp>
#include <util/span.hpp>
#include <util/util.hpp>

/**
 * Dynamic array without dynamic memory allocation.
 */
template <typename T, size_t CAPACITY>
class InplaceVector {
public:
    constexpr InplaceVector()
        : empty(), count(0) {}

    /**
     * Append `value` to the end of the array using the copy constructor
     * and return true. If no more space left, return false.
     */
    bool push_back(const T& value) {
        if (count == CAPACITY) {
            return false;
        }

        new (&items[count]) T(value);
        count++;
        return true;
    }

    const T& operator[](size_t index) const {
        ASSERT(index < count);
        return items[index];
    }

    T& operator[](size_t index) {
        ASSERT(index < count);
        return items[index];
    }

    size_t get_count() const {
        return count;
    }

    constexpr size_t get_capacity() const {
        return CAPACITY;
    }

    constexpr operator Span<const T>() const {
        return { &items[0], count };
    }

    constexpr operator Span<T>() const {
        return { &items[0], count };
    }

    constexpr T* begin() { return &items[0]; }

    constexpr T* end() { return &items[count]; }

    constexpr const T* begin() const { return &items[0]; }

    constexpr const T* end() const { return &items[count]; }

private:
    union {
        T items[CAPACITY];
        detail::EmptySpace empty;
    };
    size_t count;
};
