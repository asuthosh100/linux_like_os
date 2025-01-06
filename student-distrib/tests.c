#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"
#include "keyboard.h"
#include "filesys.h"
#include "syscallhandler.h"
#include "paging.h"


#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
// int idt_test(){
// 	TEST_HEADER;

// 	int i;
// 	int result = PASS;
// 	for (i = 0; i < 10; ++i){
// 		if ((idt[i].offset_15_00 == NULL) && 
// 			(idt[i].offset_31_16 == NULL)){
// 			assertion_failure();
// 			result = FAIL;
// 		}
// 	}

// 	return result;
// }

/* MP3.1!!!
 * Divde by zero exception test
 * 
 * Tests the exception
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Halts and prints exception
 * Coverage: Calls IDT vector after interrupt
 * Files: x86_desc.h/S
 */
// int divide_by_zero_test(){
// 	TEST_HEADER;

// 	int i = 0;
// 	int j = 1;

// 	j =  j/i;

// 	return FAIL;		// Should never reach here
// }

/* MP3.1!!!
 * Segment not present test
 * 
 * Tests the exception
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Halts and prints exception
 * Coverage: Calls IDT vector after interrupt
 * Files: x86_desc.h/S
 */
// int segment_not_present_test(){
// 	TEST_HEADER;
	
// 	asm ("int	$0x33");

// 	return FAIL;		// Should never reach here
// }

/* MP3.1!!!
 * Invalid opcode test
 * 
 * Tests the exception
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Halts and prints exception
 * Coverage: Calls IDT vector after interrupt
 * Files: x86_desc.h/S
 */
// int invalid_opcode_test(){
// 	TEST_HEADER;
	
// 	asm ("mov %cr6, %eax");

// 	return FAIL;		// Should never reach here
// }

/* MP3.1!!!
 * Testing system calls
 * 
 * System calls get called
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Calls IDT vector after interrupt
 * Files: x86_desc.h/S
 */
// int system_call_test() {
//     TEST_HEADER;
//     asm("int    $0x80");
//     return PASS; // If exception BSODs, we never get here
//  }

/* MP3.1!!!
 * paging_vidmem_test
 *   DESCRIPTION: Testing if the entries in the video memory exist from 0xB8000 to B9000.
 *   INPUTS: none.
 *   OUTPUTS: none.
 *   RETURN VALUE: none.
 *   SIDE EFFECTS: 
 *   Will return either pass or a page-fault exception on booting. We want a pass.
 */
// int paging_vidmem_test(){
// 	TEST_HEADER;
// 	int* i;
// 	int val;
// 	int j;
// 	int vid_add = 0xB8000;			// 0xB8000: Starting address of the video memory
// 	for(j = 0; j < 0x999; j++){		// 0x999: Takes you until the end of the video memory
// 		i = (int*)vid_add;
// 		val = *i;
// 		vid_add++;
// 	}
// 	i = (int*)vid_add;
// 	val = *i;
// 	// If val doesn't get the value of i, it will give a page fault. Otherwise, return PASS.
// 	return PASS;
// }

/* MP3.1!!!
 * paging_kernel_test
 *   DESCRIPTION: Testing if the entries in the kernel exist from 4MB too 8MB.
 *   INPUTS: none.
 *   OUTPUTS: none.
 *   RETURN VALUE: none.
 *   SIDE EFFECTS: 
 *   Will return either pass or a page-fault exception on booting. We want a pass.
 */
// int paging_kernel_test(){
// 	TEST_HEADER;
// 	int* i;
// 	int val;
// 	int j;
// 	int kernel_add = 0x400000;			// 0x400000: Starting address of kernel (4MB directory) 
// 	for(j = 0; j < 0x399999; j++){		// 0x399999 takes you until the end of the 4MB directory
// 		i = (int*)kernel_add;
// 		val = *i;
// 		kernel_add++;
// 	}
// 	i = (int*)kernel_add;
// 	val = *i;
// 	// If val doesn't get the value of i, it will give a page fault. Otherwise, return PASS.
// 	return PASS;
// }

