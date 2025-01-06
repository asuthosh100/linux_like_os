// Define the link for interrupt handlers
#define INTR_LNK(name, func)        \
    .globl name                     ;\
    name:                           ;\
        pushal                      ;\
        pushfl                      ;\
        call func                   ;\
        popfl                       ;\
        popal                       ;\
        iret                    

// Link handlers for the interrupts
INTR_LNK(rtc_intr, rtc_interrupt_handler);
INTR_LNK(key_intr, keyboard_interrupt_handler);
INTR_LNK(pit_intr, pit_interrupt_handler);

// Link the test handler for the system calls
INTR_LNK(sys_intr, systemcall_handler_test);
