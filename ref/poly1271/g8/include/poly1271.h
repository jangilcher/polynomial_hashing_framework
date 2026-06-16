#ifndef __POLY1271__
#define __POLY1271__

typedef unsigned char uchar8;
typedef unsigned long long uint64;

#define KEY_SIZE 16
#define BLOCK_SIZE 15
#define MSG_SIZE 8192 + 16

void poly1271(uchar8 *, uchar8 *, uint64 *, uint64);

extern void poly1271maax_precomputed_keypowers(uint64 *, uint64 *, uint64 *,
                                               uint64, uint64, uint64);
extern void poly1271keypowers(uint64 *, uint64 *);

#endif
