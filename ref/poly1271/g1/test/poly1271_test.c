#include "measure.h"
#include "poly1271.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define change_input(x, y, z)                                                  \
    { x[0] = y[0] ^ z[0]; }
#define FILE stdout

int main(int argc, char **argv) {

    uint64 i, l, n, r;

    uchar8 h[KEY_SIZE] = {0};

    uchar8 k[KEY_SIZE] = {0xee, 0xa6, 0xa7, 0x25, 0x1c, 0x1e, 0x72, 0x91,
                          0x6d, 0x11, 0xc2, 0xcb, 0x21, 0x4d, 0x3c, 0x25};

    uchar8 m[MSG_SIZE] = {
#include "message.txt"
    };

    k[KEY_SIZE - 1] = k[KEY_SIZE - 1] & 0x3F;

    if (argc != 2) {
        fprintf(FILE, "\nNo of arguments mismatch. Aborting!\n\n");
        exit(1);
    }
    l = atoi(argv[1]);
    if (l < 0 || l > 65536) {
        fprintf(FILE, "\nInput to the program out of range. Aborting!\n\n");
        exit(1);
    }

    poly1271(h, m, k, l);
    fprintf(FILE, "\n");

    fprintf(FILE, "Hash of the message: ");
    for (i = KEY_SIZE - 1; i > 0; --i)
        fprintf(FILE, "%x", h[i]);
    fprintf(FILE, "%x", h[0]);
    fprintf(FILE, "\n\n");

#define TIME
#ifdef TIME
    fprintf(FILE, "Computing CPU-cycles...\n\n");
    n = l % 8 == 0 ? l / 8 : l / 8 + 1;
    if (l == 0)
        n = 1;
    srand(time(0));
    for (i = 0; i < KEY_SIZE; ++i)
        k[i] = rand() % 0xFF;
    for (i = 0; i < n; ++i)
        m[i] = rand() % 0xFF;

    MEASURE_TIME({
        poly1271(h, m, k, l);
        change_input(m, h, k);
    });
    fprintf(FILE,
            "CPU-cycles per byte for computing poly1271 using sl-maax type "
            "computation on a message of length %llu bits: %6.4lf\n\n",
            l, ((get_median()) / (double)(n * N)));
#endif

    return 0;
}
