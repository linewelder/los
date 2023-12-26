#pragma once

#include <util/string_view.hpp>
#include <util/memory.hpp>
#include <util/math.hpp>
#include <util/util.hpp>
#include <memory/liballoc.h>

template <typename T>
class Vector {
public:
    explicit Vector(size_t initial_capacity) {
        capacity = initial_capacity;
        if (initial_capacity != 0) {
            items = reinterpret_cast<T*>(kmalloc(sizeof(T) * initial_capacity));
        } else {
            items = nullptr;
        }
        count = 0;
    }

    Vector() : Vector(0) {}

    explicit Vector(Span<const T> data)
        : Vector(max(data.get_size(), 8))
    {
        count = data.get_size();
        copy_construct<T>(*this, data);
    }

    Vector(const Vector& other)
        : items(reinterpret_cast<T*>(kmalloc(sizeof(T) * other.capacity))),
          count(other.count), capacity(other.capacity)
    {
        copy_construct<T>(*this, other);
    }

    Vector& operator=(const Vector& other) {
        delete[] items;

        items = reinterpret_cast<T*>(kmalloc(sizeof(T) * other.capacity));
        count = other.count;
        capacity = other.capacity;

        copy_construct<T>(*this, other);

        return *this;
    }

    Vector(Vector&& other)
        : items(other.items), count(other.count), capacity(other.capacity)
    {
        other.items = nullptr;
        other.capacity = 0;
        other.count = 0;
    }

    Vector& operator=(Vector&& other) {
        delete[] items;

        items = other.items;
        count = other.count;
        capacity = other.capacity;

        other.items = nullptr;
        other.capacity = 0;
        other.count = 0;

        return *this;
    }

    ~Vector() {
        delete[] items;
    }

    /**
     * Append `value` to the end of the array using the copy constructor.
     */
    void push_back(const T& value) {
        if (count == capacity) {
            grow();
        }

        new (&items[count]) T(value);
        count++;
    }

    /**
     * Append `value` to the end of the array using the move constructor.
     */
    void push_back(T&& value) {
        if (count == capacity) {
            grow();
        }

        new (&items[count]) T(move(value));
        count++;
    }

    constexpr const T& operator[](size_t index) const {
        ASSERT(index < count);
        return items[index];
    }

    constexpr T& operator[](size_t index) {
        ASSERT(index < count);
        return items[index];
    }

    constexpr size_t get_size() const {
        return count;
    }

    constexpr T* begin() { return items; }
    constexpr T* end() { return items + count; }

    constexpr const T* begin() const { return items; }
    constexpr const T* end() const { return items + count; }

    constexpr operator Span<T>() { return { items, count }; }

    constexpr operator Span<const T>() const { return { items, count }; }

private:
    void grow() {
        capacity = max(8, capacity * 2);
        items = reinterpret_cast<T*>(krealloc(
            reinterpret_cast<void*>(items), capacity * sizeof(T)));
    }

    T* items;
    size_t count;
    size_t capacity;
};