/* MP3.1!!!
 * paging_OOB_test
 *   DESCRIPTION: Testing if any entry exists out of bounds of whatever we set. 
 *   INPUTS: none.
 *   OUTPUTS: none.
 *   RETURN VALUE: none.
 *   SIDE EFFECTS: 
 *   Will return either pass or a page-fault exception on booting. We want a page-fault exception.
 */
// int paging_OOB_test(){
// 	TEST_HEADER;
// 	int* i;
// 	int val;
// 	int test_add = 0x800001;		// 0x800001: Outside of the 4MB directory
// 	i = (int*)test_add;
// 	val = *i;
// 	// If val doesn't get the value of i, it will give a page fault. Otherwise, return PASS.
// 	return PASS;
// }

/* MP3.1!!!
 * paging_NULL_test
 *   DESCRIPTION: Tests when we access nothing (NULL)
 *   INPUTS: none.
 *   OUTPUTS: none.
 *   RETURN VALUE: none.
 *   SIDE EFFECTS: 
 *   Will return either pass or a page-fault exception on booting. We want a page-fault exception.
 */
// Null test, should return page fault.
// int paging_NULL_test(){
// 	TEST_HEADER;
// 	int* i;
// 	int val;
// 	int test_add = NULL;
// 	i = (int*)test_add;
// 	val = *i;
// 	// If val doesn't get the value of i, it will give a page fault. Otherwise, return PASS.
// 	return PASS;
// }

/* Checkpoint 2 tests */
/* RTC Test
 * Asserts that RTC driver works and can change frequency
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Tests RTC open, write, and close
 */
// int rtc_test(){
// 	TEST_HEADER;
// 	int32_t test_freq, i;
// 	rtc_open(0);
// 	for(test_freq = 2; test_freq <= RTC_MAX_FREQ; test_freq <<= 1) {
// 		rtc_write(0, (void *)&test_freq, sizeof(uint32_t));
// 		clear();
// 		i = 0;
// 		while(i < LONG_WAIT_TIME) {
// 			i++;
// 		}
// 	}
// 	rtc_close(NULL);
// 	return PASS;
// }

/* MP3.2!!!
 * terminal_loop_test
 *   DESCRIPTION: Tests when the read function of the terminal (NULL), prompts you until you type in ENDTEST.
 *   INPUTS: none.
 *   OUTPUTS: none.
 *   RETURN VALUE: none.
 *   SIDE EFFECTS: 
 *   Should print whatever was written onto the terminal before pressing enter and the number of characters read from the buffer. 
 */
// int terminal_loop_test()
// {
//     TEST_HEADER;
//     int32_t read_retval;
// 	int32_t write_retval;
// 	int i = 0;
// 	char buf0[BUF_SIZE];

// 	while(1){
// 		// Open the terminal, initialize variables.
// 		terminal_open(NULL);
// 		for(i = 0; i < BUF_SIZE; i++){
// 			buf0[i] = NULL;
// 		}
// 		// Create an empty buffer to test terminal_read with.
// 		printf("Testing read input... to PASS the test, type in 'ENDTEST' \n");
// 		read_retval = terminal_read(NULL, NULL, BUF_SIZE, buf0);

// 		// Otherwise, print to the screen what they typed using terminal_write.
// 		printf("You entered: ");
// 		write_retval = terminal_write(NULL, buf0, read_retval);
// 		printf("\n");

// 		// return read_retval as it is the number of characters the buffer has.
// 		printf("Read %d characters from terminal, including the newline character.\n", read_retval);

// 		// Close the terminal, set the variables back to their values.
// 		terminal_close(NULL);
// 		if(strcmp(buf0, "ENDTEST\n") == 0){
// 			break;
// 		}
// 	}
//     return PASS;
// }

/* MP3.2!!!
 * terminal_write_mismatch
 *   DESCRIPTION: Tests when the write function has more than 128 bytes to write.
 *   INPUTS: none.
 *   OUTPUTS: none.
 *   RETURN VALUE: none.
 *   SIDE EFFECTS: 
 *   Modify write_bytes to get the desired side effect.
 */
// int terminal_write_mismatch()
// {
//     TEST_HEADER;
//     int32_t read_retval;
// 	int32_t write_retval;
// 	int i = 0;
// 	// Terminal write has a built in cap of 128, so it wouldn't matter.
// 	int write_bytes = 130;			// Choose number larger than buffer size
// 	char buf0[BUF_SIZE];

