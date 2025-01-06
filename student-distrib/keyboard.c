/* keyboard.c - Functions to interact with the keyboard */

#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
#include "syscallhandler.h"
#include "paging.h"
#include "pit.h"

#define VIDEO       0xB8000
#define NUM_COLS    80
#define NUM_ROWS    25
#define ATTRIB      0x7

uint8_t curr_term_num = 0;

// Array to store values to correctly convert inputted scancodes 
char scancode_conversion[NUM_SCANCODES][2] =
{
    {0, 0}, {0, 0}, {'1', '!'}, {'2', '@'}, {'3', '#'}, {'4', '$'}, {'5', '%'}, {'6', '^'}, {'7', '&'}, {'8','*'}, {'9','('}, {'0', ')'},
    {'-', '_'} , {'=', '+'}, {8, 8}, {' ', ' '}, {'q', 'Q'}, {'w', 'W'}, {'e', 'E'}, {'r', 'R'}, {'t', 'T'}, {'y', 'Y'}, {'u', 'U'}, 
    {'i', 'I'}, {'o', 'O'}, {'p', 'P'}, {'[', '{'}, { ']','}' }, {10, 10}, /* (13,13) -> (10,10) (newline)*/ {0, 0}, {'a', 'A'}, {'s', 'S'}, 
    {'d', 'D'}, {'f', 'F'}, {'g', 'G'}, {'h', 'H'}, {'j', 'J'}, {'k', 'K'}, {'l', 'L'}, {';', ':'}, {39, 34}, {'`', '~'}, {0, 0}, {'\\', '|'}, 
    {'z', 'Z'}, {'x', 'X'}, {'c', 'C'}, {'v', 'V'}, {'b', 'B'}, {'n', 'N'}, {'m', 'M'}, {',', '<'}, {'.', '>'}, {'/', '?'}, {0, 0}, {0, 0}, {0, 0}, {' ', ' '}
};

// Variables to store if special keys are pressed
uint8_t shift_pressed = 0;
uint8_t caps_lock_pressed = 0;
uint8_t alt_pressed = 0;
uint8_t ctrl_pressed = 0;
uint8_t onetosix = 0;

// Buffer for characters
char char_buffer[BUFFER_SIZE];
int char_buffer_idx = 0;    // keeps track of the next input in the buffer
char screen_buffer[1024];
int screen_buffer_idx;
int last_ent = 0;
int first_t1_switch = 0;
int first_t2_switch = 0;
int t1_execute = 0;
int t2_execute = 0;


/* keyboard_init
 *   DESCRIPTION: This function initializes the keyboard
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: keyboard is set to generate interrupts
 */  
void keyboard_init() {
    enable_irq(KEYBOARD_IRQ);
}

/* keyboard_interrupt_handler
 *   DESCRIPTION: This function handles interrupts from 
 *                the keyboard
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: inputted key is echoed to monitor
 */  
