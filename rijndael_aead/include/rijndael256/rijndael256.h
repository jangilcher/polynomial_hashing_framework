#pragma once
#include <stdint.h>
#include <immintrin.h>
#define ROUNDKEYS 30
#define BLCKSIZE 32
// #define PADDED_LEN(arg1) (arg1 + ((BLCKSIZE - ((arg1) % BLCKSIZE)) % BLCKSIZE))

typedef union {
    __m128i v;
    uint64_t i[2];
    uint32_t i32[4];
    uint8_t  i8[16];
} vector;

const static vector shufmask = {.i = {0x0b0a050407060100, 0x03020d0c0f0e0908}};
const static vector blendmask = {.i = {0x0000FFFF000000FF, 0x00FFFFFF0000FFFF}};
const static vector mask = {.i = {0xFFFFFFFFFFFFFFFF, 0}};
const static vector polynomial = {.i = {0x425, 0}};
const static vector zero = {.i = {0, 0}};
const static vector one = {.i32 = {0, 0, 0, 1}};
const static vector two = {.i32 = {0, 0, 0, 2}};
const static vector three = {.i32 = {0, 0, 0, 3}};
const static vector four = {.i32 = {0, 0, 0, 4}};
const static vector five = {.i32 = {0, 0, 0, 5}};
const static vector six = {.i32 = {0, 0, 0, 6}};
const static vector seven = {.i32 = {0, 0, 0, 7}};
const static vector eight = {.i32 = {0, 0, 0, 8}};
const static vector nine = {.i32 = {0, 0, 0, 9}};
const static vector ivmask = {.i = {0, -1ULL}};
const static vector iv_load_mask = {.i32 = {-1, -1, -1, 0}};

void rijndael256block(__m128i out[2], const __m128i roundkeys[ROUNDKEYS], const __m128i in[2]);
void rijndael256_ctr(const uint8_t* m, size_t m_len, uint8_t* c, const __m128i roundkeys[ROUNDKEYS], const __m128i iv[2]);
void rijndael256_ctrx2(const uint8_t* m, size_t m_len, uint8_t* c, const __m128i roundkeys[ROUNDKEYS], const __m128i iv[2]);
void rijndael256_ctrx3(const uint8_t* m, size_t m_len, uint8_t* c, const __m128i roundkeys[ROUNDKEYS], const __m128i iv[2]);
void rijndael256_ctrx4(const uint8_t* m, size_t m_len, uint8_t* c, const __m128i roundkeys[ROUNDKEYS], const __m128i iv[2]);

void rijndael256_key_expansion(__m128i out[static ROUNDKEYS], const __m128i inKeys[static 2]);

