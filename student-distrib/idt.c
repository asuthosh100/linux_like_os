
#include "idt.h"

/* Array of exception names */
char * exceptions[] = {
    "Divide Error",
    "Debug Exception",
    "NMI Interrupt",
    "Breakpoint Exception",
    "Overflow Exception",
    "BOUND Range Exceeded Exception",
    "Invalid Opcode Exception",
    "Device Not Available Exception",
    "Double Fault Exception",
    "Coprocessor Segment Exception",
    "Invalid TSS Exception",
    "Segment Not Present",
    "Stack Fault Exception",
    "General Protection Exception",
    "Page-Fault Exception",
    "Reserved",
    "x87 FPU Floating Point Exception",
    "Alignment Check Exception",
    "Machine Check Exception",
    "SIMD Floating-Point Exception",
};


/* void idt_init()
 * 
 * Initializes the values for the IDT table
 * Inputs: None
 * Outputs: IDT table filled with exception and interrupt handlers
 * Return value: None
 */
void idt_init(){
    int i;
    for(i=0; i < NUM_VEC; i++){
        
        // All magic numbers are gotten from IA-32 Maual, 5.11 IDT DESCRIPTORS

        idt[i].seg_selector = KERNEL_CS;        // Handlers run in kernel space

        idt[i].reserved0 = 0x0;
        idt[i].reserved1 = 0x1;
        idt[i].reserved2 = 0x1;
        idt[i].reserved3 = 0x0;
        idt[i].reserved4 = 0x0;

        idt[i].size = 0x1;

        if(i == SYSTEM_CALL_VEC)
            idt[i].dpl = 0x3;                     // Set to three for system calls
        else    
            idt[i].dpl = 0x0;                     // Set to zero for everything else
        
        if (i < NUM_EXCEPTIONS && i != RESERVED_VEC)
            idt[i].present = 0x1;                 // Set to present for all exceptions
        else    
            idt[i].present = 0x0;                 // except for the reserved vector
    }

    // Set exceptions - locations are from IA-32 Maual, 5.14 EXCEPTION AND INTERRUPT REFERENCE

    SET_IDT_ENTRY(idt[0x0], divide_error_exception);
    SET_IDT_ENTRY(idt[0x1], debug_exception);
    SET_IDT_ENTRY(idt[0x2], nmi_interrupt);
    SET_IDT_ENTRY(idt[0x3], breakpoint_exception);
    SET_IDT_ENTRY(idt[0x4], overflow_exception);
    SET_IDT_ENTRY(idt[0x5], bound_range_exceed_exception);
    SET_IDT_ENTRY(idt[0x6], invalid_opcode_exception);
    SET_IDT_ENTRY(idt[0x7], device_not_avail_exception);
    SET_IDT_ENTRY(idt[0x8], double_fault_exception);
    SET_IDT_ENTRY(idt[0x9], coprocessor_segment_overrun);
    SET_IDT_ENTRY(idt[0xA], invalid_tss_exception);
    SET_IDT_ENTRY(idt[0xB], segment_not_present);
    SET_IDT_ENTRY(idt[0xC], stack_fault_exception);
    SET_IDT_ENTRY(idt[0xD], general_protection_exception);
    SET_IDT_ENTRY(idt[0xE], page_fault_exception);
    // idt[] reserved by INTEL
    SET_IDT_ENTRY(idt[0x10], x87_fpu_floating_point_error);
    SET_IDT_ENTRY(idt[0x11], alignment_check_exception);
    SET_IDT_ENTRY(idt[0x12], machine_check_exception);
    SET_IDT_ENTRY(idt[0x13], simd_floating_point_exception);


    // Set interrupts
    // Set all IDT entries for interrupts to present and all reserved3 values to 1 
    // Numbers are from IA-32 Maual, 5.11 IDT DESCRIPTORS
    idt[PIT_VEC].present = 0x1;                  
    idt[PIT_VEC].reserved3 = 0x1; 
    SET_IDT_ENTRY(idt[PIT_VEC], pit_intr);

    idt[RTC_VEC].present = 0x1;                  
    idt[RTC_VEC].reserved3 = 0x1; 
    SET_IDT_ENTRY(idt[RTC_VEC], rtc_intr);

    idt[KEYBOARD_VEC].present = 0x1;
    idt[KEYBOARD_VEC].reserved3 = 0x1;
    SET_IDT_ENTRY(idt[KEYBOARD_VEC], key_intr);

    // Set system calls
    // Set system call entry to present

    idt[SYSTEM_CALL_VEC].present = 0x1;
    SET_IDT_ENTRY(idt[SYSTEM_CALL_VEC], system_call);

    // Load the IDT
    lidt(idt_desc_ptr);
}

/* system call handler for testing
 * 
 * Returns whether a system call occurs when the IDT entry is called
 * Inputs: None
 * Outputs: Prints statement if working
 * Return value: None
 */
void systemcall_handler_test(){
    //
    printf("System Call was called.\n");
}

/* Exception interrupt handler
 * 
 * Handles exception calls by printing the exception to screen
 * Inputs: None
 * Outputs: Prints statement if working
 * Return value: None
 */
void exception_handler(uint32_t id,  uint32_t flags, struct pushal_t pushal, uint32_t err) {

    cli();

    if(id < NUM_EXCEPTIONS)
        printf("Exception: %s\n", exceptions[id]);
    
    // Used for debugging purposes
    // printf("PUSHAL Information: \n");
    // printf("EAX=0x%x, EBX=0x%x, ECX=0x%x, EDX=0x%x\n", pushal.eax, pushal.ebx, pushal.ecx, pushal.edx);
    // printf("ESI=0x%x, EDI=0x%x, EBP=0x%x, ESP=0x%x\n", pushal.esi, pushal.edi, pushal.ebp, pushal.esp);

    // printf("\n Error Code: %x\n", err);

    while(1);

    sti();
}