void keyboard_interrupt_handler(void) {

    // Begin critical section
    cli();
    
    // To store character to add to buffer 
    char pressed_key;

    // Read inputted key from keyboard 
    uint8_t keycode = 0;
    
    keycode = inb(KEYBOARD_PORT);

    // Clear char buffer if newline
    if(last_ent){
        last_ent = 0;
        clear_char_buf();
    }

    // Convert keycode (scancode) to correct character to display and add to buffer if not special character
    switch(keycode) {
        case 0x2A: //Left shift pressed
        case 0x36: //Right shift pressed
            shift_pressed = 1;
            break;
        case 0xAA: //Left shift released
        case 0xB6: //Right shift released
            shift_pressed = 0;
            break;
        case 0x3A: //Capslock pressed
            caps_lock_pressed = !(caps_lock_pressed);
            break;
        case 0x38: //alt pressed
            alt_pressed = 1;
            break;
        case 0xB8: //alt released
            alt_pressed = 0;
            break;
        case 0x1D: //ctrl pressed
            ctrl_pressed = 1;
            break;
        case 0x9D: //ctrl released
            ctrl_pressed = 0;
            break;
        case 0x0E: //backspace pressed
            if(!terminals[curr_term_num].enter_pressed){     
                backspace_char();
            }
            break;
        case 0x1c: //enter pressed
            if(!terminals[curr_term_num].enter_pressed){     
                enter_char();
                terminals[curr_term_num].enter_pressed = 1;
            }
            break;
        case 0x3B:      // F1 pressed
            if(alt_pressed){
                switch_terminals(0);
                curr_process = terminal_pcb_top[0];
            }
            break;
        case 0x3C:      // F2 pressed
            if(alt_pressed){
                switch_terminals(1);
                // Allow execute to take PID value resevered for it
                if(!first_t1_switch){
                    first_t1_switch = 1;
                    unoccupy(1);
                }
                curr_process = terminal_pcb_top[1];
            }
            break;
        case 0x3D:      // F3 pressed
            if(alt_pressed){
                switch_terminals(2);
                // Allow execute to take PID value resevered for it
                if(!first_t2_switch){
                    first_t2_switch = 1;
                    unoccupy(2);
                }
                curr_process = terminal_pcb_top[2];
            }
            break;
        default:
            //get the value of the key pressed
            if(keycode > 57)    break;

            pressed_key = scancode_conversion[keycode][shift_pressed ^ caps_lock_pressed];

            //handle the special cases
            if (pressed_key == '\0')    //if the key pressed is null, do nothing
                break;
            if(ctrl_pressed == 1) {      //if ctrl is pressed, deal with ctrl + l/L
                if(pressed_key == 'l' || pressed_key == 'L') {
                    //clear the screen 
                    clear();
                    clear_char_buf();
                    clear_screen_buf();
                }
                else if (pressed_key == 'c' || pressed_key == 'C'){
                    //signal end of interrupt
                    send_eoi(KEYBOARD_IRQ);

                    //end of critical section
                    sti();
                    
                    //halt the process - do the same as exceptions when that gets fixed
                    sys_halt(255);
                    return;
                }
                break;
            }
            else if(alt_pressed){       //check if alt is pressed

            }
            else if (!terminals[curr_term_num].enter_pressed && keycode == 0x0F){   //add three spaces for tab
                vidmem_set(get_curr_term());
                char_buffer[char_buffer_idx] = ' ';
                char_buffer[char_buffer_idx + 1] = ' ';
                char_buffer[char_buffer_idx + 2] = ' ';
                char_buffer[char_buffer_idx + 3] = ' ';
                screen_buffer[screen_buffer_idx] = ' ';
                screen_buffer[screen_buffer_idx + 1] = ' ';
                screen_buffer[screen_buffer_idx + 2] = ' ';
                screen_buffer[screen_buffer_idx + 3] = ' ';
                printf(" ");
                printf(" ");
                printf(" ");
                printf(" ");
                char_buffer_idx += 4;
                screen_buffer_idx += 4;        
                vidmem_set(get_round_robin_term());
            }
            // If nothing special, add the character to buffer and print to screen
            else if (!terminals[curr_term_num].enter_pressed && char_buffer_idx < BUFFER_SIZE - 1){
                vidmem_set(get_curr_term());
                char_buffer[char_buffer_idx] = pressed_key;
                screen_buffer[screen_buffer_idx] = pressed_key;
                char_buffer_idx++;
                screen_buffer_idx++;
                putc(pressed_key);       
                vidmem_set(get_round_robin_term());
            }

    }

    // Update the cursor_pos
    update_cursor();

    // Signal end of interrupt
    send_eoi(KEYBOARD_IRQ);
    
    if(first_t1_switch && !t1_execute) {
        t1_execute = 1;
        execute((uint8_t*)"shell");
    }
    if(first_t2_switch && !t2_execute) {
        t2_execute = 1;
        execute((uint8_t*)"shell");
    }

    // End of critical section
    sti();

}

/* backspace_char
 *   DESCRIPTION: This function updates the buffer 
 *                and screen if backspace is pressed
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: deletes tha last character pressed
 */  
void backspace_char(){
    int i = 0;

    if(char_buffer_idx > 0){         //only delete a char if there is a value in the buffer
        backspace();                            
        screen_buffer_idx--;
        char_buffer_idx--;

        //Remove the character from the buffer
        screen_buffer[screen_buffer_idx] = '\0';    //Store Null character
        char_buffer[char_buffer_idx] = '\0';
    }
    else if(screen_buffer_idx > 0){
        backspace();                            
        screen_buffer_idx--;

        //Identify the correct x position of the cursor by finding previous '\n'
        for (i = 0; i < screen_buffer_idx; i++){      
            if(screen_buffer[screen_buffer_idx-i-1] == '\n')    
                break;
        }
        set_screen_x(i);
        char_buffer_idx = i;

        //Remove the character from the buffer
        screen_buffer[screen_buffer_idx] = '\0';    //Store Null character

        int j;
        for(j = 0; j < char_buffer_idx; j++){
            char_buffer[j] = screen_buffer[screen_buffer_idx-i+j];
        }
    }
}

