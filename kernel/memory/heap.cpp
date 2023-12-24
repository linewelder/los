/**
 * @file
 * Operator new and delete definitions.
 */

#include <kernel/kpanic.hpp>
#include <memory/liballoc.h>

void* operator new(size_t size) {
    if (auto ptr = kmalloc(size)) {
        return ptr;
    }

    kpanic("Failed to allocate {} bytes on the heap", size);
}

void* operator new[](size_t size) {
    if (auto ptr = kmalloc(size)) {
        return ptr;
    }

    kpanic("Failed to allocate {} bytes on the heap", size);
}

void operator delete(void* ptr) {
    kfree(ptr);
}

void operator delete(void* ptr, size_t) {
    kfree(ptr);
}

void operator delete[](void* ptr) {
    kfree(ptr);
}

void operator delete[](void* ptr, size_t) {
    kfree(ptr);
}
