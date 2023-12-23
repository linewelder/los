#include <util/assert.hpp>

#include <kernel/kpanic.hpp>

[[noreturn]]
void assertion_failed(const char* file, int line, const char* condition) {
    kpanic("[{}:{}] Assertion failed: {}", file, line, condition);
}
