/* rtc.h - Defines used in interactions with the RTC */

#ifndef _RTC_H
#define _RTC_H

#include "types.h"

#define RTC_PORT_CMD        0x70
#define RTC_PORT_DATA       0x71

/* RTC connected to IRQ 8 (as specified) */
#define RTC_IRQ             8

#define RTC_MAX_FREQ        1024
#define RTC_DEFAULT_FREQ    1024
#define RTC_MIN_FREQ        2
#define RTC_STATUS_REG_A    0x8A        /* Status Register A + disable NMI interrupts */
#define RTC_STATUS_REG_B    0x8B        /* Status Register B + disable NMI interrupts */
#define RTC_STATUS_REG_C    0x8C        /* Status Register C + disable NMI interrupts */

#define BIT_6_ON            0x40
#define BOT_4_MASK          0x0F
#define BIT_F0_MASK         0xF0

/* Initialize the RTC */
void rtc_init(void);

/* Handler for RTC interrupts */
extern void rtc_interrupt_handler(void);

/* Read from the RTC */
int32_t rtc_read(int32_t inode_num, int32_t off, int32_t nbytes, void* buf);

/* Write the RTC frequency */
int32_t rtc_write(int32_t fd, const void* buf,  int32_t nbytes);

/* Open RTC */
int32_t rtc_open(const uint8_t* fd);

/* Close RTC */
int32_t rtc_close(int32_t fd);

/* Change the frequency of RTC interrupts */
extern int32_t rtc_change_frequency(uint32_t new_rate);

#endif /* _RTC_H */
