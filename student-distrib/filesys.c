/* filesys.c - Functions to interact with the file system */

#include "filesys.h"
#include "lib.h"

#define SUCCESS 0
#define FAILURE -1

boot_block_t * bb; 
inode_t * inode_init; 
unsigned int * db;
unsigned int file_index = 0;
// uint32_t read_offset = 0;

dentry_t dentry_1;
dentry_t * dentry_1_ptr = &dentry_1;

/* MP3.2!!! 
*  filesys_init  
 *   DESCRIPTION: creates the datastructure for our file system
 *   INPUTS: uint32_t boot_block_address
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *  
 */ 
void filesys_init(uint32_t boot_block_address) {
    bb = (boot_block_t *)boot_block_address; // boot block 
    inode_init = (inode_t *)(bb+1); // inode block
    db = (unsigned int *) (inode_init + (bb->inode_count)); // data block 
}

/* MP3.2!!! 
*  read_dentry_by_name  
 *   DESCRIPTION: This function reads the directory entry by name and checks if the file that we are trying to access exists in the directory
 *   INPUTS: const uint8_t* fname, dentry_t* dentry
 *   OUTPUTS: none
 *   RETURN VALUE: returns success or failure
 *  
 */ 
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry) { 
    int i; 
     int string_comparison;
    

    for(i=0; i<63; i++){  // here 63 is the number of directory entries
          // compares the filename that inputs with the file name in the directory
        string_comparison = strncmp((int8_t*)fname,(int8_t*)bb->dentries[i].filename,32); // here 32 is the number of bytes to copy

        if(string_comparison == 0) {
            int source; 
            int destination; 

            source = (int)bb->dentries[i].filename;
            destination = (int)dentry->filename; 
            // copies the source into the destination 
            strncpy((int8_t*)destination,(int8_t*)source, 32); // here 32 is the number of bytes to copy 
            dentry->filetype = bb->dentries[i].filetype; // match the respective file type; 
            dentry->inode_num = bb->dentries[i].inode_num; // match the respective file type;

            return SUCCESS; 

        }
    }

     return FAILURE; 

}

/* MP3.2!!! 
*  read_dentry_by_index  
 *   DESCRIPTION: This function reads the directory entry by index and checks if the file that we are trying to access exists in the directory
 *   INPUTS: const uint8_t* index, dentry_t* dentry
 *   OUTPUTS: none
 *   RETURN VALUE: returns success or failure
 *  
 */ 
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){ 
      
       int i; 
       int source; 
       int destination; 

      if(index >= bb->inode_count && index <= 0)  {
        return FAILURE;
      }

      else {
        for(i = 0; i<63; i++) { // here 63 is the number of directory entries
         // checks if the index passed by the user is equivalent to an entry in the directory
            if(index == bb->dentries[i].inode_num) {
                source = (int) bb->dentries[i].filename;
                destination = (int)dentry->filename; 
            // copies source into the destination 
            strncpy((int8_t*)destination,(int8_t*)source, 32); // here 32 is the number of bytes to copy
            dentry->filetype = bb->dentries[i].filetype; // match the respective file type; 
            dentry->inode_num = bb->dentries[i].inode_num; // match the respective file type;


            return SUCCESS; 
            }
        }
      }

    return FAILURE; 
}

/* MP3.2!!! 
*  read_data  
 *   DESCRIPTION: The function reads data associated with files from the data block.
 *   INPUTS: uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length
 *   OUTPUTS: none
 *   RETURN VALUE: returns bytes read
 *  
 */ 
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) { 

    int first_block; 
    int starting_byte; 
  
    uint32_t block_num;
    int i;
    inode_t* cur_inode;


    // sets up current node
    cur_inode = (inode_t*)(inode_init + inode);

    // math done to figure out the starting block and starting byte to read from
    first_block = offset/4096; // 4096 is the blocksize
    starting_byte = offset%4096; // 4096 is the blocksize

    for(i=0 ; i<length; i++){

    // overflow check 
    if(starting_byte >= 4096){ // 4096 is the blocksize
      starting_byte = 0;
      first_block++;
    }

    // figures out the current block number
      block_num = cur_inode->data_block_num[first_block];
      if(block_num >= bb->data_count){
        return FAILURE;
      }

      // sanity check
       if((i+offset) >= cur_inode->length ){
        return i;
      }

      // populates the buffer with the data
      buf[i] =   ((uint8_t*)(inode_init+bb->inode_count+block_num))[starting_byte];
    
      // read for next byte
      starting_byte++; 
      
    }

    return i;

}

/* MP3.2!!! 
*  read_file  
 *   DESCRIPTION: The function reads data associated with files from the data block.
 *   INPUTS: int32_t* filename, int32_t off, int32_t nbytes, void* buf
 *   OUTPUTS: none
 *   RETURN VALUE: returns bytes read
 *  
 */ 
int32_t read_file(int32_t inode_num, int32_t off, int32_t nbytes, void* buf) {

        // uint32_t inode_num; 
        uint32_t i; 
        int32_t bytes_read = -1;
        // read_offset += off;
        // if(read_offset > nbytes){
        //   read_offset = 0;
        // }
        for(i = 0; i <63; i++ ) {// 63 is the number of directory entries

        // string comparison to figure out the filename input gives exists in the directory entry 

        // strncmp((int8_t*)filename,(int8_t*)bb->dentries[i].filename,32)==0 // Previous check
       if (inode_num == bb->dentries[i].inode_num) {

      // if exists then update the following

        // inode_num = bb->dentries[i].inode_num; 

        // call read data to read from the file

        bytes_read = read_data (inode_num, ( uint32_t)off,  (uint8_t*)buf, nbytes);

        // read offset used to accomodate for the offset read file test. 
        
        // read_offset += nbytes;
        
        return bytes_read;

       }
    }

  return bytes_read;

}

