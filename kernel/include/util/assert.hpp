#pragma once

void assertion_failed(const char* file, int line, const char* condition);

#define ASSERT(condition) \
    if (!static_cast<bool>(condition)) [[unlikely]] { \
        assertion_failed(__FILE__, __LINE__, #condition); \
    }
