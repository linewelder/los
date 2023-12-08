/**
 * Utilitary memory functions and declarations allowing to use placement new.
 */

#pragma once

#include <stddef.h>

/* Needed to be able to use placement new. (https://wiki.osdev.org/C%2B%2B#Placement_New) */
inline void* operator new(size_t, void* p) { return p; }
inline void* operator new[](size_t, void* p) { return p; }
inline void operator delete(void*, void*) {}
inline void operator delete[](void*, void*) {}

/**
 * Copy `size` bytes from `src` to `dst`.
 * @returns `dst`.
 */
void* memcpy(void* dst, const void* src, size_t size);

/**
 * Compare byte arrays of length `size`.
 *
 * The sign of the result is the sign of the difference between
 * the first pair of bytes that differ; or 0 if they are equal.
 */
int memcmp(const void* a, const void* b, size_t size);