/* enter_char
 *   DESCRIPTION: This function updates the buffer 
 *                and screen if enter is pressed
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: adds a newline and updates buffer
 */  
void enter_char(){
    //Update buffer
    char_buffer[char_buffer_idx] = '\n';
    char_buffer_idx++;

    screen_buffer[screen_buffer_idx] = '\n';
    screen_buffer_idx++;

    last_ent = 1;

    //Print a newline
    putc('\n');
}

/* clear_char_buf
 *   DESCRIPTION: Clears the character buffer and set the
 *                index to 0.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: updates char_buffer and char_buffer_idx
 */  
void clear_char_buf(){
    int i = 0;
    char_buffer_idx = 0;
    for(i = 0; i < BUFFER_SIZE; i++){
        char_buffer[i] = '\0';
    }
}

/* clear_screen_buf
 *   DESCRIPTION: Clears the screen buffer and set the
 *                index to 0.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: updates char_buffer and char_buffer_idx
 */  
void clear_screen_buf(){
    int i = 0;
    screen_buffer_idx = 0;
    for(i = 0; i < 1024; i++){
        screen_buffer[i] = '\0';
    }
}


                    
 
/* MP3.2!!! (nothing?)
*  terminal_open  
 *   DESCRIPTION: This function handles interrupts from 
 *                the keyboard
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: zeroes the char_buffer and char_buffer_index.
 */ 
int32_t terminal_open(const uint8_t* filename) {
    int i = 0;
    // Reset the buffer index
    char_buffer_idx = 0;

    // Clear the buffer using a for loop
    for (i = 0; i < BUFFER_SIZE; i++) {
        char_buffer[i] = NULL;
    }

    terminals[curr_term_num].enter_pressed = 1;

    return 0; 
}


/* MP3.2!!! 
*  terminal_read 
 *   DESCRIPTION: This function handles interrupts from 
 *                the keyboard
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: inputted key is echoed to monitor
 */ 
int32_t terminal_read(int32_t inode, int32_t offset, int32_t nbytes, void* buf) {
    char* curr_buffer = (char*) buf;
    int32_t curr_bytes = 0;
    int i = 0;

    // Wait for the enter input.
    sti();

    terminals[curr_term_num].enter_pressed = 0;

    while(terminals[curr_term_num].enter_pressed == 0);

    cli();
    // Copy characters from char_buffer to buf. 
    for (i = 0; i < char_buffer_idx - 1 && curr_bytes < nbytes; i++) {
        curr_buffer[i] = char_buffer[i];
        curr_bytes++;
    }
    // We shouldn't directly copy the buffer, in cases of overflow (?)
    if (curr_bytes < nbytes) {
        curr_buffer[curr_bytes] = '\n';
    }

    sti();

    clear_char_buf();
    clear_screen_buf();
    
    // Return the number of characters read.
    return curr_bytes;
}

/* MP3.2!!!
*  terminal_write 
 *   DESCRIPTION: 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */ 
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    char* curr_buffer = (char*) buf;
    int32_t curr_bytes = 0;
    int i = 0;

    if (curr_buffer == NULL || nbytes == 0) {
        return -1;
    }

    for (i = 0; i < nbytes /*&& i < BUFFER_SIZE*/; i++) {       
        vidmem_set(curr_term_num);
        putc(curr_buffer[i]);
        vidmem_set(get_round_robin_term());
        curr_bytes++;
    }
    return curr_bytes;
}

/* MP3.2!!! (nothing?)
*  terminal_close  
 *   DESCRIPTION: This function handles closing the terminal.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: zeroes the char_buffer and char_buffer_index.
 */ 
int32_t terminal_close(int32_t fd) {
    // Carbon copy of terminal_open.

    int i = 0;
    // Reset the buffer index
    char_buffer_idx = 0;

    // Clear the buffer using a for loop
    for (i = 0; i < BUFFER_SIZE; i++) {
        char_buffer[i] = NULL;
    }

    return 0; 
}

