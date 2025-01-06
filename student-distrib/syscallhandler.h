#ifndef _SYSCALLHANDLER_H
#define _SYSCALLHANDLER_H

#include "lib.h"
#include "rtc.h"
#include "filesys.h"
#include "keyboard.h"

#define MAX_TASKS 8     // The number of tasks we need for 3.3 is 2.
#define MAX_FILES 8     // The number of files tasks can open at the same time is 8 for 3.3.
#define MAX_FN_LENGTH   32      // Max possible length of file name
#define KERNEL_START_ADDR 0x400000  // 4MB
#define KERNEL_END_ADDR 0x800000    // 8MB
#define KERNEL_TASK_SIZE 0x2000    // 8kB
#define INSTR_START 24
#define INSTR_LENGTH 4
#define PROG_INFO_ADDR 0x08048000
#define PROG_IMG_ADDR 0x83FFFFC

#define USER_MEM_START  0x08000000
#define USER_MEM_END    0x08400000

// Jump table for file operations
typedef struct file_op_jmp_tbl {
    int32_t (*read) (int32_t inode_num, int32_t off, int32_t nbytes, void* buf);
    int32_t (*write) (int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*open) (const uint8_t* filename);
    int32_t (*close) (int32_t fd);
    // Add more as needed
} file_op_jmp_tbl_t;

typedef struct file_descriptor {
    // ... fields for the file descriptor ...
    file_op_jmp_tbl_t* file_op_jmp_tbl_ptr;
    uint32_t file_pos;
    int32_t inode;
    uint32_t flags;
} file_descriptor_t;

typedef struct pcb {
    uint32_t EIP;                               // Entry point of the program for this process
    uint32_t EBP; 
    uint32_t ESP;    
    uint32_t EIP_context;                               // Entry point of the program for this process
    uint32_t EBP_context; 
    uint32_t ESP_context;                                 
    uint32_t* page_directory;                   // Pointer to the process' page directory
    int terminal_number;
    file_descriptor_t file_array[MAX_FILES];    // Array of file descriptors.
    uint32_t PID;                               // Process ID
    struct pcb* parent_pcb;                     // Pointer to parent task's PCB, will use for clean up.
    uint8_t cmd_args[MAX_FN_LENGTH];            // Array of program's command line arguments
} pcb_t;

// Execute Variables:
pcb_t pcbs[MAX_TASKS];
pcb_t* curr_process;
pcb_t* terminal_pcb_top[3];


// Execute Functions:
pcb_t* allocate_pcb(); 
pcb_t* get_cur_pcb();
void setup_kernel_stack(pcb_t* pcb);
void init_pcbs();
void deallocate_pcb(pcb_t* pcb);

/* Execute system call */
int execute(const uint8_t* command);

/* Halt system call */
int sys_halt(uint8_t status);

/* Read system call */
int32_t read (int32_t fd, void* buf, int32_t nbytes);

/* Write system call */
int32_t write(int32_t fd, void* buf, int32_t nbytes);

/* Open system call */
int32_t open (const uint8_t* filename);

/* Close system call */
int32_t close(int32_t fd);

/* Get args system call */
int32_t getargs (uint8_t* buf, int32_t nbytes);

/* Vid mem system call */
int32_t vidmap (uint8_t** screen_start);

/* Set ESP, EBP, and return */
extern void halt_return(uint32_t ebp, uint32_t esp, uint8_t status);

/* Holds PID values for shells */
void occupy(int pid_to_occupy);

/* Releases PID values for PCBs */
void unoccupy(int pid_to_occupy);

#endif  /* _SYSCALLHANDLER_H */
