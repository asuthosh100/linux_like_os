/* keyboard.h - Defines used in interactions with the keyboard */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "types.h"

/* Port location keyboard connects to */
#define KEYBOARD_PORT       0x60
/* IRQ line for keyboard */
#define KEYBOARD_IRQ        1
/* Number of scancodes in conversion array */
#define NUM_SCANCODES       58
/* Number of characters in the buffer */
#define BUFFER_SIZE         129

/* Holds terminal information for switching */
typedef struct {
    struct pcb_t* term_pcb;
    uint8_t term_num;
    uint32_t cursor_x_pos;
    uint32_t cursor_y_pos;
    char term_char_buffer[BUFFER_SIZE];
    int term_char_buffer_idx;
    uint32_t term_vid_mem;
    uint8_t running;
    uint8_t running_pid;
    uint8_t enter_pressed;
} terminal_t;

volatile terminal_t terminals[3];

/* Initialize the keyboard */
void keyboard_init(void);

/* Handler for RTC interrupts */
extern void keyboard_interrupt_handler(void);

/* Dealing with enter and backspace */
void backspace_char();
void enter_char();

/* Clear buffer */
void clear_char_buf();
void clear_screen_buf();

/* Terminal Drivers*/
int32_t terminal_open(const uint8_t* filename);
int32_t terminal_read(int32_t inode, int32_t offset, int32_t nbytes, void* buf);
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t terminal_close(int32_t fd);

/* Switching terminals */
int32_t switch_terminals(int8_t t_num);
void init_terminals();
void vidmem_set(uint8_t t_num);
uint8_t get_curr_term();

#endif /* _KEYBOARD_H */
