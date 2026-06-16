#include "cyclecount.h"
#include "test_aead.h"
#include <immintrin.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/random.h>

#ifndef REPETITIONS
#define REPETITIONS 1000
#endif
#ifndef ITERATIONS
#define ITERATIONS 1024
#endif
#ifndef MAXINPUTSIZE
#define MAXINPUTSIZE 16000
#endif
#ifndef ADLEN
#define ADLEN 0
#endif
#ifndef STEPSIZE
#define STEPSIZE 500
#endif
#ifndef FOLDER
#define FOLDER "./"
#endif
#ifndef NAME
#define NAME "test"
#endif


void randbytes(unsigned char *const buf, const size_t num)
{
    size_t got = 0;
    do {
        got += getrandom(buf + got, num - got, 0);
        if (got == -1) {
            abort();
        }
    } while (got < num);
}

void do_bench(size_t msgsize, size_t ad_len, FILE *f)
{
    // printf("ML: %zu\n", message_len);
    uint8_t *m = malloc(msgsize);
    if (!m) {
        abort();
    }
    uint8_t *ct = calloc((CTXT_LEN(msgsize)), 1);
    if (!ct) {
        abort();
    }
    uint8_t *ad = malloc(ad_len);
    if (!ad) {
        abort();
    }

    uint8_t key[KEY_SIZE] = {0};
    uint8_t nonce[NONCE_SIZE] = {0};

    for (int i = 0; i < ITERATIONS; i++) {
        randbytes(nonce, NONCE_SIZE);
        randbytes(key, KEY_SIZE);
        randbytes(m, msgsize);
        randbytes(ad, ad_len);
        AEAD(msgsize, m, ad_len, ad, ct, key, nonce);
    }
    uint64_t start, stop, cycles = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        randbytes(nonce, NONCE_SIZE);
        randbytes(key, KEY_SIZE);
        randbytes(m, msgsize);
        randbytes(ad, ad_len);
        start = rdtscp_start();
        AEAD(msgsize, m, ad_len, ad, ct, key, nonce);
        stop = rdtscp_stop();
        cycles += stop - start;
    }
    uint64_t correction = 0U;
    for (int i = 0; i < ITERATIONS; i++) {
        start = rdtscp_start();
        stop = rdtscp_stop();
        correction += stop - start;
    }
    cycles -= correction;

    fprintf(f, "%zu,%zu,%f\n", msgsize, ad_len, ((double)cycles) / ITERATIONS);

    free(m);
    free(ct);
    free(ad);
}

int main(int argc, char *argv[])
{
    FILE *f = fopen(FOLDER "" NAME "_results.csv", "w");

    fprintf(f, "MessageLength,ADLength,cycles\n");

    for (int j = 0; j < REPETITIONS; j++) {
        for (int i = 0; i <= MAXINPUTSIZE;) {
                i += STEPSIZE;
                do_bench(i, ADLEN, f);
        }
    }
    fclose(f);
    return 0;
}
