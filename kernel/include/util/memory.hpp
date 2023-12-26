/**
 * Utilitary memory functions and declarations allowing to use placement new.
 */

#pragma once

#include <stddef.h>
#include <util/span.hpp>

/* Needed to be able to use placement new. (https://wiki.osdev.org/C%2B%2B#Placement_New) */
inline void* operator new(size_t, void* p) { return p; }
inline void* operator new[](size_t, void* p) { return p; }
inline void operator delete(void*, void*) {}
inline void operator delete[](void*, void*) {}

/**
 * Copy `size` objects from `src` to `dst` using
 * the copy constructor.
 */
template <typename T>
void copy_construct(Span<T> dst, Span<const T> src) requires(IsCopyConstructible<T>) {
    ASSERT(dst.get_size() == src.get_size());

    // To avoid bound checks (we have the assertion above).
    auto dst_iter = dst.begin();
    for (const auto& elem : src) {
        new (dst_iter) T(elem);
        dst_iter++;
    }
}

/**
 * Copy `size` objects from `src` to `dst` using
 * the copy assignment operator.
 */
template <typename T>
void copy_assign(Span<T> dst, Span<const T> src) requires(IsCopyConstructible<T>) {
    ASSERT(dst.get_size() == src.get_size());

    // To avoid bound checks (we have the assertion above).
    auto dst_iter = dst.begin();
    for (const auto& elem : src) {
        *dst_iter = elem;
        dst_iter++;
    }
}

/**
 * Compare two spans alphabetically.
 */
template <typename T>
int compare(Span<const T> lhs, Span<const T> rhs) {
    auto lhs_iter = lhs.begin();
    auto rhs_iter = rhs.begin();

    auto lhs_end = lhs.end();
    auto rhs_end = rhs.end();

    while (lhs_iter != lhs_end && rhs_iter != rhs_end) {
        if (*lhs_iter < *rhs_iter) {
            return -1;
        } else if (*lhs_iter > * rhs_iter) {
            return 1;
        }

        lhs_iter++;
        rhs_iter++;
    }

    if (lhs_iter != lhs_end) {
        return 1;
    } else if (rhs_iter != rhs_end) {
        return -1;
    } else {
        return 0;
    }
}
