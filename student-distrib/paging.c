#include "lib.h"
#include "paging.h"

/* MP3.1!!!
 * initialize_paging
 *   DESCRIPTION: Sets up the paging system by initializing the directory, tables, and populating the first two entries.
 *   INPUTS: none.
 *   OUTPUTS: none.
 *   RETURN VALUE: none.
 *   SIDE EFFECTS: 
 *   Modifies global base_dir and pte arrays, and modifies the page directory register via the load_directory function.
 */
void initialize_paging() {
    int i;
    //Initializes the base directory entries to default values. 
    for(i = 0; i < DIR_SIZE; i++){
        base_dir[i].KB_dir.P = 0;  
        base_dir[i].KB_dir.R_W = 1;      // Read/Write
        base_dir[i].KB_dir.U_S = 0;      // User/Supervisor
        base_dir[i].KB_dir.PWT = 0;      // Write-through
        base_dir[i].KB_dir.PCD = 0;      // Cache Disable
        base_dir[i].KB_dir.A = 0;        // Accessed
        base_dir[i].KB_dir.PS = 0;       // Page Size
        base_dir[i].KB_dir.AVL_1 = 0;    // Available
        base_dir[i].KB_dir.AVL_4 = 0;    // Available
        base_dir[i].KB_dir.address = i; 
    }
    int j;
    unsigned int vid_mem = VIDEO>>12; 
    // Initializes the Page Table Entries (PTE) with default values and sets the video memory entry as present.
    for(j = 0; j < TABLE_SIZE; j++){
        pte[j].PWT = 0;      // Write-through
        pte[j].PCD = 0;      // Cache Disable
        pte[j].A = 0;        // Accessed
        pte[j].D = 0;        // Dirty
        pte[j].PAT = 0;      // Page Attribute Table
        pte[j].G = 0;        // Global
        pte[j].AVL_3 = 0;    // Available
        pte[j].address = j;  // Set the adress to the index.
        pte[j].P = 0;        // Present
        pte[j].R_W = 1;      // Read/Write
        pte[j].U_S = 0;      // User/Supervisor
        if(j == vid_mem) {                   
            pte[j].P = 1;    // Present
        } 
    }   

    // Populates the first entry of the base directory for a 4KB page.
    base_dir[0].KB_dir.P = 1;        // Present
    base_dir[0].KB_dir.address = (uint32_t)pte >> 12; // Shifted 12 to get the 10 MSB bits.

    // Populates the second entry of the base directory for a 4MB page.
    base_dir[1].MB_dir.R_W = 1;      // Read/Write
    base_dir[1].MB_dir.P = 1;        // Present
    base_dir[1].MB_dir.PS = 1;       // Page Size
    base_dir[1].MB_dir.address = 1;  // Address should point to the physical memory of the kernel page (doubt)
 
    load_directory((uint32_t*) base_dir);
    }

////////////////////////////////////// Checkpoint 3 ////////////////////////////////////////////////////

/* MP3.3!!!
 * paging_for_execute
 *   DESCRIPTION: Sets up the paging for execute system call and will also be called in halt system call to restore parent paging. The PID input plays a significant role in accessing the physical memeory of each program. 
 *   INPUTS: uint32_t pid.
 *   OUTPUTS: none.
 *   RETURN VALUE: none.
 */

void paging_for_execute(uint32_t pid) {
   uint32_t index; // to find

   index = (0x08000000>>22); //0x08000000 is the index where the program info will be stored (virtual mem)
    base_dir[index].MB_dir.P = 1; // Present is 1
    base_dir[index].MB_dir.R_W = 1; // Read-Write is 1
    base_dir[index].MB_dir.U_S = 1; // Supervisor mode
    base_dir[index].MB_dir.PWT = 0;
    base_dir[index].MB_dir.PCD = 0;
    base_dir[index].MB_dir.A= 0;
    base_dir[index].MB_dir.D = 0;
    base_dir[index].MB_dir.PS = 1; // Page Size is 1
    base_dir[index].MB_dir.G = 0;
    base_dir[index].MB_dir.AVL_11_9 = 0;
    base_dir[index].MB_dir.PAT = 0;
    base_dir[index].MB_dir.add_20_13= 0;
    base_dir[index].MB_dir.RSVD = 0;
    base_dir[index].MB_dir.address = 2+pid; // find using PID; 

    flush_tlb(); // flush tlb

} 

/* MP3.3!!!
 * flush_tlb
 *   DESCRIPTION: Flushes TLB
 *   OUTPUTS: none.
 *   RETURN VALUE: none.
 */
void flush_tlb() {

// This is how we flush TLB, source: OS DEV. 
    // movl	%cr3,%eax
	//movl	%eax,%cr3

    asm volatile (
        "movl %%cr3, %%eax                              ;"
        "movl %%eax, %%cr3                              ;"
        :
        :
        :"%eax","cc" // specifying eax as clobbered register, and cc for condition code registers (refer inline assembly documentation [Section 5.3] on course page.)

    );

}

