
#define ASM     1

.text
.globl load_directory

/* MP3.1!!!
 * load_directory
 *   DESCRIPTION: Loads the base directory address into CR3, sets the page size extension bit in CR4 to high, 
 *                and sets the paging enable (PGE) along with protection bit (PE) to high in CR0.
 *   INPUTS: none.
 *   OUTPUTS: none.
 *   RETURN VALUE: none.
 *   SIDE EFFECTS: 
 *   Modifies registers EAX, CR3, CR4 and CR0.
 */
load_directory:
pushl %ebp
movl %esp, %ebp

# Store the pushed arg (base_dir) in cr3.
movl 8(%ebp), %eax
movl %eax, %cr3

# Set the 4th bit (Page Size Extension) to 1 in CR4.
movl %cr4, %eax 
orl 0x00000010, %eax # 0x00000010 will set the 4th bit to 1.
movl %eax, %cr4

# Set the 31st bit (PGE) and and 0th (PE) bit to 1 in CR0
movl %cr0, %eax
orl $0x80000001, %eax # 0x80000001 will set the 31st bit and 0th bit to 1.
movl %eax, %cr0

leave
ret



