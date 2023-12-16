#pragma once

#include <stddef.h>
#include <util/assert.hpp>

/**
 * A const slice of a string. Not null-terminated.
 */
class StringView {
public:
    constexpr StringView(const char* start, size_t size)
        : start(start), size(size) {}

    template <size_t SIZE>
    constexpr StringView(const char (&literal)[SIZE])
        : start(literal), size(SIZE - 1) {}

    constexpr StringView()
        : start(nullptr), size(0) {}

    constexpr char operator[](size_t index) const {
        ASSERT(index < size);
        return start[index];
    }

    constexpr size_t get_size() const {
        return size;
    }

    constexpr StringView substring(size_t start, size_t size) const {
        ASSERT(start + size <= this->size);
        return StringView(this->start + start, size);
    }

private:
    const char* start;
    size_t size;
};