/* MP3.4!!!
 * initialize_paging_vidmem
 *   DESCRIPTION: We use to set the virtual and physicals addresses for vidmem.
 *   INPUTS: none.
 *   OUTPUTS: none.
 *   RETURN VALUE: none.
 *   SIDE EFFECTS: Flushes tlb, sets the paging for vidmem.
 */
void initialize_paging_vidmem() {
    int j = 0;
    // Initialize page table for vidmem.
    for(j = 0; j < TABLE_SIZE; j++){
        pte_vidmap[j].PWT = 0;      // Write-through
        pte_vidmap[j].PCD = 0;      // Cache Disable
        pte_vidmap[j].A = 0;        // Accessed
        pte_vidmap[j].D = 0;        // Dirty
        pte_vidmap[j].PAT = 0;      // Page Attribute Table
        pte_vidmap[j].G = 0;        // Global
        pte_vidmap[j].AVL_3 = 0;    // Available
        pte_vidmap[j].address = (uint32_t)VIDEO >> 12;  
        pte_vidmap[j].P = 1;        // Present
        pte_vidmap[j].R_W = 1;      // Read/Write
        pte_vidmap[j].U_S = 1;      // User/Supervisor
    }
    // Align virtual address before using to map videomem to.
    uint32_t i = (uint32_t)VIDEO_VIRTUAL >> 22;
    base_dir[i].KB_dir.P = 1;  
    base_dir[i].KB_dir.R_W = 1;      
    base_dir[i].KB_dir.U_S = 1;      
    base_dir[i].KB_dir.PWT = 0;      
    base_dir[i].KB_dir.PCD = 0;      
    base_dir[i].KB_dir.A = 0;        
    base_dir[i].KB_dir.PS = 0;       
    base_dir[i].KB_dir.AVL_1 = 0;    
    base_dir[i].KB_dir.AVL_4 = 0;    
    base_dir[i].KB_dir.address = (uint32_t)pte_vidmap >> 12;  

    flush_tlb();
}

////////////////////////////////////// Checkpoint 5 ////////////////////////////////////////////////////

/* MP3.5!!!
 * initialize_terminal_vidmem_paging
 *   DESCRIPTION: Sets a page, j into vidmem.
 *   INPUTS: j.
 *   OUTPUTS: none.
 *   RETURN VALUE: none.
 *   SIDE EFFECTS: Flushes tlb, changes a page table entry.
 */
void initialize_terminal_vidmem_paging(uint8_t j){
    pte[j].PWT = 0;      // Write-through
    pte[j].PCD = 0;      // Cache Disable
    pte[j].A = 0;        // Accessed
    pte[j].D = 0;        // Dirty
    pte[j].PAT = 0;      // Page Attribute Table
    pte[j].G = 0;        // Global
    pte[j].AVL_3 = 0;    // Available
    pte[j].address = j;  // Set the address to the index.
    pte[j].P = 0;        // Present
    pte[j].R_W = 1;      // Read/Write
    pte[j].U_S = 0;      // User/Supervisor                 
    pte[j].P = 1;        // Present
}

/* MP3.5!!!
 * map_to_vidmem_page
 *   DESCRIPTION: we set a virtual address to a point to a specific vidmem page.
 *   INPUTS: physical_address
 *   OUTPUTS: none.
 *   RETURN VALUE: none.
 *   SIDE EFFECTS: Flushes tlb, sets the paging for vidmem.
 */
void map_to_vidmem_page(uint8_t physical_address) {
    int j = 0;
    // Initialize page table for vidmem.
    for(j = 0; j < TABLE_SIZE; j++){
        pte_vidmap[j].PWT = 0;      // Write-through
        pte_vidmap[j].PCD = 0;      // Cache Disable
        pte_vidmap[j].A = 0;        // Accessed
        pte_vidmap[j].D = 0;        // Dirty
        pte_vidmap[j].PAT = 0;      // Page Attribute Table
        pte_vidmap[j].G = 0;        // Global
        pte_vidmap[j].AVL_3 = 0;    // Available
        pte_vidmap[j].address = physical_address >> 12;  
        pte_vidmap[j].P = 1;        // Present
        pte_vidmap[j].R_W = 1;      // Read/Write
        pte_vidmap[j].U_S = 1;      // User/Supervisor
    }
    // Align virtual address before using to map videomem to.
    uint32_t i = VIDEO_VIRTUAL >> 22;
    base_dir[i].KB_dir.P = 1;  
    base_dir[i].KB_dir.R_W = 1;      
    base_dir[i].KB_dir.U_S = 1;      
    base_dir[i].KB_dir.PWT = 0;      
    base_dir[i].KB_dir.PCD = 0;      
    base_dir[i].KB_dir.A = 0;        
    base_dir[i].KB_dir.PS = 0;       
    base_dir[i].KB_dir.AVL_1 = 0;    
    base_dir[i].KB_dir.AVL_4 = 0;    
    base_dir[i].KB_dir.address = (uint32_t)pte_vidmap >> 12;  

    flush_tlb();
}
