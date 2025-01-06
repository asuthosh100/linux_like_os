#define ASM 1
#include "syscall_link.h"
#include "x86_desc.h"

.text
.globl system_call

#Implemented this based on the system call lecture slide, check out the lecture 19 slide 18 for accurate information. 
syscall_jmp_table : 
    .long 0x0
    .long sys_halt
    .long execute
    .long read
    .long write
    .long open
    .long close
    .long getargs
    .long vidmap
    # .long set_handler
    # .long sigreturn

system_call : 

# saves register to stack (I pushed these registers to replicate kernel stack after call to the jump table)
    pushl %edx
    pushl %ecx
    pushl %ebx 
    pushl %ebp
    pushl %edi 
    pushl %esi
    pushfl


    pushl %edx
    pushl %ecx
    pushl %ebx 

    pushl %eax
    movw $KERNEL_DS, %ax
    movw %ax, %ds
    popl %eax

# check for a valid system call, system calls for our MP are basically numbered from 1 to 10 and eax contains the system call number
   
   cmpl $1, %eax
   jl invalid
   cmpl $10, %eax 
   jg invalid

    # HARDCODE TO TEST EXECUTE.
    # movl $0, %eax
# call specific routine using a jump table
   call *syscall_jmp_table(, %eax, 4)

DONE: 
    addl $12, %esp 
    

    popfl
    popl %esi
    popl %edi 
    popl %ebp 
    popl %ebx
    popl %ecx
    popl %edx

    IRET

invalid : 
    movl $-1, %eax
    jmp DONE
    

.globl halt_return
halt_return:
    pushl %ebp
    movl %esp, %ebp;

    movl 16(%ebp), %ebx     # Get status
    movl %ebx, %eax
    movl 12(%ebp), %ebx     # Get ESP
    movl %ebx, %esp
    movl 8(%ebp), %ebx      # Get EBP
    movl %ebx, %ebp

    leave
    ret
    
