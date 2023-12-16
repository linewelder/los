#pragma once

#include <util/util.hpp>

template <typename T>
inline T max(T a, IdentityType<T> b) {
    return a > b ? a : b;
}

template <typename T>
inline T min(T a, IdentityType<T> b) {
    return a < b ? a : b;
}
