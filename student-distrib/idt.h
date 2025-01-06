#ifndef IDT_H
#define IDT_H

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "exception_wrap.h"
#include "interrupts_link.h"
#include "keyboard.h"
#include "syscall_link.h"

#define RESERVED_VEC        0x0F
#define SYSTEM_CALL_VEC     0x80
#define KEYBOARD_VEC        0x21
#define RTC_VEC             0x28
#define NUM_EXCEPTIONS      20
#define PIT_VEC             0x20

// This gets pushed on stack when pushal is called in exception wrap
struct pushal_t {  
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
};

// Declare functions.
void idt_init();
void exception_handler(uint32_t id,  uint32_t flags, struct pushal_t pushal, uint32_t err);

#endif /* _IDT_H */

