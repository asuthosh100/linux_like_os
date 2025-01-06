#include "filesys.h"
#include "syscallhandler.h"
#include "x86_desc.h"
#include "paging.h"
#include "pit.h"


file_op_jmp_tbl_t file_jmp_tbl = {&read_file, &write_file, &open_file, &close_file};

file_op_jmp_tbl_t dir_jmp_tbl = {&read_directory, &write_dir, &open_dir, &close_dir};

file_op_jmp_tbl_t rtc_jmp_tbl = {&rtc_read, &rtc_write, &rtc_open, &rtc_close};

file_op_jmp_tbl_t term_jmp_tbl = {&terminal_read, &terminal_write, &terminal_open, &terminal_close};

uint32_t curr_pid;
pcb_t* par_pcb;

/* MP3.3!!! 
 * execute
 *   DESCRIPTION: Executes a command by setting up paging and the pcbs, loading the program into memory, and switching to user mode.
 *   INPUTS: command - a pointer to the command string to be executed
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: Modifies paging structures and kernel stack, changes process context
 */
int execute(const uint8_t* command) {
    // 1. Parse the command to get the filename of the program to be executed. (Not need for CP3)
        // Declare local variables
        cli();
        int i, j;
        uint8_t file_cmd[MAX_FN_LENGTH];
        uint8_t file_args[MAX_FN_LENGTH];
        uint8_t length = strlen((const int8_t*)command);
        uint8_t cmd_len = 0;
        uint8_t arg_len = 0;
        uint8_t cmd_offset = 0;

        // Initialize file_cmd and file_arg arrays
        for(i = 0; i < MAX_FN_LENGTH; i++) {
            file_cmd[i] = '\0';
            file_args[i] = '\0';
        }

        // Parse commands
        for(i = 0; i < length; i++) {
            // If input is not space then add to file_cmd buffer and increment length
            if(command[i] != ' ') {
                file_cmd[cmd_len] = command[i];
                cmd_len++;
            // Otherwise, increment offset for extra spacing before command and break if there is a command
            } else {
                cmd_offset++;
                if(cmd_len > 0) {
                    break;
                }
            }
        }

        // Parse arguments
        for(i = cmd_len + cmd_offset; i < length; i++) {
            // If input is space, check whether there is an argument; if there is then break, else continue
            if(command[i] == ' ') {
                if(arg_len > 0)
                    break;
                else
                    continue;
            // Otherwise loop through rest of command buffer and add to file_args buffer
            } else {
                j = i;
                while (j < length) {
                    file_args[arg_len] = command[j];
                    arg_len++;
                    j++;
                }
                file_args[arg_len] = '\0';
                break;
            }
        }

    // 2. Check if the file exists in the filesystem and is executable.
        if(executable_file_check((int32_t*) file_cmd) == -1){
            return -1;
        }

    // 3. Allocate a PCB for the new task.
        // init_pcbs();
        pcb_t* curr_pcb = allocate_pcb(); 

        if (curr_pcb == NULL) {
            // Handle error: No available PCBs.
            return -1;
        }

        // Set parent's and temrinals numbers

        curr_pid = curr_pcb->PID;
        if(curr_pid != 0){
            curr_pcb->parent_pcb = curr_process;
            if(curr_pcb->PID > 2) { 
                curr_pcb->terminal_number = get_curr_term();
            }
        } else {
            par_pcb = curr_pcb;
        }

        // Fill out the array for scheduling
        switch (curr_pid){
            case 0:
                terminal_pcb_top[0] = curr_pcb;
                break;
            case 1:
                terminal_pcb_top[1] = curr_pcb;
                break;
            case 2:
                terminal_pcb_top[2] = curr_pcb;
                break;
            default:
                terminal_pcb_top[get_curr_term()] = curr_pcb;
        }


        // Set the commad arguements for the PCB
        for(i = 0; i < FILENAME_LEN; i++){
            curr_pcb->cmd_args[i] = '\0';
        }

        strncpy((int8_t*)curr_pcb->cmd_args, (int8_t*)file_args, arg_len);

    // 4. Set up the kernel stack for the new task.
        setup_kernel_stack(curr_pcb);

    // 5. Set up paging for the new task.
        paging_for_execute(curr_pcb->PID);


    // 6. Load the program image from the filesystem into memory.
        dentry_t dentry_3;
        uint32_t inode_num; 
        int read1,read2;
        read1 = 0;
        read2 = 0;

        if(read_dentry_by_name((const uint8_t*)file_cmd, &dentry_3)!=0){
                return -1;
            } 
            
        inode_num = dentry_3.inode_num;
        read1 = read_data(inode_num,INSTR_START,(uint8_t*)&curr_pcb->EIP,INSTR_LENGTH);
        read2 = read_data(inode_num, 0, (uint8_t*)PROG_INFO_ADDR,100000); // 0 and 100000 because we want to read the whole thing.

        if(read1 == - 1 || read2 == -1) return -1;


    // 7. Prepare for context switch to user mode.
        // SS0 (Stack Segment at Privilege Level 0): Specifies the kernel's stack segment, which will be used when the processor switches to kernel mode.
        // ESP0 (Stack Pointer at Privilege Level 0): Specifies the stack pointer that points to the process's kernel-mode stack. This stack will be used when transitioning 
        // from user mode to kernel mode.

        tss.ss0 = KERNEL_DS; 
        tss.esp0 = KERNEL_END_ADDR - (curr_process->PID) * KERNEL_TASK_SIZE - sizeof(curr_process);
        uint32_t uds; 
        uint32_t esp; 
        uint32_t ucs; 
        uint32_t eiip; 
        uds = USER_DS;
        esp = (uint32_t)PROG_IMG_ADDR; 
        ucs = USER_CS; 
        eiip = curr_process->EIP;

    // 8. Switch to user mode and start executing the program.

        // IRET Context : 
        // Things required on the stack 
        // USER_DS, ESP(132MB) , EFLAG, CS, EIP (!!!!!Be Careful with the order!!!!!!!!)

        sti(); 

        asm volatile(
            "movl %%esp, %0;"
            : "=r"(curr_pcb->ESP)
            : 
        );

        asm volatile(
            "movl %%ebp, %0;"
            : "=r"(curr_pcb->EBP)
            : 
        );
        asm volatile(
            "pushl %0;"
            "pushl %1;"
            "pushfl  ;"
            "pushl %2;"
            "pushl %3;"
            :
            : "r"(uds), "r"(esp), "r"(ucs), "r"(eiip)
            : "cc", "memory"
        );
        
        asm volatile(
            "IRET"
        );

    return 0;    
   
}