/* MP3.5!!!
*  switch_terminals  
 *   DESCRIPTION: This function handles switching between terminals.
 *   INPUTS: t_num - the terminal to switch into
 *   OUTPUTS: none
 *   RETURN VALUE: int32_t - -1 for failure, 0 for success
 *   SIDE EFFECTS: changes vidmem and the terminal.
 */ 
int32_t switch_terminals(int8_t t_num) {
    int i;
    
    // Check for garbage input
    if(t_num > 2)
        return -1;
    
    // To switch to current terminal, do nothing
    if(t_num == curr_term_num)
        return 0;

    // Enter critcal section
    cli();

    // Save the information of the old terminal
    terminals[curr_term_num].term_char_buffer_idx = char_buffer_idx;

    for(i = 0; i < BUFFER_SIZE; i++){
        terminals[curr_term_num].term_char_buffer[i] = char_buffer[i];
    }

    // terminals[curr_term_num].term_pcb = (struct pcb_t *) curr_process;
    terminals[curr_term_num].cursor_x_pos = get_screen_x();
    terminals[curr_term_num].cursor_y_pos = get_screen_y();

    // screen_x = &terminals[t_num].cursor_x_pos;
    // screen_y = &terminals[t_num].cursor_y_pos;

    // Deal with paging
    vidmem_set(curr_term_num);

    memcpy((void*) terminals[curr_term_num].term_vid_mem, (void*) VIDEO, ALIGNBYTES);

    // Get the information of the new terminal
    curr_term_num = t_num;
    
    char_buffer_idx = terminals[curr_term_num].term_char_buffer_idx;

    for(i = 0; i < BUFFER_SIZE; i++){
        char_buffer[i] = terminals[curr_term_num].term_char_buffer[i];
    }

    // curr_process = (pcb_t *) terminals[curr_term_num].term_pcb;
    set_screen_x(terminals[curr_term_num].cursor_x_pos);
    set_screen_y(terminals[curr_term_num].cursor_y_pos);

    memcpy((void*) VIDEO, (void*) terminals[curr_term_num].term_vid_mem, ALIGNBYTES);
    
    // Deal with paging
    vidmem_set(get_round_robin_term());

    sti();

    return 0;
}

/* MP3.5!!!
*  init_terminals  
 *   DESCRIPTION: This function intilaizes all three terminals.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: sets up paging for vidmem buffers.
 */ 
void init_terminals(){
    int i, j;
    
    for(i = 0; i < 3; i++){
        terminals[i].term_pcb = NULL;
        terminals[i].term_num = i;
        terminals[i].cursor_x_pos = 0;
        terminals[i].cursor_y_pos = 0;
        for(j = 0; j < BUFFER_SIZE; j++){
            terminals[i].term_char_buffer[j] = '\0';
        }
        terminals[i].term_char_buffer_idx = 0;
        terminals[i].running_pid = -1;
        terminals[i].enter_pressed = 1;
 
        // Set up vidmem buffers
        terminals[i].term_vid_mem = VIDEO + (i+1)*ALIGNBYTES;
        
        initialize_terminal_vidmem_paging(terminals[i].term_vid_mem >> 12);

    }
    
    flush_tlb();

    // Initalize terminal is 0
    curr_term_num = 0;
}

/* MP3.5!!!
*  vidmem_set  
 *   DESCRIPTION: This function deals with paging when switching terminals.
 *   INPUTS: t_num - terminal to switch into
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes paging for vidmem buffers.
 */ 
void vidmem_set(uint8_t t_num){
    
    // screen_x = &terminals[t_num].cursor_x_pos;
    // screen_y = &terminals[t_num].cursor_y_pos;
    
    // Setting up paging
    int32_t p_val = VIDEO >> 12;

    if(t_num == curr_term_num){
        pte[p_val].address = p_val;
        pte_vidmap[p_val].P = 1;
        pte_vidmap[p_val].address = p_val; 
    } else {
        pte[p_val].address = terminals[t_num].term_vid_mem >> 12;
        pte_vidmap[p_val].P = 1;
        pte_vidmap[p_val].address = terminals[t_num].term_vid_mem >> 12;
    }
    
    flush_tlb();
}

/* Getter for curr_term_num */
uint8_t get_curr_term() {
    return curr_term_num;
}