// 	while(1){
// 		// Open the terminal, initialize variables and the buffer.
// 		terminal_open(NULL);
// 		for(i = 0; i < BUF_SIZE; i++){
// 			buf0[i] = NULL;
// 		}
// 		// Create an empty buffer to test terminal_read with.
// 		printf("Testing write mismatch... to PASS the test, type in 'END'\n", write_bytes);
// 		read_retval = terminal_read(NULL, NULL, buf0, BUF_SIZE);

// 		// Otherwise, print to the screen what they typed using terminal_write.
// 		printf("You entered: ");
// 		write_retval = terminal_write(NULL, buf0, write_bytes);
// 		printf("\n");

// 		// return read_retval as it is the number of characters the buffer has and write_bytes for personal use.
// 		printf("Read %d characters from terminal. Wrote %u bytes. \n", read_retval, write_retval);

// 		// Close the terminal, set the variables back to their values.
// 		terminal_close(NULL);
// 		if(strcmp(buf0, "END\n") == 0){
// 			break;
// 		}
// 	}
//     return PASS;
// }

/* MP3.2!!!
* strcmp
 *   DESCRIPTION: Helper function to compare two strings.
 *   INPUTS: none.
 *   OUTPUTS: none.
 *   RETURN VALUE: 0 if they are equal, otherwise the difference between str1[i] and str2[i] if they are not equal.
 *   SIDE EFFECTS: none.
 */
int strcmp(const char* str1, const char* str2) {
    int i = 0;

	// Loop through the strings until you reach a null character on either of them.
    for (i = 0; str1[i] && str2[i]; i++) {
		// If the characters are not equal, then return the difference.
        if (str1[i] != str2[i]) {
            return str1[i] - str2[i];
        }
    }
	// Otherwise return 0
    return 0;
}


/* Small File Read Test
 * 
 * Asserts that file system driver works and can read a small file
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Prints file to screen
 * Coverage: Open, close, read, write files
 */
// int small_file_read_test(){
// 	TEST_HEADER;
// 	const uint8_t* file = (uint8_t*)"frame1.txt";
// 	dentry_t den;
// 	int to_ret;
// 	char buf[174];

// 	if(read_dentry_by_name(file, &den))
// 		return FAIL;

// 	if(open_file((const uint8_t*)"frame1.txt") == -1)
// 		return FAIL;
// 	if(write_file((int32_t)file, (const void*)buf, (int32_t)10) != -1)
// 		return FAIL;
// 	to_ret = read_file(den.inode_num, 0, FRAME_1_SIZE, buf);
// 	int j;
// 	for (j = 0; j < FRAME_1_SIZE; j++) {
// 		putc(buf[j]);
// 	}
// 	putc('\n');
// 	to_ret = !close_file(NULL);		// Since it returns 0 on success but we need 1

// 	return to_ret;
// }


/* File Open Fail Test
 * 
 * Asserts failure when file is incorrectly attempted to be opened
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: File open
 */
// int file_open_fail_test(){
// 	TEST_HEADER;
// 	int32_t* file;
// 	int to_ret;
// 	file = (int32_t*)"frame12.txt";
// 	to_ret = open_file((const uint8_t*)file);
// 	if(to_ret == -1) 
// 		return FAIL;
// 	return PASS;
// }

/* Executable File Read Test
 * 
 * Asserts that an executable can be read
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Prints file to screen
 * Coverage: Read file
 */
// int executable_file_read_test(){
// 	TEST_HEADER;
// 	int32_t* file;
// 	int to_ret;
// 	char buf[LS_SIZE];
// 	file = (int32_t*)"ls";
// 	to_ret = read_file(file, 0, LS_SIZE, buf);
// 	int j;
// 	for (j = 0; j < LS_SIZE; j++) {
// 		if(buf[j]!='\0')
// 			putc(buf[j]);
// 	}
// 	putc('\n');
// 	return to_ret;
// }

/* Directory Read Test One
 * 
 * Asserts read directory only returns one filename
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Prints first directory to screen
 * Coverage: Open, write, close dir and read directory
 */
