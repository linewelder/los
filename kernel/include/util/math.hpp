#pragma once

template <typename T>
inline T max(T a, T b) {
    return a > b ? a : b;
}

template <typename T>
inline T min(T a, T b) {
    return a < b ? a : b;
}
