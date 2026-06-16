
#include "cyclecount.h"
#include "test_params.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(USE_RIJNDAEL256)
#include "rijndael256/rijndael256_aead.h"
#define AEAD rijndael_aead
#elif defined(USE_AES256)
#include "aes256/aes256_aead.h"
#define AEAD aes_aead
#elif defined(USE_CHACHA)
#include "chacha/chacha_aead.h"
#define AEAD chacha_aead
#endif

uint64_t /*ccycles=0, hcycles=0,*/ totcycles = 0;
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
    uint8_t *ct = calloc((PADDED_LEN(msgsize) + TAG_SIZE), 1);
    uint8_t *ad = calloc(ad_len, 1);
    memset(ad, 0xff, ad_len);
    unsigned char key[KEY_SIZE] = {0};
    unsigned char nonce[IV_SIZE] = {0};
    uint64_t start, stop;
    // getrandom(nonce, sizeof(nonce), 0);
    for (int i = 0; i < ITERATIONS; i++) {
        AEAD(msgsize, m, ad_len, ad, ct, key, nonce);
    }
    totcycles = 0; // ccycles=0; hcycles=0;
    for (int i = 0; i < ITERATIONS; i++) {
        start = rdtscp_start();
        AEAD(msgsize, m, ad_len, ad, ct, key, nonce);
        stop = rdtscp_stop();
        totcycles += stop - start;
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
        printf("%02" PRIx8 "", ct[PADDED_LEN(msgsize) + i]);
    }
    printf("\n");
    double dtotcycles = ((double)totcycles) / ITERATIONS;
    printf("Tot Cycles = %f\n", dtotcycles);
    printf("Tot Cycles/byte = %f\n", dtotcycles / msgsize);

    return 0;
}
