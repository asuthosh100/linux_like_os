#ifndef PIT_H
#define PIT_H

#include "lib.h"
#include "types.h" 
#include "i8259.h" 
#include "syscallhandler.h"

#define PIT_CMD_REG 0x43
#define PIT_CH0_PORT 0x40
#define _100hz 11932       // 1.193182 mHz / 11932 = 100 Hz
#define PIT_IRQ 0          // PIT is connected to IRQO
#define PARENT_SHELL_NUM 3

/* Variable to store the current active process */
extern pcb_t* curr_active_process;

/* Initializes the pit */
void pit_init(void);

/* Deals with pit interrupts */
extern void pit_interrupt_handler(void);

/* Getter for round_robin_term */
uint8_t get_round_robin_term();

#endif // PIT_H
