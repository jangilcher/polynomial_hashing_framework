#include "poly1271.h"
#include <string.h>

void poly1271(uchar8 *h, uchar8 *m, const uchar8 *k, uint64 l) {

    uint64 i, n, r, s, f = 1, *q;
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

    switch (r) {

    case 1:
        m[n - 1] = m[n - 1] & 0x01;
        m[n - 1] = m[n - 1] | 0x02;
        f = 0;
        break;
    case 2:
        m[n - 1] = m[n - 1] & 0x03;
        m[n - 1] = m[n - 1] | 0x04;
        f = 0;
        break;
    case 3:
        m[n - 1] = m[n - 1] & 0x07;
        m[n - 1] = m[n - 1] | 0x08;
        f = 0;
        break;
    case 4:
        m[n - 1] = m[n - 1] & 0x0f;
        m[n - 1] = m[n - 1] | 0x10;
        f = 0;
        break;
    case 5:
        m[n - 1] = m[n - 1] & 0x1f;
        m[n - 1] = m[n - 1] | 0x20;
        f = 0;
        break;
    case 6:
        m[n - 1] = m[n - 1] & 0x3f;
        m[n - 1] = m[n - 1] | 0x40;
        f = 0;
        break;
    case 7:
        m[n - 1] = m[n - 1] & 0x7f;
        m[n - 1] = m[n - 1] | 0x80;
        f = 0;
        break;

    case 0:
        if (n % BLOCK_SIZE > 0) {

            n = n + 1;
            m[n - 1] = m[n - 1] & 0x00;
            m[n - 1] = m[n - 1] | 0x01;
            f = 0;
        };
    }
    m[n] = 0x00;
    s = n / BLOCK_SIZE;
    r = n % BLOCK_SIZE;

    if (r > 0) {

        p = (uchar8 *)((uint64 *)(m + s * BLOCK_SIZE)) + r;
        memset((void *)p, 0x0, (size_t)(BLOCK_SIZE + 1 - r));
        s = s + 1;
    }

    /* compute the hash of the message
     *  h: stores the hash
     *  m: input message
     *  k: key
     *  s: number of block
     *  f: f=1 indicates the last block is full
     *     f=0 indicates the last block is partial
     *  n: number of bytes in the message;
     *     passing this to optimize the computation
     *     for formatted messages upto 129 bits
     */
    poly1271maax((uint64 *)h, (uint64 *)m, (uint64 *)k, s, f, n);
}
