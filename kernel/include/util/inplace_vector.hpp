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

    /* Default implementations for trivial objects. */

    InplaceVector(const InplaceVector& other) = default;
    InplaceVector& operator=(const InplaceVector& other) = default;
    constexpr ~InplaceVector() = default;

    /* Conditionally override defaults for not trivial objects. */

    InplaceVector(const InplaceVector& other)
        requires(!IsTriviallyCopyConstructible<T>)
        : count(other.count)
    {
        for (size_t i = 0; i < count; i++) {
            new (&items[i]) T(other.items[i]);
        }
    }

    constexpr InplaceVector& operator=(const InplaceVector& other)
        requires(!IsTriviallyCopyAssignable<T>)
    {
        count = other.count;
        for (size_t i = 0; i < count; i++) {
            items[i] = other.items[i];
        }
        return *this;
    }

    constexpr ~InplaceVector() requires(!IsTriviallyDestructible<T>) {
        for (size_t i = 0; i < count; i++) {
            items[i].~T();
        }
    }

    /* Move constructors. */

    InplaceVector(InplaceVector&& other)
        : count(other.count)
    {
        for (size_t i = 0; i < count; i++) {
            new (&items[i]) T(move(other.items[i]));
        }
        other.count = 0;
    }

    constexpr InplaceVector& operator=(InplaceVector&& other) {
        count = other.count;
        for (size_t i = 0; i < count; i++) {
            items[i] = move(other.items[i]);
        }
        other.count = 0;
        return *this;
    }

    /**
     * Append `value` to the end of the array using the copy constructor
     * and return true. If no more space left, return false.
     */
    [[nodiscard]] bool push_back(const T& value) {
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
