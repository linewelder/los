#pragma once

#include <stdarg.h>

/** Supports only %d %x %c %s formatters without precision and width. */
int vprintf(const char* format, va_list args);

/** Supports only %d %x %c %s formatters without precision and width. */
int printf(const char* format, ...) __attribute__((format(printf, 1, 2)));
