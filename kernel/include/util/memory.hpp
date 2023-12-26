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
 * Compare byte arrays of length `size`.
 *
 * The sign of the result is the sign of the difference between
 * the first pair of bytes that differ; or 0 if they are equal.
 */
int memcmp(const void* a, const void* b, size_t size);
