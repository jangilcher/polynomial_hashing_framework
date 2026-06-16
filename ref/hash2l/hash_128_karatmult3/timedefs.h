#ifndef _TIMEDEFS_H_
#define _TIMEDEFS_H_

#include <wmmintrin.h>

#include <xmmintrin.h>

#include <smmintrin.h>

#include <emmintrin.h>

#include <tmmintrin.h>

#include <malloc.h>

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#define TIME_TEST 1 /*if measuring time then 1 else 0 */

#define TYPE 0 /* defines type of masking, see maskingNew.h for more */

#define align __attribute__((aligned(16)))

#if !defined(ALIGN16)
#if defined(__GNUC__)
#define ALIGN16 __attribute__((aligned(16)))
#else
#define ALIGN16 __declspec(align(16))
#endif
#endif

// #define BPI 8  /* Number of AES calls to be done in parallel. Do not change
// this, other values not implemented */

#define CACHE_WARM_ITER 1000

#ifndef MAX_ITER
#define MAX_ITER 2000
#endif

#define M 1000

#define N 1000

#define STAMP                                                                  \
    ({                                                                         \
        unsigned res;                                                          \
        __asm__ __volatile__("rdtsc" : "=a"(res) : : "edx");                   \
        res;                                                                   \
    }) /* Time stamp */

#define DO(x)                                                                  \
    do {                                                                       \
        int i, j;                                                              \
        for (i = 0; i < M; i++) {                                              \
            unsigned c2, c1;                                                   \
            for (j = 0; j < CACHE_WARM_ITER; j++) {                            \
                x;                                                             \
            }                                                                  \
            c1 = STAMP;                                                        \
            for (j = 1; j <= N; j++) {                                         \
                x;                                                             \
            }                                                                  \
            c1 = STAMP - c1;                                                   \
            median_next(c1);                                                   \
        }                                                                      \
    } while (0)

#if (TIME_TEST == 1)
unsigned values[MAX_ITER];
int num_values = 0;

int comp(const void *x, const void *y) {
    return *(unsigned *)x - *(unsigned *)y;
}

void median_next(unsigned x) { values[num_values++] = x; }

unsigned median_get(void) {
    unsigned res;
    /*for (res = 0; res < num_values; res++)
    //   printf("%d ", values[res]);
    //printf("\n");*/
    qsort(values, num_values, sizeof(unsigned), comp);
    res = values[num_values / 2];
    num_values = 0;
    return res;
}

void median_print(void) {
    int res;
    qsort(values, num_values, sizeof(unsigned), comp);
    for (res = 0; res < num_values; res++)
        printf("%d ", values[res]);
    printf("\n");
}

#endif

/*struct aeadExpandedKeys { __m128i keySch[11];
                          __m128i dkeySch[11];
                        };*/

typedef struct message {
    unsigned char *content;
    int ml; /* message length in bytes */
} MESSAGE;

void printBytesM(__m128i *data) {
    int i;
    unsigned char *ar = (unsigned char *)data;

    for (i = 0; i < 16; i++)
        printf("%02x ", ar[i]);
    printf("\n");
}

void printBytes(unsigned char *data, int num) {
    int i;

    for (i = 0; i < num; i++)
        printf("%02x ", data[i]);
    printf("\n");
}

int compare(unsigned char *data1, unsigned char *data2, int len) {
    int i, flag = 1;

    for (i = 0; i < len; i++) {
        if (data1[i] != data2[i])
            flag = 0;
    }
    return (flag);
}

MESSAGE *readFile(char *fileName) {
    FILE *fp;
    int i, size, blocks, lastBlock;
    unsigned char *a;
    MESSAGE *m;

    fp = fopen(fileName, "r");
    if (fp == NULL) {
        printf("file %s not found\n", fileName);
        exit(0);
    }

    fseek(fp, 0L, SEEK_END);
    size = (int)ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    blocks = (int)size / 16;

    // printf("size = %d, blocks = %d\n", size, blocks);

    lastBlock = size - 16 * blocks;
    if (lastBlock > 0)
        blocks++;

    a = (unsigned char *)_mm_malloc(blocks * 16 * sizeof(unsigned char), 16);

    for (i = 0; i < size; i++) {
        fread(&a[i], sizeof(unsigned char), 1, fp);
        // printf("%x ",a[i]);
    }

    fclose(fp);
    m = (MESSAGE *)_mm_malloc(sizeof(MESSAGE), 16);
    m->content = a;
    m->ml = size;

    return m;
}

MESSAGE *vecRead(char *fileName, unsigned char id) {
    FILE *fp;
    int i, size, blocks, lastBlock;
    unsigned char *a;
    MESSAGE *m;

    fp = fopen(fileName, "r");
    if (fp == NULL) {
        printf("file %s not found\n", fileName);
        exit(0);
    }

    fseek(fp, 0L, SEEK_END);
    size = (int)ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    size = size + 1;
    // printf("size = %d, blocks = %d\n", size, blocks);

    blocks = (int)size / 16;
    lastBlock = size - 16 * blocks;
    if (lastBlock > 0)
        blocks++;

    a = (unsigned char *)_mm_malloc(blocks * 16 * sizeof(unsigned char), 16);

    a[0] = id;

    for (i = 1; i < size; i++) {
        fread(&a[i], sizeof(unsigned char), 1, fp);
        // printf("%x ",a[i]);
    }

    fclose(fp);
    m = (MESSAGE *)_mm_malloc(sizeof(MESSAGE), 16);
    m->content = a;
    m->ml = size;

    return m;
}

#endif
