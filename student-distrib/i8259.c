/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = INIT_MASK; /* IRQs 0-7  */
uint8_t slave_mask = INIT_MASK;  /* IRQs 8-15 */

/* i8259_init
 *   DESCRIPTION: This function initializes the 8259 PIC(s)
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: master and slave PICs set to generate interrupts
 *                 0x20-0x27 and 0x28-0x2F respectively, and slave
 *                 PIC connected to line 2 of master PIC.
 */  
void i8259_init(void) {
    /* Credit: https://wiki.osdev.org/RTC for logic */
    /* Mask all interrupts on primary and secondary PICs */
    outb(master_mask, MASTER_8259_PORT_DATA);
    outb(slave_mask, SLAVE_8259_PORT_DATA);

    /* Initialize primary and secondary PICS */
    outb(ICW1, MASTER_8259_PORT_CMD);
    outb(ICW1, SLAVE_8259_PORT_CMD);
    outb(ICW2_MASTER, MASTER_8259_PORT_DATA);
    outb(ICW2_SLAVE, SLAVE_8259_PORT_DATA);
    outb(ICW3_MASTER, MASTER_8259_PORT_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_PORT_DATA);
    outb(ICW4, MASTER_8259_PORT_DATA);
    outb(ICW4, SLAVE_8259_PORT_DATA);

    /* Mask all interrupts on primary and secondary PICs */
    outb(master_mask, MASTER_8259_PORT_DATA);
    outb(slave_mask, SLAVE_8259_PORT_DATA);

    /* Enable slave 8259's IRQs to be processed on master 8259 */
    enable_irq(MASTER_SLAVE_PORT_NUM);
}

/* enable_irq
 *   DESCRIPTION: This function enables (unmasks) the specified IRQ
 *   INPUTS: irq_num - IRQ to be unmasked; between 0-15
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Specified IRQ bit is set to 0 and sent to PIC
 */
void enable_irq(uint32_t irq_num) {
    /* Credit: https://wiki.osdev.org/RTC for logic */
    /* Declare variables */
    uint32_t port;
    uint32_t value;

    /* Check for valid irq_num */
    if(irq_num > MAX_IRQ_NUM){
        return;
    }
 
    if(irq_num < MASTER_MAX_IRQ_NUM) {
        /* if irq_num < 8, then unmask specifed IRQ on master 8259 */
        port = MASTER_8259_PORT_DATA;
    } else {
         /* else (irq_num is >= 8), then unmask specifed IRQ on slave 8259 */
        port = SLAVE_8259_PORT_DATA;
        irq_num -= MASTER_MAX_IRQ_NUM;
    }
    value = inb(port) & ~(1 << irq_num);
    outb(value, port);  
}

/* disable_irq
 *   DESCRIPTION: This function disables (masks) the specified IRQ
 *   INPUTS: irq_num - IRQ to be masked; between 0-15
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Specified IRQ bit is set to 1 and sent to PIC
 */
void disable_irq(uint32_t irq_num) {
    /* Credit: https://wiki.osdev.org/RTC for logic */
    /* Declare variables */
    uint32_t port;
    uint32_t value;

    /* Check for valid irq_num */
    if(irq_num > MAX_IRQ_NUM){
        return;
    }
 
    if(irq_num < MASTER_MAX_IRQ_NUM) {
         /* if irq_num < 8, then mask specifed IRQ on master 8259 */
        port = MASTER_8259_PORT_DATA;
    } else {
        /* else (irq_num is >= 8), then mask specifed IRQ on slave 8259 */
        port = SLAVE_8259_PORT_DATA;
        irq_num -= MASTER_MAX_IRQ_NUM;
    }
    value = inb(port) | (1 << irq_num);
    outb(value, port); 

}

/* send_eoi
 *   DESCRIPTION: This function sends an end-of-interrupt signal for the specified IRQ
 *   INPUTS: irq_num - IRQ to send EOI for; between 0-15
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: EOI is sent to PIC (and if IRQ is on slave, then EOI is also sent
 *                 for line that connects slave PIC to master).
 */
void send_eoi(uint32_t irq_num) {
    /* Check for valid irq_num */
    if(irq_num > MAX_IRQ_NUM){
        return;
    }

    /* If irq_num is >= 8 (on secondary PIC), send EOI for both; else send for primary */
    if(irq_num < MASTER_MAX_IRQ_NUM) {
        outb(irq_num | EOI, MASTER_8259_PORT_CMD);                          /* Send EOI to master 8259 (OR as specified by spec) */
    } else {
        outb(EOI | (irq_num - MASTER_MAX_IRQ_NUM), SLAVE_8259_PORT_CMD);    /* Send EOI to slave 8259 (OR as specified by spec) */
        outb(EOI | MASTER_SLAVE_PORT_NUM, MASTER_8259_PORT_CMD);            /* Send EOI to master 8259 (OR as specified by spec) */
    }

}
