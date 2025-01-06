
#ifndef _FILESYS_H
#define _FILESYS_H

#include "lib.h"
#include "types.h"

#define FILENAME_LEN 32

typedef struct dentry {
  int8_t filename[FILENAME_LEN];
  int16_t filetype; 
  int32_t inode_num; 
  int8_t rsvd[24]; // 24 reserved 
} dentry_t; 

typedef struct boot_block {
  int32_t dir_count; 
  int32_t inode_count;
  int32_t data_count; 
  int8_t rsvd[52]; // 52 reserved 
  dentry_t dentries[63]; // 63 no of entries
} boot_block_t; 

typedef struct inode {
    int32_t length; 
    int32_t data_block_num[1023];// 1023 number of datablocks
} inode_t;

int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);

int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);

int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t read_file(int32_t inode_num, int32_t off, int32_t nbytes, void* buf);

int32_t open_file (const uint8_t* filename);

int32_t close_file(int32_t fd);

int32_t write_file (int32_t fd, const void* buf, int32_t nbytes);

int32_t read_directory(int32_t inode_num, int32_t off, int32_t nbytes, void* buf); 

void filesys_init(uint32_t boot_block_address); 

int32_t open_dir(const uint8_t* filename);

int32_t close_dir(int32_t fd);

int32_t write_dir(int32_t fd, const void* buf, int32_t nbytes);

///////////////////////Checkpoint 3///////////////////////////////////////////////// 

int32_t executable_file_check(int32_t* filename);


#endif /* _FILESYS  _H */