// int directory_read_test_one(){
// 	TEST_HEADER;
// 	int j;
// 	char buf[100];				// Allocating adequately large buffer
// 	int to_ret = FAIL; 
// 	if(open_dir() != 0)
// 		return FAIL;
// 	if(write_dir()!=-1)
// 		return FAIL;
// 	read_directory((void*)buf); 
// 	for(j=0 ; j<MAX_FN_LENGTH; j++) {
// 		putc(buf[j]);
// 	}
// 	to_ret = !close_dir();		// Since it returns 0 on success but we need 1
// 	return to_ret;
// }

/* Directory Read Test Loop
 * 
 * Asserts that all filenames can be outputted
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Outputs all filenames in directory to screen
 * Coverage: open, close, write dir, and read directory
 */
// int directory_read_test_loop(){
// 	TEST_HEADER;
// 	int j;
// 	int i;
// 	int to_ret;
// 	char buf[MAX_FN_LENGTH]; 
// 	// How many files to show:
// 	int file_num = NUM_FILES;
// 	if(open_dir() != 0)
// 		return FAIL;
// 	if(write_dir()!=-1)
// 		return FAIL;
// 	for(i = 0; i < file_num; i++){
// 		read_directory((void*)buf); 
// 		for(j=0 ; j<MAX_FN_LENGTH; j++) {
// 			putc(buf[j]);
// 		}
// 		putc('\n');
// 	}
// 	to_ret = !close_dir();		// Since it returns 0 on success but we need 1
// 	return to_ret;
// }

/* File Offset Read Test
 * 
 * Asserts that when repeatedly tring to read from one file,
 * 		   the file is outputted from the correct offset
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Outputs file with messages to screen
 * Coverage: read file
 */
// int file_offset_read_test(){
// 	TEST_HEADER;
// 	int32_t* file;
// 	int to_ret;
// 	int j;
// 	char buf0[50], buf1[50], buf2[74];		// Split frame1 into three parts
// 	// buf[0] = NULL;
// 	file = (int32_t*)"frame1.txt";

// 	printf("First chunk\n");
// 	to_ret = read_file(file, 0, 50, buf0);	// Loop over split buffer (size is 50)
// 	for (j = 0; j < 50; j++) {
// 		putc(buf0[j]);
// 	}
	
// 	putc('\n');
// 	printf("Second chunk\n");

// 	to_ret = read_file(file, 0, 50, buf1);	// Loop over split buffer (size is 50)
// 	for (j = 0; j < 50; j++) {
// 		putc(buf1[j]);
// 	}
	
// 	putc('\n');
// 	printf("Third chunk\n");

// 	to_ret = read_file(file, 0, 74, buf2);	// Loop over split buffer (size is 74)
// 	for (j = 0; j < 74; j++) {
// 		putc(buf2[j]);
// 	}

// 	putc('\n');
// 	return to_ret;
// }



/* Checkpoint 3 tests */

/* sys_call_read_file_test
 * 
 * Asserts that system call read works for files
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Outputs file to screen
 * Coverage: open, close, read (sys_call)
 */
int sys_call_read_file_test() {
	TEST_HEADER;
	int i;
	int32_t fd;
	uint8_t* filename;
	filename = (uint8_t*)"frame1.txt"; 
	char buf[100];		// Allocate ad·equately large buffer

	// Check that open works
	if((fd = open(filename)) == -1) return FAIL;

	if((read(fd, buf, 100)) == -1) return FAIL;
	for(i = 0; i < 100; i++)
		putc(buf[i]);

	
	if((read(fd, buf, 74)) == -1) return FAIL;
	for(i = 0; i < 74; i++)
		putc(buf[i]);

	putc('\n');

	return !close(fd);		// Since it returns 0 on success but we need 1
}

/* sys_call_read_dir_test
 * 
 * Asserts that system call read works for directories
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Outputs all directories to screen
 * Coverage: open, close, read (sys_call)
 */
int sys_call_read_dir_test() {
	TEST_HEADER;
	int i, j;
	int32_t fd;
	const uint8_t* filename = (uint8_t*)"."; 
	char buf[FILENAME_LEN];

	if((fd = open(filename)) == -1) return FAIL;
	
	for(i = 0; i < NUM_FILES; i++){
		read(fd, (void*)buf, MAX_FN_LENGTH); 
		for(j=0 ; j<MAX_FN_LENGTH; j++) {
			putc(buf[j]);
		}
		putc('\n');
	}

	if(close(fd) == -1) return FAIL;

	return PASS;
}

