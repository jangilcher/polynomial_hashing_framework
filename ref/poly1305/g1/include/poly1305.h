#ifndef __POLY1305__
#define __POLY1305__

typedef unsigned char uchar8;
typedef unsigned long long uint64;

#define KEY_SIZE 16
#define BLOCK_SIZE 16
#define MSG_SIZE 8192

void poly1305(uchar8 *, uchar8 *, const uchar8 *, uint64);

extern void poly1305maax(uint64 *, uint64 *, uint64 *, uint64, uint64, uint64);

#endif
