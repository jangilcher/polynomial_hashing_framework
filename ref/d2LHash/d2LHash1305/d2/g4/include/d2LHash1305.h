#ifndef __d2LHash1305__
#define __d2LHash1305__

typedef unsigned char uchar8;
typedef unsigned long long uint64;

#define KEY_SIZE 16
#define BLOCK_SIZE 16
#define MSG_SIZE 8192

void d2LHash1305(uchar8 *, uchar8 *, uint64 *, uint64);

extern void d2LHash1305maax(uint64 *, uint64 *, uint64 *, uint64, uint64);
extern void d2LHash1305keypowers(uint64 *, uint64 *);

#endif
