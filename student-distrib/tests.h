#ifndef TESTS_H
#define TESTS_H

#define LONG_WAIT_TIME  1000000000
#define BUF_SIZE        128
#define FRAME_1_SIZE    174
#define LS_SIZE         5349
#define MAX_FN_LENGTH   32
#define NUM_FILES       17

// test launcher
void launch_tests();

// strcmp helper
int strcmp(const char* str1, const char* str2);
#endif /* TESTS_H */