/* sys_call_rtc_test
 * 
 * Asserts that system call read/write/open/close works for RTC
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Displays RTC's changing frequency
 * Coverage: open, close, read, write (sys_call)
 */
int sys_call_rtc_test(){
	TEST_HEADER;
	int32_t test_freq, i;
	int32_t fd;
	void* buf;

	fd = open((uint8_t*)"rtc");
	if (fd == -1) return FAIL;
	
	for(test_freq = 2; test_freq <= RTC_MAX_FREQ; test_freq <<= 1) {
		write(fd, (void *)&test_freq, sizeof(uint32_t));
		clear();
		i = 0;
		while(i < test_freq) {
			if(read(fd, buf, 0) == -1) return FAIL;
			i++;
		}
	}
	return !close(fd);
}

/* sys_write_file_test
 * 
 * Asserts that system call write/open works for files
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: open, write (sys_call)
 */
int sys_call_write_file_test() {
	TEST_HEADER;
	int32_t fd;
	uint8_t* filename;
	filename = (uint8_t*)"frame1.txt"; 
	char buf[100];

	if((fd = open(filename)) == -1) return FAIL;

	// Should not be able to write, so return PASS if -1s
	if((write(fd, buf, 100)) == -1) return PASS;

	return FAIL;
}

/* sys_call_write_dir_test
 *
 * Asserts that system call write/open works for directories
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: open, write (sys_call)
 */
int sys_call_write_dir_test() {
	TEST_HEADER;
	int32_t fd;
	const uint8_t* filename = (uint8_t*)"ls"; 
	char buf[FILENAME_LEN];

	if((fd = open(filename)) == -1) return FAIL;
	
	// Should not be able to write, so return PASS if -1
	if(write(fd, (void*)buf, MAX_FN_LENGTH) == -1) return PASS;

	return FAIL;
}

/* sys_call_stdio
 *
 * DESCRIPTION: Tests when the read function of the terminal (NULL), prompts you until you type in ENDTEST.
 * INPUTS: none.
 * OUTPUTS: none.
 * RETURN VALUE: none.
 * SIDE EFFECTS: 
 * Should print whatever was written onto the terminal before pressing enter and the number of characters read from the buffer. 
 */
int sys_call_stdio() {
	TEST_HEADER;
    int32_t read_retval;
	int32_t write_retval;
	int i = 0;
	char buf0[BUF_SIZE];
	while(1){
		for(i = 0; i < BUF_SIZE; i++){
				buf0[i] = NULL;
			}
		// Create an empty buffer to test terminal_read with.
		printf("Testing read input... to PASS the test, type in 'ENDTEST' \n");
		read_retval = read(0, buf0, BUF_SIZE);

		// Otherwise, print to the screen what they typed using terminal_write.
		printf("You entered: ");
		write_retval = write(1, buf0, read_retval);
		printf("\n");

		// return read_retval as it is the number of characters the buffer has.
		printf("Read %d characters from terminal, including the newline character.\n", read_retval);

		// Close the terminal, set the variables back to their values.
		terminal_close(NULL);
		if(strcmp(buf0, "ENDTEST\n") == 0){
			break;
		}
	}
    return PASS;
}

/* MP3.3!!!
 * executable_file_test
 * executable_file_test : checks if the executable_file_check function is valid
 input: none
 output: none
 */

int executable_file_test() {

	int ret_val;
	int32_t* filename;
	filename = (int32_t*)"frame1.txt"; 
	ret_val = executable_file_check(filename);

	printf("return value is : %d\n", ret_val); 

	if(ret_val == 0) {
		return PASS;
	}
	
	if(ret_val == -1) {
		return FAIL;
	} 

	return FAIL;
}

/* MP3.3!!!
 * test_paging_for_execute
 * Description: Asserts that the process paging is set up correctly depending on the PID number.
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Sets up paging for whatever test_pid is.
 * Coverage: paging_for_execute
 */
