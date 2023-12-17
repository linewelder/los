#pragma once

#include <util/util.hpp>

/*
Option<T> - represents an item that be present or not.

NOTE:
Access methods do not check whether the value is present.
You have to check manually using has_value().
*/

template <typename T>
class [[nodiscard]] Option {
public:
    constexpr Option() : exists(false) {}

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

    constexpr bool has_value() const {
        return exists;
    }

    constexpr T get_value() const {
        return value;
    }

    constexpr T* operator->() {
        return &value;
    }

    constexpr const T* operator->() const {
        return &value;
    }

private:
    T value;
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

    constexpr bool has_value() const {
        return ptr != nullptr;
    }

    constexpr T& get_value() const {
        return *ptr;
    }

    constexpr T* operator->() const {
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

    constexpr bool has_value() const {
        return ptr != nullptr;
    }

    constexpr T* get_value() const {
        return ptr;
    }

    constexpr T* operator->() const {
        return ptr;
    }

private:
    T* ptr;
};
