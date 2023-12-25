#pragma once

#include <util/assert.hpp>
#include <util/util.hpp>

/*
Option<T> - represents an item that be present or not.
*/

template <typename T>
class [[nodiscard]] Option {
public:
    constexpr Option()
        : empty(), exists(false) {}

    constexpr Option(const T& value)
        : value(value), exists(true) {}

    constexpr Option& operator=(const T& value) {
        this->value = value;
        exists = true;
        return *this;
    }

    constexpr Option(T&& value)
        : value(move(value)), exists(true) {}

    constexpr Option& operator=(T&& value) {
        this->value = move(value);
        exists = true;
        return *this;
    }

    /* Default implementations for trivial objects. */

    constexpr Option(const Option& other) = default;
    constexpr Option& operator=(const Option& other) = default;
    constexpr ~Option() = default;

    /* Conditionally override defaults for not trivial objects. */

    constexpr Option(const Option& other) requires(!IsTriviallyCopyConstructible<T>)
        : exists(other.exists)
    {
        if (other.exists) {
            new (&value) T(other.value);
        }
    }

    constexpr Option& operator=(const Option& other)
        requires(!IsTriviallyCopyAssignable<T>)
    {
        exists = other.exists;
        if (other.exists) {
            value = other.value;
        }
        return *this;
    }

    constexpr ~Option() requires(!IsTriviallyDestructible<T>) {
        if (exists) {
            value.~T();
        }
    }

    /* Move constructors. */

    constexpr Option(Option&& other)
        : exists(other.exists)
    {
        if (other.exists) {
            new (&value) T(move(other.value));
            other.exists = false;
        }
    }

    constexpr Option& operator=(Option&& other) {
        exists = other.exists;
        if (other.exists) {
            value = move(other.value);
            other.exists = false;
        }
        return *this;
    }

    constexpr bool has_value() const {
        return exists;
    }

    constexpr T& get_value() {
        ASSERT(has_value());
        return value;
    }

    constexpr const T& get_value() const {
        ASSERT(has_value());
        return value;
    }

    constexpr T* operator->() {
        ASSERT(has_value());
        return &value;
    }

    constexpr const T* operator->() const {
        ASSERT(has_value());
        return &value;
    }

private:
    union {
        T value;
        detail::EmptySpace empty;
    };
    bool exists;
};

template <typename T>
class [[nodiscard]] Option<T&> {
public:
    constexpr Option() {
        this->ptr = nullptr;
    }

    constexpr Option(T& value) {
        this->ptr = &value;
    }

    constexpr Option& operator=(T& value) {
        this->ptr = &value;
        return *this;
    }

    constexpr bool has_value() const {
        return ptr != nullptr;
    }

    constexpr T& get_value() const {
        ASSERT(has_value());
        return *ptr;
    }

    constexpr T* operator->() const {
        ASSERT(has_value());
        return ptr;
    }

private:
    T* ptr;
};

template <typename T>
class [[nodiscard]] Option<T*> {
public:
    constexpr Option() {
        this->ptr = nullptr;
    }

    constexpr Option(T* value) {
        this->ptr = value;
    }

    constexpr Option& operator=(T* value) {
        this->ptr = value;
        return *this;
    }

    constexpr bool has_value() const {
        return ptr != nullptr;
    }

    constexpr T* get_value() const {
        ASSERT(has_value());
        return ptr;
    }

    constexpr T* operator->() const {
        ASSERT(has_value());
        return ptr;
    }

private:
    T* ptr;
};
