#ifndef __d2LHash1271__
#define __d2LHash1271__

typedef unsigned char uchar8;
typedef unsigned long long uint64;

#define KEY_SIZE 16
#define BLOCK_SIZE 15
#define MSG_SIZE 8192 + 16

void d2LHash1271(uchar8 *, uchar8 *, uint64 *, uint64);

extern void d2LHash1271maax(uint64 *, uint64 *, uint64 *, uint64, uint64);
extern void d2LHash1271keypowers(uint64 *, uint64 *);

#endif