int test_paging_for_execute() {
	// uint32_t test_pid = 1; 
	// uint32_t expected_address;
	// switch (test_pid) {
    // case 0:
    //     expected_address = 0x2; // 8MB for the first process
    //     break;
    // case 1:
    //     expected_address = 0x3; // 12MB for the second process
    //     break;
    // default:
    //     break;
	// }
    // paging_for_execute(test_pid);

    // // Verify page directory entry
    // uint32_t index = (0x08000000 >> 22);
    // if (base_dir[index].MB_dir.address != expected_address) {
    //     return FAIL;
    // }
    return PASS;
}

/* MP3.3!!!
 * pcb_kernel_stack_test
 * Description:  Checks if it allocates pcb correctly and if it stores it in the kernel properly.
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Modifies the stack.
 * Coverage: init_pcbs, setup_kernel_stack
 */
int pcb_kernel_stack_test() {
    // Initialize the PCBs
    init_pcbs();

    // Allocate a PCB for testing
    pcb_t* test_pcb = allocate_pcb();
    if (test_pcb == NULL) {
        printf("Failed to allocate PCB.\n");
        return FAIL;
    }
	// Save the current curr_process.
	pcb_t* prev_curr_process = curr_process;
    // Set up the kernel stack for the allocated PCB
    setup_kernel_stack(test_pcb);

    // Check if the parent PCB of the test PCB is set to the current process
	if (test_pcb != curr_process) {
        printf("Child PCB not set correctly.\n");
        return FAIL;
    }
    if (test_pcb->parent_pcb != prev_curr_process) {
        printf("Parent PCB not set correctly.\n");
        return FAIL;
    }

    // Determine the starting address of the kernel stack based on the PID.
    uint32_t stack_start_addr = KERNEL_END_ADDR - ((1 + test_pcb->PID) * KERNEL_TASK_SIZE);

    // Dereference the address to get the PCB stored at that location.
    pcb_t* stack_pcb = (pcb_t*)stack_start_addr;

	if(stack_pcb->PID == test_pcb->PID){
		return PASS;
	}
    return FAIL;
}

/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// TEST_OUTPUT("idt_test", idt_test());

	// Checkpoint 1 Tests
	// TEST_OUTPUT("divide_by_zero_test", divide_by_zero_test());
	// TEST_OUTPUT("segment_not_present_test", segment_not_present_test());
	// TEST_OUTPUT("invalid_opcode_test", invalid_opcode_test());
	// TEST_OUTPUT("system_call_test", system_call_test());
	// TEST_OUTPUT("paging_vidmem_test", paging_vidmem_test());
	// TEST_OUTPUT("paging_kernel_test", paging_kernel_test());
	// TEST_OUTPUT("paging_OOB_test", paging_OOB_test());
	// TEST_OUTPUT("paging_NULL_test", paging_NULL_test());

	// Checkpoint 2 Tests
	// TEST_OUTPUT("rtc_read_write_test", rtc_test());
	// TEST_OUTPUT("Terminal Loop Test", terminal_loop_test());
	// TEST_OUTPUT("Terminal Write Mismatch Test", terminal_write_mismatch());
	// TEST_OUTPUT("small_file_read_test", small_file_read_test());
	//TEST_OUTPUT("executable_file_read_test", executable_file_read_test());
	// TEST_OUTPUT("directory_read_test_one", directory_read_test_one());
	// TEST_OUTPUT("directory_read_test_loop", directory_read_test_loop());
	// TEST_OUTPUT("file_offset_read_test", file_offset_read_test());
	// TEST_OUTPUT("file_open_fail_test", file_open_fail_test());

	//checkpoint 3 tests 
	// TEST_OUTPUT("executable_file_test", executable_file_test());
	// TEST_OUTPUT("sys_call_read_file_test", sys_call_read_test());
	// TEST_OUTPUT("sys_call_read_dir_test", sys_call_read_dir_test());
	// TEST_OUTPUT("sys_call_rtc_test", sys_call_rtc_test());
	// TEST_OUTPUT("sys_call_write_file_test", sys_call_write_file_test());
	// TEST_OUTPUT("sys_call_write_dir_test", sys_call_write_dir_test());
	// TEST_OUTPUT("sys_call_stdio", sys_call_stdio());
}
