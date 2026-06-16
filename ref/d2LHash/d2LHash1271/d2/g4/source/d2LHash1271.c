#include "d2LHash1271.h"
#include <string.h>

void d2LHash1271(uchar8 *h, uchar8 *m, uint64 *k, uint64 l) {
    uint64 i, n, r, s, *q;
    uchar8 *p;

    if (l == 0) {

        q = (uint64 *)h;
        *q = 0;
        *(q + 1) = 0;
        return;
    }

    n = l / 8;
    r = l % 8;

    if (r > 0)
        n = n + 1;

    s = n / BLOCK_SIZE;
    r = n % BLOCK_SIZE;

    if (r > 0) {

        p = (uchar8 *)((uint64 *)(m + s * BLOCK_SIZE)) + r;
        memset((void *)p, 0x0, (size_t)(BLOCK_SIZE - r));
        s = s + 1;
    }

    /* compute the hash of the message
     *  h: stores the hash
     *  m: input message
     *  k: base address of key powers array
     *  s: number of blocks
     *  l: bit-size of the message
     */
    d2LHash1271maax((uint64 *)h, (uint64 *)m, k, s, l);
}
