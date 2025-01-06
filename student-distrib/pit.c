#include "pit.h"
#include "keyboard.h"
#include "paging.h"
#include "filesys.h"
#include "x86_desc.h"

pcb_t* curr_active_process = NULL;
static uint8_t round_robin_term = 0;

/* MP3.5!!!
 * pit_init
 *   DESCRIPTION: Initializes the PIT to mode 3 on channel 0 with a specific frequency.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Modifies PIT command register and channel 0 frequency, affecting system timing and interrupt behavior.
 */
void pit_init(void) {
    // Change to square-wave form (mode 3)
    outb(0x37, PIT_CMD_REG);

    // The divisor is shifted by 8 bits and sent in two parts because the PIT's data port can only accept 8 bits at a time, while the divisor is a 16-bit value.
    outb((uint8_t) _100hz, PIT_CH0_PORT); 
    outb((uint8_t) _100hz >> 8, PIT_CH0_PORT);  

    // Enable IRQ and return
    enable_irq(PIT_IRQ);
    
    return;
}

/* MP3.5!!!
 * pit_interrupt_handler
*    DESCRIPTION: This function handles interrupts from the PIT
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Switches process that is running
 */  
extern void pit_interrupt_handler(void) {
    // Start of critical section
    cli();
    
    // Send EOI for PIT_IRQ
    send_eoi(PIT_IRQ);

    // Get process that is currently running and save ESP/EBP for context switch
    curr_active_process = terminal_pcb_top[round_robin_term];

    if(curr_active_process != NULL) {
        asm volatile(
            "movl %%esp, %0;"
            "movl %%ebp, %1;"
            : "=r"(curr_active_process->ESP), "=r"(curr_active_process->EBP) // Output operands
            :
            : "memory", "cc"
        );
    }

    // Update which terminal's process we are running
    round_robin_term = (round_robin_term+1) % PARENT_SHELL_NUM;

    // If the array is NULL, that means there is no active process and we can just return.
    if(curr_active_process == NULL){
        // End of critical section and return
        sti();
        return;
    }

    // If nothing is running, we can return (new shells are created in execute if needed)
    if(terminal_pcb_top[round_robin_term] == NULL) {
        // End of critical section and return
        sti();
        return;

    } else {

        // Sets up video memory for current process
        vidmem_set(round_robin_term);

        // Grab new process to execute
        curr_active_process = (terminal_pcb_top[round_robin_term]);

        // Retunr if NULL
        if(curr_active_process == NULL) {
            // End of critical section and return
            sti();
            return;
        }

        // Setup paging for new process
        paging_for_execute(curr_active_process->PID);

        // Null check 
        if(curr_active_process == NULL) {
            // send_eoi(PIT_IRQ);
            sti();
            return;
        }

        // Save ss0 and esp0 for context switch
        tss.ss0 = KERNEL_DS;
        tss.esp0 = KERNEL_END_ADDR - ((curr_active_process->PID) * KERNEL_TASK_SIZE) - sizeof(curr_active_process);

        // End of critical section
        sti();

        // Context switch
        asm volatile(
            "movl %0, %%esp ;" 
            "movl %1, %%ebp ;"
            :
            : "r" (curr_active_process->ESP), "r" (curr_active_process->EBP)
            : "esp", "ebp"
        );

    }
    
    // End of critical section
    sti();

}

/* MP3.5!!!
 * get_round_robin_term
 *   DESCRIPTION: Getter for round_robin_term
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: round_robin_term - current termnial to execute process
 *   SIDE EFFECTS: 
 */ 
uint8_t get_round_robin_term(){
    return round_robin_term;
}