/* MP3.3!!! 
 * sys_halt
 *   DESCRIPTION: Terminates a process and returns control to the parent process or starts a new shell if it's the base shell.
 *   INPUTS: status - the return status of the process
 *   OUTPUTS: none
 *   RETURN VALUE: The status of the halt operation; typically the status argument if successful
 *   SIDE EFFECTS: Modifies current process, updates PCB and paging
 */
int sys_halt(uint8_t status) {
    // Get the pcb that we are currently executing.
    cli();
    
    if(curr_process == NULL) return -1;
    uint32_t curr_ebp, curr_esp;
    int i;
    if(curr_process->PID > 2){
        for(i = 0; i < MAX_FILES; i++) {
            close(i);
        }
        curr_process->PID = -1;

        tss.ss0 = KERNEL_DS;
        tss.esp0 = KERNEL_END_ADDR - ((curr_process->parent_pcb->PID) * KERNEL_TASK_SIZE) - sizeof(curr_process);

        //if the current process has a parent process, then we will return control to it an deallocated the curr_pcb.
        curr_ebp = curr_process->EBP;
        curr_esp = curr_process->ESP;

        curr_process = curr_process->parent_pcb;
        terminal_pcb_top[get_curr_term()] = curr_process;

        paging_for_execute(curr_process->PID);

        // Then, return the stack pointer to the parent stack.

        sti();

        halt_return(curr_ebp, curr_esp, (int)status);

    return (int)status;

    } else {
        // If there's no parent, create a new shell process.
        for(i = 0; i < MAX_FILES; i++) {
            curr_process->file_array[i].flags = 0;
            curr_process->file_array[i].file_pos = 0;
        }
        curr_process->PID = -1;
        terminals[get_round_robin_term()].running_pid = -1;
        execute((uint8_t*)"shell");
        return 0;
    }

    return 0;  
}

