#ifndef INTERRUPTS_LINK_H
#define INTERRUPTS_LINK_H

// Delcare the interrupt handler links
extern void rtc_intr();
extern void key_intr();
extern void pit_intr();

// Declare the system call link
extern void sys_intr();

#endif
