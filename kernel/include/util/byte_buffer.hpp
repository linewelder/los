#pragma once

#include <util/memory.hpp>

class ByteBuffer {
public:
    explicit ByteBuffer(size_t size) : size(size) {
        if (size > 0) {
            data = new uint8_t[size];
        } else {
            data = nullptr;
        }
    }

    ByteBuffer(const ByteBuffer& other)
        : data(new uint8_t[other.size]),
          size(other.size)
    {
        copy_construct<uint8_t>(*this, other);
    }

    ByteBuffer& operator=(const ByteBuffer& other) {
        delete[] data;

        data = new uint8_t[other.size];
        size = other.size;

        copy_construct<uint8_t>(*this, other);

        return *this;
    }

    ByteBuffer(ByteBuffer&& other)
        : data(other.data), size(other.size)
    {
        other.data = nullptr;
        other.size = 0;
    }

    ByteBuffer& operator=(ByteBuffer&& other) {
        delete[] data;

        data = other.data;
        size = other.size;

        other.data = nullptr;
        other.size = 0;

        return *this;
    }

    ~ByteBuffer() {
        delete[] data;
    }

    constexpr const uint8_t& operator[](size_t index) const {
        ASSERT(index < size);
        return data[index];
    }

    constexpr uint8_t& operator[](size_t index) {
        ASSERT(index < size);
        return data[index];
    }

    constexpr size_t get_size() const {
        return size;
    }

    constexpr uint8_t* begin() { return data; }
    constexpr uint8_t* end() { return data + size; }

    constexpr const uint8_t* begin() const { return data; }
    constexpr const uint8_t* end() const { return data + size; }

    constexpr operator Span<uint8_t>() { return { data, size }; }

    constexpr operator Span<const uint8_t>() const { return { data, size }; }

private:
    uint8_t* data;
    size_t size;
};