/* MP3.3!!! 
 * allocate_pcb
 *   DESCRIPTION: Allocates a PCB for a new process from the array of PCBs.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: Pointer to the allocated PCB, or NULL if allocation fails
 *   SIDE EFFECTS: Updates the PCB array, assigns a PID
 */ 
pcb_t* allocate_pcb() {
    int i;
    for (i = 0; i < MAX_TASKS; i++) {
        if (pcbs[i].PID == -1) {  // Check if the PCB is unused
            pcbs[i].PID = i;      // Assign a new PID 
            curr_pid = i;
            terminals[get_round_robin_term()].running_pid = i;
            return &pcbs[i];      // Return a pointer to the allocated PCB
        }
    }
    return NULL;  
}

/* MP3.3!!! 
 * setup_kernel_stack
 *   DESCRIPTION: Sets up the kernel stack for a process in its PCB.
 *   INPUTS: pcb - pointer to the PCB of the process
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Modifies the kernel stack.
 */
void setup_kernel_stack(pcb_t* pcb) {
    // Determine the starting address of the kernel stack based on the PID. 
    uint32_t stack_start_addr = KERNEL_END_ADDR - ( (1 + pcb->PID) * KERNEL_TASK_SIZE);

    // Copy the PCB data to this location
    memcpy((void*)stack_start_addr, pcb, sizeof(pcb_t));

    curr_process = pcb;
}

/* MP3.3!!! 
 * get_cur_pcb
 *   DESCRIPTION: Retrieves the current process's PCB based on the stack pointer.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: Pointer to the current process's PCB
 *   SIDE EFFECTS: none
 */
pcb_t* get_cur_pcb(){
    pcb_t* cur_pcb;
    asm volatile(                 
        "movl %%esp, %%eax              ;"
        "andl %1, %%eax                 ;"
        "movl %%eax, %0                 ;"
        :"=r"(cur_pcb)          // Output to store cur_pcb val
        :"g"(0xFFFFE000)        // Bit mask for 8KB
        :"%eax"                 // Clobbers EAX
    );
    return cur_pcb;
}

/* MP3.3!!! 
 * init_pcbs
 *   DESCRIPTION: Initializes the array of PCBs, marking them as unused.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Set the PCB array to an unused state
 */
void init_pcbs() {
    int i,j;
    for (i = 0; i < MAX_TASKS; i++) {
        pcbs[i].EBP = 0;
        pcbs[i].PID = -1;  // -1 indicates that the PCB is not in use
        pcbs[i].page_directory = NULL;
        pcbs[i].parent_pcb = NULL;
        pcbs[i].EIP = 0;
        for (j = 0; j < MAX_FILES; j++){
            pcbs[i].file_array[j] = (file_descriptor_t){0};
        }
        for (j = 0; j < MAX_FN_LENGTH; j++){
            pcbs[i].cmd_args[j] = '\0';
        }
    }

    // Set up the terminal numbers for the inital shells
    pcbs[0].terminal_number = 0;
    pcbs[1].terminal_number = 1;
    pcbs[2].terminal_number = 2;
}

/* MP3.3!!! 
 * deallocate_pcb
 *   DESCRIPTION: Marks a PCB as unused and resets its contents.
 *   INPUTS: pcb - pointer to the PCB to deallocate
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Resets the specified PCB to an unused state
 */
void deallocate_pcb(pcb_t* pcb) {
    int i;
    pcb->EBP = 0;
    pcb->PID = -1;  // -1 indicates that the PCB is not in use
    pcb->page_directory = NULL;
    pcb->parent_pcb = NULL;
    pcb->EIP = 0;
    for (i = 0; i < MAX_FILES; i++){
        pcb->file_array[i] = (file_descriptor_t){0};
    }
    for (i = 0; i < MAX_FN_LENGTH; i++){
        pcb->cmd_args[i] = '\0';
    }
}



/* MP3.3!!! 
 * read
 *   DESCRIPTION: Reads data from a file descriptor into a buffer.
 *   INPUTS: fd - file descriptor from which to read
 *           buf - buffer to read data into
 *           nbytes - number of bytes to read
 *   OUTPUTS: buf - contains the data read from the file
 *   RETURN VALUE: number of bytes read on success, -1 on failure
 *   SIDE EFFECTS: Modifies the buffer and changes the filel position in the file descriptor
 */
