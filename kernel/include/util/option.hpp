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

namespace detail {
    template <typename RefT, typename PtrT>
    class OptionRefBase {
    public:
        constexpr bool has_value() const {
            return ptr != nullptr;
        }

        constexpr RefT get_value() const {
            return *ptr;
        }

        constexpr PtrT operator->() const {
            return ptr;
        }

    protected:
        PtrT ptr;
    };
}

template <typename T>
class [[nodiscard]] Option<T&>
    : public detail::OptionRefBase<T&, T*>
{
public:
    constexpr Option() {
        this->ptr = nullptr;
    }

    constexpr Option(T& value) {
        this->ptr = &value;
    }
};

template <typename T>
class [[nodiscard]] Option<const T&>
    : public detail::OptionRefBase<const T&, const T*>
{
public:
    constexpr Option() {
        this->ptr = nullptr;
    }

    constexpr Option(const T& value) {
        this->ptr = &value;
    }
};

namespace detail {
    template <typename PtrT>
    class OptionPtrBase {
    public:
        constexpr bool has_value() const {
            return ptr != nullptr;
        }

        constexpr PtrT get_value() const {
            return ptr;
        }

        constexpr PtrT operator->() const {
            return ptr;
        }

    protected:
        PtrT ptr;
    };
}

template <typename T>
class [[nodiscard]] Option<T*>
    : public detail::OptionPtrBase<T*>
{
public:
    constexpr Option() {
        this->ptr = nullptr;
    }

    constexpr Option(T* value) {
        this->ptr = value;
    }
};


template <typename T>
class [[nodiscard]] Option<const T*>
    : public detail::OptionPtrBase<const T*>
{
public:
    constexpr Option() {
        this->ptr = nullptr;
    }

    constexpr Option(const T* value) {
        this->ptr = value;
    }
};