/* MP3.2!!! 
*  read_directory  
 *   DESCRIPTION: The function reads files present in the directory.
 *   INPUTS: int32_t inode_num, int32_t off, int32_t nbytes, void *buf
 *   OUTPUTS: none
 *   RETURN VALUE: returns success or fail
 *  
 */ 
int32_t read_directory(int32_t inode_num, int32_t off, int32_t nbytes, void* buf){
  int8_t*  source;
  int8_t* destination;
  if(file_index > 63){ // 63 the number of directory entries
    file_index = 0;
  }
  if(bb->dentries[file_index].filename == NULL){
    file_index = 0;
    return -1; // returns failure
  }
  
  // int i = 0;
  // while(((char*)buf)[i] != '\0'){
  //   ((char*)buf)[i] = '\0';
  //   i++;
  // }
  memset(buf, (int32_t)'\0', 33);

  // populates the directory names into the buffer
  source = (int8_t*) bb->dentries[file_index].filename;
  destination = (int8_t*) buf;
  strncpy(destination, source, 32);
  file_index++;

  if(strlen(destination) == 0)
    file_index = 0;

  return strlen(destination); // returns success
}

/* MP3.2!!! 
*  open_file  
 *   DESCRIPTION: The function opens file.
 *   INPUTS: const uint8_t* filename
 *   OUTPUTS: none
 *   RETURN VALUE: returns success or fail
 *  
 */ 
 int32_t open_file (const uint8_t* filename) {
    
    
    if(read_dentry_by_name(filename, dentry_1_ptr)!= 0) {
      return -1; // -1 is fail
    }
    
    return 0; // 0 is success
 }

 /* MP3.2!!! 
*  close_file  
 *   DESCRIPTION: The function closes file.
 *   INPUTS: int32_t fd
 *   OUTPUTS: none
 *   RETURN VALUE: returns success or fail
 *  
 */ 
 int32_t close_file(int32_t fd) {
    return 0; // 0 is success
 }

/* MP3.2!!! 
*  write_file  
 *   DESCRIPTION: The function writes file.
 *   INPUTS: int32_t fd, const void* buf, int32_t nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: returns success or fail
 *  
 */ 
 int32_t write_file (int32_t fd, const void* buf, int32_t nbytes) {
    return -1; // -1 is fail
 }

/* MP3.2!!! 
*  open_dir  
 *   DESCRIPTION: The function opens dir.
 *   INPUTS:
 *   OUTPUTS: none
 *   RETURN VALUE: returns success 
 *  
 */ 
 int32_t open_dir (const uint8_t* filename) {
  
      return 0; // success
  
 }

 /* MP3.2!!! 
*  close_dir  
 *   DESCRIPTION: The function closes dir.
 *   INPUTS:
 *   OUTPUTS: none
 *   RETURN VALUE: returns success 
 *  
 */
 int32_t close_dir (int32_t fd) {
      file_index = 0;
      return 0; // success
  
 }

 /* MP3.2!!! 
*  write_dir  
 *   DESCRIPTION: The function writes dir.
 *   INPUTS:
 *   OUTPUTS: none
 *   RETURN VALUE: returns success 
 *  
 */
  int32_t write_dir (int32_t fd, const void* buf, int32_t nbytes) {
    return -1;  // failure
 }

////////////////////////////////////////////////// Checkpoint 3 /////////////////////////////////////// 



// Strategy : Write as much as helpers possible and unit test each block. 

// first helper is going to check if the file is an exectuable or not 



dentry_t dentry_2;

/* MP3.3!!! 
*  executable_file_check 
 *   DESCRIPTION: This function checks if the file is executable or not.
 *   INPUTS: int32_t* filename
 *   OUTPUTS: none
 *   RETURN VALUE: returns success (0) or fail (1)
 *  
 */ 


int32_t executable_file_check(int32_t* filename) {
    char buf[40]; 
    // int to_ret; 
    int32_t inode_num; 

    // checking if the file exists 

    if(read_dentry_by_name((const uint8_t*)filename, &dentry_2)!=0){
        return -1;
    } 

    inode_num = dentry_2.inode_num;

    // read first 40 bytes from the file

    read_data((uint32_t) inode_num, (uint32_t) 0, (uint8_t*) buf, (uint32_t) 40);  // 40 here is number of bytes to be read. 

    // check for magic numbers, if the fist four magic numbers match as shown in the following code, then it is an executable file. 

    if (buf[0] != 0x7f){ // 0x7f is the first magic number
        return -1; // -1 indicates failure 
    }

    if(buf[1] != 0x45) { // 0x45 is the second magic number
        return -1; // -1 indicates failure 
    }

     if(buf[2] != 0x4c) { // 0x4c is the third magic number
        return -1; // -1 indicates failure 
    }

     if(buf[3] != 0x46) { // 0x46 is the fourth magic number
        return -1; // -1 indicates failure 
    }


    return 0; // 0 indicates Success 

} 







