// Define the link for exceptions without error codes
#define EXCEPTION_LNK(name, id)     \
.globl name                         ;\
.align 4                            ;\
name:                               ;\
    push $0                         ;\
    pushal                          ;\
    pushfl                          ;\
    pushl $id                       ;\
    call exception_handler          ;\
    addl $4, %esp                   ;\
    popfl                           ;\
    popal                           ;\
    addl $4, %esp                   ;\
    iret

// Defin the link for exceptions with error codes
#define EXCEPTION_LNK_ERR(name, id) \
.globl name                         ;\
.align 4                            ;\
name:                               ;\
    pushal                          ;\
    pushfl                          ;\
    pushl $id                       ;\
    call exception_handler          ;\
    addl $4, %esp                   ;\
    popfl                           ;\
    popal                           ;\
    addl $4, %esp                   ;\
    iret


// Link handlers for exceptions
EXCEPTION_LNK    (divide_error_exception,           0x00);
EXCEPTION_LNK    (debug_exception,                  0x01);
EXCEPTION_LNK    (nmi_interrupt,                    0x02);
EXCEPTION_LNK    (breakpoint_exception,             0x03);
EXCEPTION_LNK    (overflow_exception,               0x04);
EXCEPTION_LNK    (bound_range_exceed_exception,     0x05);
EXCEPTION_LNK    (invalid_opcode_exception,         0x06);
EXCEPTION_LNK    (device_not_avail_exception,       0x07);
EXCEPTION_LNK_ERR(double_fault_exception,           0x08);
EXCEPTION_LNK    (coprocessor_segment_overrun,      0x09);
EXCEPTION_LNK_ERR(invalid_tss_exception,            0x0A);
EXCEPTION_LNK_ERR(segment_not_present,              0x0B);
EXCEPTION_LNK_ERR(stack_fault_exception,            0x0C);
EXCEPTION_LNK_ERR(general_protection_exception,     0x0D);
EXCEPTION_LNK_ERR(page_fault_exception,             0x0E);
// 0x0F reserved by INTEL
EXCEPTION_LNK    (x87_fpu_floating_point_error,     0x10);
EXCEPTION_LNK_ERR(alignment_check_exception,        0x11);
EXCEPTION_LNK    (machine_check_exception,          0x12);
EXCEPTION_LNK    (simd_floating_point_exception,    0x13);
