#include <arch/i386/exceptions.h>

#include <stdarg.h>

#include <arch/i386/asm.h>
#include <arch/i386/gdt.h>
#include <arch/i386/idt.h>
#include <kernel/kpanic.h>
#include <kernel/log.h>
#include <util/bits.h>

static constexpr char const* TABLE_NAME[] = {
    "GDT", "IDT", "LDT", "IDT"
};

static void report_selector(int selector) {
    bool external = get_bit(selector, 0);
    int index = get_bit_range(selector, 3, 13);
    const char* table = TABLE_NAME[get_bit_range(selector, 1, 2)];

    LOG_ERROR("Index 0x%x in %s%s\n",
        index, table,
        external ? ", exception originated externally to the processor" : "");
}

enum class ErrorCodeType {
    NONE,
    RAW,
    SELECTOR,
};

static void unhandled_exception(
    const char* message,
    idt::InterruptFrame* frame,
    ErrorCodeType error_code_type = ErrorCodeType::NONE,
    int error_code = 0)
{
    LOG_ERROR("Unhandled exception occured! %s", message);
    LOG_ERROR("IP=0x%x CS=0x%x FLAGS=0x%x", frame->ip, frame->cs, frame->flags);

    switch (error_code_type) {
    case ErrorCodeType::RAW:
        LOG_ERROR("Error code: %x", error_code);
        break;
    case ErrorCodeType::SELECTOR:
        report_selector(error_code);
        break;
    case ErrorCodeType::NONE:
        break;
    }
    kpanic("Unhandled exception: %s", message);
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
    unhandled_exception("Page fault (Error code: %x)", frame,
        ErrorCodeType::RAW, error_code);
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
