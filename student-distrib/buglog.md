# BUG LOG  

## Checkpoint 1
Bug: "Cannot find bounds of function" when causing a divide by zero error  
Fix: Set the present value of exceptions to 1 to mark as present

Bug: Not receiving interrupts from PIC devices  
Fix: Rewrote logic for enable and disable_irq  

Bug: Calling interrupt gates casued error: "Cannot find bounds of function"  
Fix: Set the reserved3 bit to 1 for interrupts

Bug: RTC Interrupt never called handler  - Segment not found exception  
Fix: Corrrected the vector location on IDT

Bug: Incorrect keys are printed (not what we pressed)  
Fix: Updated scantable (error in entries)  

Bug: For our 4MB entry struct, we had the address at [20:13] as an uint8_t, which made us boot loop.  
Fix: We changed it to a uint16_t, which fixed the boot loop.  

Bug : Intially set the kernel address to 1024  
Fix : We set it to 1. 

Bug: Tried non union implemention for Page Directory structs, but bootloaded.  
Fix : Used union implementation. 

Bug : Tried inline assembly implementation for setting up CR3, CR0 and CR4.  
Fix : Switched to seperate assembly file instead and called the function in the Initialize paging function.  

## Checkpoint 2
Bug: Tab did not print correctly.  
Fix: Change the scancode value to be correct.

Bug: Scrolling the page causes a boot loop.  
Fix: Forgot to call when adding new lines, and forgot to set the value of screen_y

Bug: Pressing enter does nothing.  
Fix: Forgot to call the enter_char function.  

Bug: Pressing enter prints a character instead of making a new line.  
Fix: Used quotation marks around the character instead of apostrophes. 

Bug: RTC test does not show functionality of RTC  
Fix: Test was not written correctly; rewrote tests to change frequency properly  

Bug: RTC set frequency always fails (returns -1)  
Fix: Missing curly braces (so it always returned -1)  

Bug: The read test shows one extra random characters from whatever we wrote.  
Fix: We didn't initialize our test buffers properly, plus we were placing the newline one extra character after the end of the string.  
  
Bug: Backspace doesn't delete the first character.  
Fix: Did the logic for going to the previous line in the wrong location, needed to decrement the buffer's idx first. 
  
Bug: Offset test wasnt working, it was printing junk variables.  
Fix: Setting the global offset variable to increment by nbytes instead of being equals to nbytes.  
  
Bug: Page faulting in general.  
Fix: We were dereferencing a non pointer in kernel.c when initializing the file system when it wasnt the pointer so it page faults.  
  
Bug: RTC did not output at desired frequency (incorrect virtualization of RTC)  
Fix: Fixed math to set frequency as desired  

Bug: Page faulting (due to read_data)  
Fix: We were trying to access memory that we could not; had to not account for chunk of memory vs memory address  
  

## Checkpoint 3  
Bug: Unable to read and write from terminal  
Fix: Corected implementation of stdin and stdout  

Bug: Pointers for jumptables for read, write, open, close were incorect - getting warnings  
Fix: Changed the arguements for the functions for the keyboard, rtc, terminal, and directory, and redid some logic for read_file  

Bug: Program Loader error  
Fix: Initially we were reading data once and using mem copy load the file contents into the location 0x08048000. Instead we used read_date twice, one to load file contents into 0x08048000 and the second one to load bytes 24 to 27 into EIP.   
  
Bug: Page Faut at Context Switch inline assembly  
Fix : Tried to access something that was out of range, changed the inline assembly and instead of using registers, we used 0,1,2,and 3 to push User_DS, ESP, USER_CS and EIP onto the stack.  
  
Bug: Restoring paging was page faulting.  
Fix: We were passing in curr_pcb->EIP instead of curr_pcb->PID  
  
Bug: Page faults for sys_halt.  
Fix: For the parent check, we were checking if the parent was NULL, changing that to checking if curr_pcb's PID is 0 fixed that. If it is 0, then it has no parent.  

Bug: ls was not outputting more than once
Fix: Our file index number was not being reset so we had reset to 0 when length of file name was 0 (i.e. no more files), so it would re-read all files when calling ls  

Bug: The txt file with a very long name was outputting a random character on the next line when calling ls    
Fix: The buffer was being overfilled but not cleared, so we had to clear it before filling it in read directory  


## Checkpoint 4  
Bug: Arguments from command line input could not be parsed  
Fix: The file argument buffer was being overwritten by itself because the buffer's length was not being incremented; fixed by incrementing buffer length when adding new character to buffer    
  
Bug: Arguments from command line input could not be parsed (part 2)  
Fix: Tried to use wrong buffer to copt to user space in strncpy (did not use cmd_args member of struct); fixed by changing input to correct one  
  
Bug: Whole image was not printed when trying cat on frame0/1.txt  
Fix: Had to change limit on buffer size in terminal write to allow all of frame0/1.txt to be printed  
  
Bug: If spacing was strange before command in command line (i.e. cat had spaces before it), the arguments could not be read  
Fix: Created variable to store how many spaces were before command in command line so argument buffer takes characters from command line starting at correct total offset  

Bug: After calling pingpong, no rtc interrupts were called  
Fix: Called sti() to set flags to allow interrupts before reading  

Bug: Vidmap causing a page fault  
Fix: Added vidmap to the system call jumptable

Bug: Vidmap doesn't output anything  
Fix: Fixed the garbage checks for vidmap  

## Checkpoint 5
Bug: When attempting to exit in terminal 0, it used to exit out of the last shell we executed in other terminals, either 1 or 2.  
Fix: We set up an array that kept the active pcb for each terminal and we switched between that when terminals were switched.  

Bug: We used to initialize shells by using the scheduler, but it wouldn't work properly.  
Fix: The fix for this was that we set up the shells whenever its corresponding hotkey was pressed for the first time, using a couple of flags and occupying necessary PIDs.  

Bug: When switching terminals, the video memory was being incorrectly copied.  
Fix: Changed how we saved the memory address to correctly represent the vidmem buffer location.  

Bug: Getting page fault when context switching for the PIT interrupt handler.  
Fix: N/A  
