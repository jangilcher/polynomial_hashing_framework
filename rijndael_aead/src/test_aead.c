#include "test_aead.h"
#include "cyclecount.h"
#include "test_params.h"
#include <immintrin.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/random.h>
#include <string.h>

int main(int argc, char *argv[])
{
    uint32_t msgsize;
    uint32_t ad_len;
    int res = 0;
    if (argc > 1) {
        res = sscanf(argv[1], "%u", &msgsize);
        if (res == 0)
            msgsize = 0;
        if (argc > 2) {
            res = sscanf(argv[2], "%u", &ad_len);
            if (res == 0)
                ad_len = 0;
        } else {
            ad_len = ADLEN;
        }
    } else {
        msgsize = MSGSIZE;
        ad_len = ADLEN;
    }
    uint8_t *m = calloc(msgsize, 1);
    uint8_t *ct = calloc((CTXT_LEN(msgsize)), 1);
    uint8_t *ad = calloc(ad_len, 1);
    memset(ad, 0xff, ad_len);
       
    uint8_t key[KEY_SIZE] = {0};
    uint8_t nonce[NONCE_SIZE] = {0};

    for (int i = 0; i < ITERATIONS; i++) {
        AEAD(msgsize, m, ad_len, ad, ct, key, nonce);
    }
    uint64_t start, stop, cycles = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        start = rdtscp_start();
        AEAD(msgsize, m, ad_len, ad, ct, key, nonce);
        stop = rdtscp_stop();
        cycles += stop - start;
    }
    printf("ct:\n");
    for (size_t i = 0; i < PADDED_LEN(msgsize); i++) {
        if (i > 0 && (i % 32 == 0))
            printf("\n");
        printf("%02" PRIx8 "", ct[i]);
    }
    printf("\n");
    printf("Tag:\n");
    for (size_t i = 0; i < TAG_SIZE; i++) {
        if (i > 0 && (i % 32 == 0))
            printf("\n");
        printf("%02" PRIx8 "", ct[i + PADDED_LEN(msgsize)]);
    }
    printf("\n");
    double dcycles = ((double)cycles) / ITERATIONS;
    printf("Cycles = %f\n", dcycles);
    printf("Cycles/byte = %f\n", dcycles / msgsize);
    free(m);
    free(ct);
    free(ad);
}