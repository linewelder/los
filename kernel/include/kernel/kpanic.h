#pragma once

[[noreturn]]
void kpanic(const char* message, ...) __attribute__((format(printf, 1, 2)));
