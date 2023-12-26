#pragma once

#include <util/vector.hpp>
#include <util/string_view.hpp>

/**
 * Heap-allocated char vector.
 */
class String {
public:
    explicit String(StringView view)
        : chars(view) {}

    String() : chars() {}

    constexpr operator StringView() const {
        return { begin(), get_size() };
    }

    constexpr char& operator[](size_t index) {
        return chars[index];
    }

    constexpr const char& operator[](size_t index) const {
        return chars[index];
    }

    constexpr operator Span<char>() { return chars; }

    constexpr operator Span<const char>() const { return chars; }

    constexpr size_t get_size() const {
        return chars.get_size();
    }

    void push_back(char ch) {
        chars.push_back(ch);
    }

    constexpr char* begin() { return chars.begin(); }
    constexpr char* end() { return chars.end(); }

    constexpr const char* begin() const { return chars.begin(); }
    constexpr const char* end() const { return chars.end(); }

private:
    Vector<char> chars;
};