int32_t read (int32_t fd, void* buf, int32_t nbytes) {
    // Declare local variables
    pcb_t* cur_pb_ptr;
    int32_t bytes_read;

    // Check for valid inputs
    if(buf == NULL) {
        return -1;
    }

    if(fd < 0 || fd > MAX_FILES) {
        return -1;
    }

    if(nbytes < 0) {
        return -1;
    }

    // Inputs are valid, so read data
    cur_pb_ptr = get_cur_pcb();      // Get pointer to current PCB

    // If file is closed, then return fail
    if(fd > 1 && cur_pb_ptr->file_array[fd].flags == 0) {
        return -1;
    }

    sti();

    // Since file is used, call read using jump table and return number of bytes read
    switch (fd){
        case 0:
            return terminal_read(NULL, NULL, nbytes, buf);
        case 1:
            return -1;
        default:
            bytes_read = cur_pb_ptr->file_array[fd].file_op_jmp_tbl_ptr->read(cur_pb_ptr->file_array[fd].inode, cur_pb_ptr->file_array[fd].file_pos, nbytes, buf);
            cur_pb_ptr->file_array[fd].file_pos += bytes_read;
    }

    return bytes_read;
}

/* MP3.3!!! 
 * write
 *   DESCRIPTION: Writes data to a file descriptor from a buffer.
 *   INPUTS: fd - file descriptor to which to write
 *           buf - buffer containing data to write
 *           nbytes - number of bytes to write
 *   OUTPUTS: none
 *   RETURN VALUE: number of bytes written on success, -1 on failure
 *   SIDE EFFECTS: None.
 */
int32_t write (int32_t fd, void* buf, int32_t nbytes) {
    // Declare local variables
    pcb_t* cur_pb_ptr;

    // Check for valid inputs
    if(buf == NULL) {
        return -1;
    }

    if(fd < 0 || fd > MAX_FILES) {
        return -1;
    }

    if(nbytes < 0) {
        return -1;
    }

    // Inputs are valid, so read data
    cur_pb_ptr = get_cur_pcb();      // Get pointer to current PCB

    // If file is closed, then return fail
    if(fd > 1  && cur_pb_ptr->file_array[fd].flags == 0) {
        return -1;
    }

    // Since file is used, call read using jump table and return number of bytes read
    switch (fd){
        case 0:
            return -1;
        case 1:
            return terminal_write(fd, buf, nbytes);
        default:
            return cur_pb_ptr->file_array[fd].file_op_jmp_tbl_ptr->write(fd, buf, nbytes);
    }
}

/* MP3.3!!! 
 * close
 *   DESCRIPTION: Close system call, closes the task by reseting file descriptor values and allowing it to be in use
 *   INPUTS: fd - file descriptor used to closed
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: Allows the file desccriptor to be used in new task.
 */
int32_t close (int32_t fd) {
    // Declare local variables
    pcb_t* cur_pb_ptr;
    int32_t is_closed;

    // Check for valid input
    if(fd < 0 || fd > MAX_FILES) {
        return -1;
    }

    // Get the PCB
    cur_pb_ptr = get_cur_pcb();

    // If file is already closed, then return fail (-1)
    if(cur_pb_ptr->file_array[fd].flags == 0) {
        return -1;
    }

    // Othewise, reset file's members and return result of close (0 if pass, -1 if fail)
    cur_pb_ptr->file_array[fd].flags = 0;
    is_closed = cur_pb_ptr->file_array[fd].file_op_jmp_tbl_ptr->close(fd);
    return is_closed;
}

/* MP3.3!!! 
 * open
 *   DESCRIPTION: Open system call, opens file by assign a filel descriptor from current pcb, setting file descriptor values, and assign jumptaable based on filetype
 *   INPUTS: filename - file being opened
 *   OUTPUTS: none
 *   RETURN VALUE: file descriptor index on success, -1 on failure
 *   SIDE EFFECTS: Flags a file desscriptor in use and sets its values.
 */
