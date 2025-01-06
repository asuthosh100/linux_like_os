// -------------------------------------------------START OF UNION IMPLENTATION --------------------------------------
// #include "multiboot.h"
// #include "x86_desc.h"
#include "lib.h"
// #include "i8259.h"
// #include "debug.h"
// #include "tests.h"
// #ifndef _X86_DESC_H
// #define _X86_DESC_H

#define DIR_SIZE 1024
#define TABLE_SIZE 1024
#define ALIGNBYTES 4096
#define VIDEO 0xB8000
#define VIDEO_VIRTUAL 0x08800000
#define SIZE_4MB 0x400000 
#define FOUR_MB 0x400000
#define SHELL_ADDR 0x00800000  

typedef union page_dir_entry_4KB {
    uint32_t val;
    struct {
        uint8_t P: 1;       // Present
        uint8_t R_W : 1;    // Read/Write
        uint8_t U_S : 1;    // User/Supervisor
        uint8_t PWT : 1;    // Write-through
        uint8_t PCD : 1;    // Cache Disable
        uint8_t A : 1;      // Accessed
        uint8_t AVL_1 : 1;  // Available
        uint8_t PS : 1;     // Page Size
        uint8_t AVL_4 : 4;  // Available
        uint32_t address : 20;
    } __attribute__((packed));
} page_dir_entry_4KB_t;

typedef union page_dir_entry_4MB {
    uint32_t val;
    struct {
        uint8_t P: 1;       // Present
        uint8_t R_W : 1;    // Read/Write
        uint8_t U_S : 1;    // User/Supervisor
        uint8_t PWT : 1;    // Write-through
        uint8_t PCD : 1;    // Cache Disable
        uint8_t A : 1;      // Accessed
        uint8_t D : 1;      // Dirty
        uint8_t PS : 1;     // Page Size
        uint8_t G : 1;      // Global
        uint8_t AVL_11_9 : 3; // Available
        uint8_t PAT : 1;    // Page Attribute Table
        uint16_t add_20_13 : 8; // address from 20:13
        uint8_t RSVD : 1;   // RSVD
        uint16_t address : 10;
    } __attribute__((packed));
} page_dir_entry_4MB_t;

typedef union page_table_entry {
    uint32_t val;
    struct {
        uint8_t P: 1;       // Present
        uint8_t R_W : 1;    // Read/Write
        uint8_t U_S : 1;    // User/Supervisor
        uint8_t PWT : 1;    // Write-through
        uint8_t PCD : 1;    // Cache Disable
        uint8_t A : 1;      // Accessed
        uint8_t D : 1;      // Dirty
        uint8_t PAT : 1;    // Page Attribute Table
        uint8_t G : 1;      // Global
        uint8_t AVL_3 : 3;  // Available
        uint32_t address : 20;
    } __attribute__((packed));
} page_table_entry_t;

typedef union page_directories {
        page_dir_entry_4MB_t MB_dir;
        page_dir_entry_4KB_t KB_dir;
} page_directories_t;

// Array of structs that will have the 4KB and 4MB directory structs.
page_directories_t base_dir[DIR_SIZE] __attribute__((aligned (ALIGNBYTES)));
// Then, for the table, we will need 2^10 1024 tables with 1024 entries, pointing to pages.
page_table_entry_t pte[TABLE_SIZE] __attribute__((aligned (ALIGNBYTES)));
// Page table for vidmem.
page_table_entry_t pte_vidmap[TABLE_SIZE] __attribute__((aligned(ALIGNBYTES)));

extern void initialize_paging();

void load_directory(unsigned int* page_directory);

////////////////////////////Checkpoint 3/////////////////////////////////////////////////////////////////////////
void paging_for_execute(uint32_t pid); 
void flush_tlb(); 
////////////////////////////Checkpoint 4/////////////////////////////////////////////////////////////////////////
void initialize_paging_vidmem();
////////////////////////////Checkpoint 5/////////////////////////////////////////////////////////////////////////
void initialize_terminal_vidmem_paging(uint8_t j);
void map_to_vidmem_page(uint8_t physical_address);

