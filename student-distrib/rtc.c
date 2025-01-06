/* rtc.c - Functions to interact with the RTC */

#include "rtc.h"
#include "lib.h"
#include "i8259.h"

uint32_t rtc_int_count = 0;
uint32_t rtc_global_count = RTC_DEFAULT_FREQ/RTC_MIN_FREQ;  // Initialize RTC interrupt frequency to 2 Hz
uint32_t rtc_freq = RTC_MIN_FREQ;                           // Initialize RTC interrupt frequency to minimum (2 Hz)

/* rtc_init
 *   DESCRIPTION: This function initializes the RTC periodic interrupt
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: RTC is set to generate periodic interrupts
 */  
void rtc_init(void) {
    /* Credit: https://wiki.osdev.org/RTC for logic */
    /* Declare variable(s) */
    char prev;  

    /* Turn on periodic interrupt for RTC */
    outb(RTC_STATUS_REG_B, RTC_PORT_CMD);   /* Select register B and disable NMI */
    prev = inb(RTC_PORT_DATA);              /* Read current value of register B */
    outb(RTC_STATUS_REG_B, RTC_PORT_CMD);   /* Set index again since read sets index to register D */
    outb(prev | BIT_6_ON, RTC_PORT_DATA);   /* Write previous value ORed with 0x40 (turn on bit 6 of register B) */
    
    /* Set RTC IRQ to 1024 Hz */
    // outb(RTC_STATUS_REG_A, RTC_PORT_CMD);
    // outb(RTC_DEFAULT_FREQ, RTC_PORT_DATA);

    /* Enable interrupts for the RTC */
    enable_irq(RTC_IRQ);

}


/* rtc_init
 *   DESCRIPTION: This function handles interrupts from the RTC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: When test_interrupts is enabled, monitor flashes
 */  
extern void rtc_interrupt_handler(void) {
    // test_interrupts();
    // Start critical section
    cli();
    outb(RTC_STATUS_REG_C, RTC_PORT_CMD);   /* Select Register C */
    inb(RTC_PORT_DATA);                     /* Throw away contents */

    /* Check whether another cycle has passed */
    rtc_global_count--;
    // If interrupt has occurred, raise flag and reset counter
    if(rtc_global_count == 0) {
        rtc_int_count = 1;
        rtc_global_count = RTC_DEFAULT_FREQ/rtc_freq;
        // printf("%d", 1);
    }

    // End critical section
    sti();
    
    // Signal end of interrupt
    send_eoi(RTC_IRQ);

}

/* rtc_read
 *   DESCRIPTION: This function reads the RTC frequency. 
 *   INPUTS: int32_t ls - file to read from
 *           void* buf - buffer to store what to set
 *           int32_t nbytes - number of bytes to read
 *   OUTPUTS: none
 *   RETURN VALUE: Returns 0 on success (always)
 *   SIDE EFFECTS: Blocks until interrupt from the RTC 
 */  
int32_t rtc_read(int32_t inode_num, int32_t off, int32_t nbytes, void* buf) {
    // Set RTC interrupt flag to 0
    rtc_int_count = 0;
    // Block until interrupt is raised by RTC
    while(rtc_int_count == 0);
    return 0;
}

/* rtc_write
 *   DESCRIPTION: This function writes new RTC frequency.
 *   INPUTS: int32_t fd - file to read from
 *           const void* buf - buffer to store frequency to set
 *           int32_t nbytes - number of bytes to read
 *   OUTPUTS: none
 *   RETURN VALUE: Returns 0 on success, -1 on failure
 *   SIDE EFFECTS: Changes frequency of RTC interrupts 
 */  
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes) {
    // Declare local variables
    int32_t to_ret;
    int32_t freq;
    // Get new frequency, change RTC frequency, and return success (or fail)
    freq = *((int32_t*)buf);
    to_ret = rtc_change_frequency(freq);
    return to_ret;
}

/* rtc_open
 *   DESCRIPTION: This function opens the RTC.
 *   INPUTS: const uint8_t* fd - file to read from
 *   OUTPUTS: none
 *   RETURN VALUE: Returns 0 on success (always)
 *   SIDE EFFECTS: Sets frequency of RTC interrupts to 2 Hz
 */  
int32_t rtc_open(const uint8_t* fd) {
    // Set RTC frequency to 2 Hz and return pass
    rtc_change_frequency(RTC_MIN_FREQ);
    return 0;
}

/* rtc_close
 *   DESCRIPTION: This function closes the RTC.
 *   INPUTS: int32_t fd - file to read from
 *   OUTPUTS: none
 *   RETURN VALUE: Returns 0 on success (always)
 *   SIDE EFFECTS: Sets frequency of RTC interrupts to 2 Hz
 */  
int32_t rtc_close(int32_t fd) {
    // Set RTC frequency to 2 Hz and return pass
    rtc_change_frequency(RTC_MIN_FREQ);
    return 0;
}

/* rtc_change_frequency
 *   DESCRIPTION: This function changes the RTC frequency.
 *   INPUTS: uint32_t new_freq - frequency to set RTC interrupts to
 *   OUTPUTS: none
 *   RETURN VALUE: Returns 0 on success, -1 on failure
 *   SIDE EFFECTS: Changes frequency of RTC interrupts (and sets global
 *                 variables accordingly)
 */  
int32_t rtc_change_frequency(uint32_t new_freq) {
    // Start of critical section
    cli();

    // Check whether frequency to set is out of bounds or not a power of 2 (return -1 if so)
    if(new_freq < RTC_MIN_FREQ || new_freq > RTC_MAX_FREQ || (new_freq & (new_freq - 1))) {
        return -1;
    }

    // Update frequency for RTC interrupts
    rtc_freq = new_freq;

    // Set new rate for RTC interrupts for counter
    rtc_global_count = RTC_MAX_FREQ/rtc_freq;

    // End of critical section
    sti();

    // Return 0 on success
    return 0;
}
