#include "chacha.h"
#include "cyclecount.h"
#include <immintrin.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
// #include <sys/random.h>
#include "hash.h"
#include <stdlib.h>

#ifndef BLCKSIZE
#define BLCKSIZE 14
#endif
#ifndef HASHSIZE
#define HASHSIZE 16
#endif
#ifndef ITERATIONS 
#define ITERATIONS 10000
#endif
#define MSGSIZE ((672 << 0))
#define TOT_KEYSIZE (3 * BLCKSIZE)

uint64_t ccycles = 0, hcycles = 0, totcycles = 0;
size_t getrandom(void *__buffer, size_t __length, unsigned int __flags) {
    uint32_t r = 0;
    size_t l = 0;
    for (size_t i = 0; i < __length; i += sizeof(uint32_t)) {
        r = random();
        l = __length - i > sizeof(uint32_t) ? sizeof(uint32_t) : __length - i;
        memcpy(((uint8_t *)__buffer) + i, &r, l);
    }
    return 0;
}
void hash(unsigned char *out, const unsigned char *in, unsigned long long inlen,
          unsigned char *key, unsigned long long keylen);
int main() {
    srandom(0);
    // uint8_t m[MSGSIZE] = {0};
    uint8_t *m = calloc(1, MSGSIZE);
    if (!m)
        exit(-1);
    // for (int i = 0; i < MSGSIZE; i += BLCKSIZE) {
    //     // m[i+ BLCKSIZE -15] = 0xff;
    //     m[i+ BLCKSIZE -14] = 0xff;
    //     // m[i+ BLCKSIZE -13] = 0xff;
    //     // m[i+ BLCKSIZE -12] = 0xff;
    //     // m[i+ BLCKSIZE -11] = 0xff;
    //     // m[i+ BLCKSIZE -10] = 0xff;
    //     // m[i+ BLCKSIZE -9] = 0xff;
    //     // m[i+ BLCKSIZE -8] = 0xff;
    //     // m[i+ BLCKSIZE -7] = 0xff;
    //     // m[i+ BLCKSIZE -6] = 0xff;
    //     // m[i+ BLCKSIZE -5] = 0xff;
    //     // m[i+ BLCKSIZE -4] = 0xff;
    //     // m[i+ BLCKSIZE -3] = 0xff;
    //     // m[i+ BLCKSIZE -2] = 0xff;
    //     // m[i+ BLCKSIZE -1] = 0xff;//((i / BLCKSIZE) + 1) << 3;
    // }

    uint8_t ct[HASHSIZE] = {0};
    unsigned char key[TOT_KEYSIZE] = {0};
    uint64_t start, stop;
    key[0] = 0x3<<6;
    key[BLCKSIZE] = 0;
    key[2*BLCKSIZE] = 0;
    getrandom(key, BLCKSIZE, 0);
    getrandom(key + BLCKSIZE, BLCKSIZE, 0);
    getrandom(key + 2*BLCKSIZE, BLCKSIZE, 0);
    getrandom(m, MSGSIZE, 0);
    for (size_t i = 0; i < MSGSIZE; i++ ){
        printf("%02"PRIx8"", m[i]);
    }
    printf("\n");
    for (size_t i = 0; i < sizeof(key); i++ ){
        if (i > 0 && i % BLCKSIZE == 0) printf(" ");
        printf("%02"PRIx8"", key[i]);
    }
    printf("\n");
    for (int i = 0; i < ITERATIONS; i++) {
        hash(ct, m, MSGSIZE, key, sizeof(key));
    }
    totcycles = 0;
    ccycles = 0;
    hcycles = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        start = rdtscp_start();
        hash(ct, m, MSGSIZE, key, sizeof(key));
        stop = rdtscp_stop();
        hcycles += stop - start;
    }
    printf("\n");
    double dhcycles = ((double)hcycles) / ITERATIONS;
    for (size_t i = 0; i < HASHSIZE; i++) {
        printf("%02" PRIx8 "", ct[i]);
    }
    printf("\n");
    printf("Hash Cycles = %f\n", dhcycles);
    printf("Hash Cycles/byte = %f\n", (dhcycles) / MSGSIZE);
    printf("\n");
    printf("==================================================================="
           "\n");
    free(m);
    return 0;
}
