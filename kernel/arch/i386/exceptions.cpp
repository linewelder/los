#include <arch/i386/exceptions.hpp>

#include <arch/i386/asm.hpp>
#include <arch/i386/gdt.hpp>
#include <arch/i386/idt.hpp>
#include <kernel/kpanic.hpp>
#include <kernel/log.hpp>
#include <util/array.hpp>
#include <util/bits.hpp>

static constexpr Array TABLE_NAME = {
    "GDT", "IDT", "LDT", "IDT"
};

static void report_selector(int selector) {
    bool external = get_bit(selector, 0);
    int index = get_bit_range(selector, 3, 13);
    const char* table = TABLE_NAME[get_bit_range(selector, 1, 2)];

    LOG_ERROR("Index 0x{:x} in {}{}\n",
        index, table,
        external ? ", exception originated externally to the processor" : "");
}

static void report_page_fault(int selector) {
    bool write = get_bit(selector, 1);
    bool execute = get_bit(selector, 4);
    bool userspace = get_bit(selector, 2);

    const char* action;
    if (execute) {
        action = "execute";
    } else if (write) {
        action = "write to";
    } else {
        action = "read";
    }

    const char* ring = userspace ? "userspace" : "kernel";

    size_t address;
    asm volatile("movl %%cr2, %0" : "=r"(address));
    LOG_ERROR("Trying to {} address {} by {}",
        action, reinterpret_cast<void*>(address), ring);

    if (get_bit(selector, 0)) {
        LOG_ERROR("Page present");
    } else {
        LOG_ERROR("Page unpresent");
    }

    // Flags that we do not know what they mean.

    if (get_bit(selector, 3)) {
        LOG_ERROR("Reserved write");
    }

    if (get_bit(selector, 5)) {
        LOG_ERROR("Error caused by protection-key violation");
    }

    if (get_bit(selector, 6)) {
        LOG_ERROR("Error caused by shadow stack access");
    }

    if (get_bit(selector, 15)) {
        LOG_ERROR("SGX violation");
    }
}

enum class ErrorCodeType {
    NONE,
    RAW,
    SELECTOR,
    PAGE_FAULT,
};

static void unhandled_exception(
    const char* message,
    idt::InterruptFrame* frame,
    ErrorCodeType error_code_type = ErrorCodeType::NONE,
    int error_code = 0)
{
    LOG_ERROR("Unhandled exception occured! {}", message);
    LOG_ERROR("IP=0x{:x} CS=0x{:x} FLAGS=0x{:x}", frame->ip, frame->cs, frame->flags);

    switch (error_code_type) {
    case ErrorCodeType::RAW:
        LOG_ERROR("Error code: {:x}", error_code);
        break;
    case ErrorCodeType::SELECTOR:
        report_selector(error_code);
        break;
    case ErrorCodeType::PAGE_FAULT:
        report_page_fault(error_code);
        break;
    case ErrorCodeType::NONE:
        break;
    }
    kpanic("Unhandled exception: {}", message);
}

__attribute__((interrupt))
static void division_error(idt::InterruptFrame* frame) {
    unhandled_exception("Division error", frame);
}

__attribute__((interrupt))
static void bound_range_exceeded(idt::InterruptFrame* frame) {
    unhandled_exception("Bound range exceeded", frame);
}

__attribute__((interrupt))
static void invalid_opcode(idt::InterruptFrame* frame) {
    unhandled_exception("Invalid opcode", frame);
}

__attribute__((interrupt))
static void device_not_available(idt::InterruptFrame* frame) {
    unhandled_exception("No FPU available", frame);
}

__attribute__((interrupt))
static void double_fault(idt::InterruptFrame* frame) {
    unhandled_exception("Error handling another exception", frame);
}

__attribute__((interrupt))
static void invalid_tss(idt::InterruptFrame* frame, int selector) {
    unhandled_exception("Invalid Task State Segment", frame,
        ErrorCodeType::SELECTOR, selector);
}

__attribute__((interrupt))
static void segment_not_present(idt::InterruptFrame* frame, int selector) {
    unhandled_exception("Segment not present", frame,
        ErrorCodeType::SELECTOR, selector);
}

__attribute__((interrupt))
static void stack_segment_fault(idt::InterruptFrame* frame, int selector) {
    unhandled_exception("Stack segment fault", frame,
        ErrorCodeType::SELECTOR, selector);
}

__attribute__((interrupt))
static void general_protection_fault(idt::InterruptFrame* frame, int selector) {
    unhandled_exception("General protection fault", frame,
        ErrorCodeType::SELECTOR, selector);
}

__attribute__((interrupt))
static void page_fault(idt::InterruptFrame* frame, int error_code) {
    unhandled_exception("Page fault", frame,
        ErrorCodeType::PAGE_FAULT, error_code);
}

__attribute__((interrupt))
static void x87_exception(idt::InterruptFrame* frame) {
    unhandled_exception("x87 exception", frame);
}

__attribute__((interrupt))
static void alignment_check(idt::InterruptFrame* frame, int error_code) {
    unhandled_exception("Alignment check failed", frame,
        ErrorCodeType::RAW, error_code);
}

__attribute__((interrupt))
static void simd_exception(idt::InterruptFrame* frame) {
    unhandled_exception("SIMD exception", frame);
}

__attribute__((interrupt))
static void virtualization_exception(idt::InterruptFrame* frame) {
    unhandled_exception("Virtualization exception", frame);
}

__attribute__((interrupt))
static void control_protection_exception(idt::InterruptFrame* frame, int error_code) {
    unhandled_exception("Control protection exception", frame,
        ErrorCodeType::RAW, error_code);
}

__attribute__((interrupt))
static void hypervisor_injection_exception(idt::InterruptFrame* frame) {
    unhandled_exception("Hypervisor injection exception", frame);
}

__attribute__((interrupt))
static void vmm_communication_exception(idt::InterruptFrame* frame, int error_code) {
    unhandled_exception("VMM communication exception", frame,
        ErrorCodeType::RAW, error_code);
}

__attribute__((interrupt))
static void security_exception(idt::InterruptFrame* frame, int error_code) {
    unhandled_exception("Security exception", frame,
        ErrorCodeType::RAW, error_code);
}

void register_exception_handlers() {
    idt::register_trap(0x00, division_error);
    idt::register_trap(0x05, bound_range_exceeded);
    idt::register_trap(0x06, invalid_opcode);
    idt::register_trap(0x07, device_not_available);
    idt::register_trap(0x08, double_fault);
    idt::register_trap(0x0a, invalid_tss);
    idt::register_trap(0x0b, segment_not_present);
    idt::register_trap(0x0c, stack_segment_fault);
    idt::register_trap(0x0d, general_protection_fault);
    idt::register_trap(0x0e, page_fault);
    idt::register_trap(0x10, x87_exception);
    idt::register_trap(0x11, alignment_check);
    idt::register_trap(0x13, simd_exception);
    idt::register_trap(0x14, virtualization_exception);
    idt::register_trap(0x15, control_protection_exception);
    idt::register_trap(0x1c, hypervisor_injection_exception);
    idt::register_trap(0x1d, vmm_communication_exception);
    idt::register_trap(0x1e, security_exception);
}
