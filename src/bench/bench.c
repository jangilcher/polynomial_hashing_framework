// MIT License
//
// Copyright (c) 2023 Jan Gilcher, Jérôme Govinden
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "../include/hash.h"
#include "../randombytes.h"
#include "cyclecount.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef ITERATIONS
#define ITERATIONS 1024
#endif
#ifndef REPETITIONS
#define REPETITIONS 15
#endif
#ifndef MAXINPUTSIZE
#define MAXINPUTSIZE (1 << 14)
#endif
#ifndef STEPSIZE
#define STEPSIZE 100
#endif

#ifndef NAME
#define NAME "null"
#endif

#ifndef FOLDER
#define FOLDER "./"
#endif

void do_bench(size_t message_len, FILE *f) {
    unsigned char *message = malloc(message_len);
    if (!message) {
        exit(-1);
    }

    unsigned char key[KEYSIZE];
    unsigned char mac[CRYPTO_HASH];
    uint64_t start = 0U, stop = 0U, time = 0U;

    for (int i = 0; i < 1000; i++) {
        randbytes(message, message_len);
        randbytes(key, sizeof key);
        hash(mac, message, message_len, key);
    }

    // char name[sizeof(ALGORITHM_NAME)+10];
    // sprintf(name, "%s_%zu", ALGORITHM_NAME, message_len);

    for (int i = 0; i < ITERATIONS; i++) {
        randbytes(message, message_len);
        randbytes(key, sizeof key);
        start = rdtscp_start();
        hash(mac, message, message_len, key);
        stop = rdtscp_stop();
        time += stop - start;
    }

    uint64_t correction = 0U;
    for (int i = 0; i < ITERATIONS; i++) {
        start = rdtscp_start();
        stop = rdtscp_stop();
        correction += stop - start;
    }
    time -= correction;

    // if (message_len < 1024U) {
    //     printf("MessageLength: %zu B\n", message_len);
    // } else if(message_len < (1U<<20)) {
    //     printf("MessageLength: %zu KB\n", message_len>>10);
    // } else {
    //     printf("MessageLength: %zu MB\n", message_len>>20);
    // }

    // printf("%s:\t%" PRIu64 " Cycles\n", ALGORITHM_NAME, time/ITERATIONS);
    fprintf(f, "%zu,%f\n", message_len, ((double)time) / ITERATIONS);
    free(message);
}

int main(int argc, char *argv[]) {
    if (init_lib() < 0) {
        return -1;
    }

    FILE *f = fopen(FOLDER "" NAME "_results.csv", "w");
    fprintf(f, "MessageLength,cycles\n");
    // size_t inputsize = 0;
    // size_t samplessize = ((MAXINPUTSIZE) / 8) > sizeof(inputsize) ?
    // sizeof(inputsize) : ((MAXINPUTSIZE) / 8);

    // uint8_t* mask = calloc(samplessize, sizeof(uint8_t));
    // if (!mask) {
    //     exit(-1);
    // }
    for (int j = 0; j < REPETITIONS; j++) {
        for (int i = 0; i <= MAXINPUTSIZE;) {
            // while (!(mask[inputsize >> 3] & (1 << (inputsize & 0x7)))){
            //     do {
            //         randombytes((unsigned char *) &inputsize, samplessize);
            //     }
            //     while (inputsize > MAXINPUTSIZE);
            // }
            // mask[inputsize >> 3] |= (1 << (inputsize & 0x7));
            i += STEPSIZE;
            do_bench(i, f);
        }
    }
    fclose(f);
    return 0;
}