int32_t open (const uint8_t* filename){
    // Declare local variables
    pcb_t* cur_pb_ptr;
    int i;
    int32_t fd = -1;

    // Check inputs
    if(filename == NULL || *filename == '\0')
        return -1;

    dentry_t check_pos_dentry;
    if(read_dentry_by_name(filename, &check_pos_dentry))
        return -1;
    
    // Get the PCB
    cur_pb_ptr = get_cur_pcb();

    // Get the first avaialble file desc
    for(i = 2; i < MAX_FILES; i++){
        if(cur_pb_ptr->file_array[i].flags == 0){
            fd = i;
            break;
        }
    }

    // If no empty file desc
    if(fd == -1)
        return -1;

    // Initalize PCB vals
    cur_pb_ptr->file_array[fd].flags = 1;
    cur_pb_ptr->file_array[fd].file_pos = 0;
    cur_pb_ptr->file_array[fd].inode = check_pos_dentry.inode_num;

    // Set the correct jump table depending on the filetype
    switch(check_pos_dentry.filetype){
        case 0:
            cur_pb_ptr->file_array[fd].file_op_jmp_tbl_ptr = &rtc_jmp_tbl;
            break;
        case 1: 
            cur_pb_ptr->file_array[fd].file_op_jmp_tbl_ptr = &dir_jmp_tbl;
            break;
        case 2:
            cur_pb_ptr->file_array[fd].file_op_jmp_tbl_ptr = &file_jmp_tbl;
            break;
        default:
            return -1;
    }

    // Call open
    if(cur_pb_ptr->file_array[fd].file_op_jmp_tbl_ptr->open(filename))
        return -1;

    return fd;
}

/* MP3.4!!! 
 * getargs
 *   DESCRIPTION: 
 *   INPUTS: buf - buffer to write args into
 *           nbytes - number of bytes to be written
 *   OUTPUTS: none
 *   RETURN VALUE: returns 0 on success, -1 on failure
 *   SIDE EFFECTS: Reads program's command line arguments to user-level buffer
 */
int32_t getargs (uint8_t* buf, int32_t nbytes) {
    // Declare local variables
    pcb_t* cur_pb_ptr;

    // Check for valid inputs
    if(buf == NULL || nbytes < 0) {
        return -1;
    }

    // Get current PCB
    cur_pb_ptr = get_cur_pcb();

    // Check whether there are any command lines arguments
    if(cur_pb_ptr->cmd_args[0] == NULL) {
        return -1;
    }    

    // If there are, then copy them to user space
    strncpy((int8_t*)buf, (int8_t*)(cur_pb_ptr->cmd_args), (uint32_t)nbytes);
    return 0;
}

/* MP3.4!!! 
 * vidmap
 *   DESCRIPTION: passes the virtual address for vid mem
 *   INPUTS: screen_start
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success and -1 on failure
 *   SIDE EFFECTS: Rewrites the video memory
 */
int32_t vidmap (uint8_t** screen_start) {
    // Validate the pointer from the user space
    if (screen_start == NULL) {
        return -1;
    }

    int address = (uint32_t) screen_start;
    if (address < USER_MEM_START || address > USER_MEM_END){
        return -1;
    }

    // Set the correct page values
    initialize_paging_vidmem();

    // Provide the virtual address of the video memory
    *screen_start = (uint8_t*)(VIDEO_VIRTUAL);

    return 0;
}

/* MP3.5!!! 
 * occupy
 *   DESCRIPTION: Set pid_to_occupy to a non-negative number, so it new pcbs wont be set to it.
 *   INPUTS: 
 *   OUTPUTS: none
 *   RETURN VALUE: 
 *   SIDE EFFECTS: 
 */
void occupy(int pid_to_occupy) {
    pcbs[pid_to_occupy].PID = pid_to_occupy;      // Set PID to used.
}

/* MP3.5!!! 
 * unoccupy
 *   DESCRIPTION: Set pid_to_occupy to -1, so new pcbs will be set to it.
 *   INPUTS: 
 *   OUTPUTS: none
 *   RETURN VALUE: 
 *   SIDE EFFECTS: 
 */
void unoccupy(int pid_to_occupy){
    pcbs[pid_to_occupy].PID = -1;      // Set PID to unused.
}
