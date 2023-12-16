#pragma once

#include <stdint.h>
#include <stddef.h>

#include <util/assert.hpp>
#include <util/memory.hpp>
#include <util/span.hpp>

/**
 * Dynamic array without dynamic memory allocation.
 */
template <typename T, size_t CAPACITY>
class InplaceVector {
public:
    constexpr InplaceVector() : count(0) {}

    /**
     * Append `value` to the end of the array using the copy constructor
     * and return true. If no more space left, return false.
     */
    bool push_back(const T& value) {
        if (count == CAPACITY) {
            return false;
        }

        T* slot = &reinterpret_cast<T*>(data)[count];
        new (slot) T(value);
        count++;
        return true;
    }

    const T& operator[](size_t index) const {
        ASSERT(index < count);
        return reinterpret_cast<const T*>(data)[index];
    }

    T& operator[](size_t index) {
        ASSERT(index < count);
        return reinterpret_cast<T*>(data)[index];
    }

    size_t get_count() const {
        return count;
    }

    constexpr size_t get_capacity() const {
        return CAPACITY;
    }

    constexpr operator Span<const T>() const {
        return { reinterpret_cast<const T*>(data), count };
    }

    constexpr operator Span<T>() const {
        return { reinterpret_cast<T*>(data), count };
    }

private:
    alignas(T) uint8_t data[sizeof(T) * CAPACITY]; // Raw byte array to avoid needing to initialize the elements.
    size_t count;
};
