#ifndef EXCEPTION_WRAP_H
#define EXCEPTION_WRAP_H

// Declare the exception handlers
extern void divide_error_exception();
extern void debug_exception();
extern void nmi_interrupt();
extern void breakpoint_exception();
extern void overflow_exception();   
extern void bound_range_exceed_exception();
extern void invalid_opcode_exception();
extern void device_not_avail_exception();
extern void double_fault_exception();
extern void coprocessor_segment_overrun();
extern void invalid_tss_exception();
extern void segment_not_present();
extern void stack_fault_exception();
extern void general_protection_exception();
extern void page_fault_exception();
// idt[0x0F] reserved by INTEL
extern void x87_fpu_floating_point_error();
extern void alignment_check_exception();
extern void machine_check_exception();
extern void simd_floating_point_exception();

#endif

