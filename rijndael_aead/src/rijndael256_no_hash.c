// #define DEBUG 1
#include <assert.h>
#include <immintrin.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "rijndael256/rijndael256.h"
#include "rijndael256/rijndael256_no_hash.h"
#include "rijndael256_impl.h"

typedef union {
    __m256i *lv;
    __m128i *v;
    __uint8_t *c;
} vec_ptr;

const vector BYTE_IDX = {
    .i8 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}};

#define SPRBLCKSIZE 36
#define CARRYPROPCYCLE 16
#if CARRYPROPCYCLE == 0
#define CARRYPROP
#else
#define CARRYPROP                                                              \
    if (blck_num % CARRYPROPCYCLE == 0) {                                      \
        carry_round(&acc, &dacc);                                              \
        dacc.val[0] = (uint128_t)acc.val[0];                                   \
        dacc.val[1] = (uint128_t)acc.val[1];                                   \
        dacc.val[2] = (uint128_t)acc.val[2];                                   \
        dacc.val[3] = (uint128_t)acc.val[3];                                   \
    }
#endif
void rijndael256_no_hash(
    size_t m_len, const uint8_t m[static m_len], size_t ad_len,
    const uint8_t ad[static ad_len], uint8_t c[static CTXT_LEN(m_len)],
    const uint8_t key[static KEY_SIZE], const uint8_t nonce[NONCE_SIZE])
{
    __m128i store_mask;

    vec_ptr m_ptr = {.v = (__m128i *)m};
    vec_ptr ct_ptr = {.v = (__m128i *)c};
    
    __m128i ct[2] = {0};
    __m128i msg[2] = {0};
    __m128i iv[2][2];
    size_t to_enc = m_len;
    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    iv[0][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5], ((uint32_t *)nonce)[6], 0}}).v;
    
#ifndef __GNUC__
#pragma region ENCRYPTION
#endif
    /* PROCESS M */
    while (to_enc >= BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0 + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 0 + 1);
        m_ptr.v += 2;
        to_enc -= BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.v += 2;
    }
    if (to_enc > 16) {
        msg[0] = _mm_loadu_si128(m_ptr.v);
        msg[1] = _mm_setzero_si128();
        memcpy(&msg[1], m_ptr.v + 1, to_enc - 16);
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        store_mask = _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)to_enc - 16),
                                    BYTE_IDX.v);
        ct[1] = _mm_and_si128(ct[1], store_mask);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        memcpy(ct_ptr.v + 1, &ct[1], to_enc - 16);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.c += to_enc;
        to_enc -= to_enc;
    } else if (to_enc > 0) {
        msg[0] = _mm_setzero_si128();
        memcpy(&msg[0], m_ptr.v, to_enc);
        msg[1] = _mm_setzero_si128();
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        store_mask =
            _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)to_enc), BYTE_IDX.v);
        ct[0] = _mm_and_si128(ct[0], store_mask);
        memcpy(ct_ptr.v + 0, &ct[0], to_enc);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.c += to_enc;
        to_enc -= to_enc;
    }
#ifndef __GNUC__
#pragma endregion ENCRYPTION
#endif
    return;
}

void rijndael256x2_no_hash(
    size_t m_len, const uint8_t m[static m_len], size_t ad_len,
    const uint8_t ad[static ad_len], uint8_t c[static CTXT_LEN(m_len)],
    const uint8_t key[static KEY_SIZE], const uint8_t nonce[NONCE_SIZE])
{
    __m128i store_mask;

    vec_ptr m_ptr = {.v = (__m128i *)m};
    vec_ptr ct_ptr = {.v = (__m128i *)c};
    
    __m128i ct[4] = {0};
    __m128i msg[4] = {0};
    __m128i iv[2][2];
    size_t to_enc = m_len;
    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    iv[0][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5], ((uint32_t *)nonce)[6], 0}}).v;
    iv[1][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[1][1] = _mm_add_epi32(iv[0][1], one.v);

#ifndef __GNUC__
#pragma region ENCRYPTION
#endif
    /* PROCESS M */
    while (to_enc >= 2*BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        m_ptr.v += 4;
        to_enc -= 2*BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        rijndael256blockXOR(&ct[2], roundkeys, iv[1], &msg[2]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        iv[0][1] = _mm_add_epi32(iv[0][1], two.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], two.v);
        ct_ptr.v += 4;
    }
    while (to_enc >= BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        m_ptr.v += 2;
        to_enc -= BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.v += 2;
    }
    if (to_enc > 16) {
        msg[0] = _mm_loadu_si128(m_ptr.v);
        msg[1] = _mm_setzero_si128();
        memcpy(&msg[1], m_ptr.v + 1, to_enc - 16);
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        store_mask = _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)to_enc - 16),
                                    BYTE_IDX.v);
        ct[1] = _mm_and_si128(ct[1], store_mask);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        memcpy(ct_ptr.v + 1, &ct[1], to_enc - 16);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.c += to_enc;
        to_enc -= to_enc;
    } else if (to_enc > 0) {
        msg[0] = _mm_setzero_si128();
        memcpy(&msg[0], m_ptr.v, to_enc);
        msg[1] = _mm_setzero_si128();
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        store_mask =
            _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)to_enc), BYTE_IDX.v);
        ct[0] = _mm_and_si128(ct[0], store_mask);
        memcpy(ct_ptr.v + 0, &ct[0], to_enc);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.c += to_enc;
        to_enc -= to_enc;
    }
#ifndef __GNUC__
#pragma endregion ENCRYPTION
#endif
    return;
}

void rijndael256x2x_no_hash(
    size_t m_len, const uint8_t m[static m_len], size_t ad_len,
    const uint8_t ad[static ad_len], uint8_t c[static CTXT_LEN(m_len)],
    const uint8_t key[static KEY_SIZE], const uint8_t nonce[NONCE_SIZE])
{
    __m128i store_mask;

    vec_ptr m_ptr = {.v = (__m128i *)m};
    vec_ptr ct_ptr = {.v = (__m128i *)c};
    
    __m128i ct[4] = {0};
    __m128i msg[4] = {0};
    __m128i iv[2][2];
    size_t to_enc = m_len;
    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    iv[0][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5], ((uint32_t *)nonce)[6], 0}}).v;
    iv[1][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[1][1] = _mm_add_epi32(iv[0][1], one.v);

#ifndef __GNUC__
#pragma region ENCRYPTION
#endif
    /* PROCESS M */
    while (to_enc >= 2*BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        m_ptr.v += 4;
        to_enc -= 2*BLCKSIZE;
        rijndael256blockXORx2(&ct[0], roundkeys, (__m128i *)iv, &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        iv[0][1] = _mm_add_epi32(iv[0][1], two.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], two.v);
        ct_ptr.v += 4;
    }
    while (to_enc >= BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        m_ptr.v += 2;
        to_enc -= BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.v += 2;
    }
    if (to_enc > 16) {
        msg[0] = _mm_loadu_si128(m_ptr.v);
        msg[1] = _mm_setzero_si128();
        memcpy(&msg[1], m_ptr.v + 1, to_enc - 16);
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        store_mask = _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)to_enc - 16),
                                    BYTE_IDX.v);
        ct[1] = _mm_and_si128(ct[1], store_mask);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        memcpy(ct_ptr.v + 1, &ct[1], to_enc - 16);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.c += to_enc;
        to_enc -= to_enc;
    } else if (to_enc > 0) {
        msg[0] = _mm_setzero_si128();
        memcpy(&msg[0], m_ptr.v, to_enc);
        msg[1] = _mm_setzero_si128();
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        store_mask =
            _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)to_enc), BYTE_IDX.v);
        ct[0] = _mm_and_si128(ct[0], store_mask);
        memcpy(ct_ptr.v + 0, &ct[0], to_enc);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.c += to_enc;
        to_enc -= to_enc;
    }
#ifndef __GNUC__
#pragma endregion ENCRYPTION
#endif
    return;
}

void rijndael256x3_no_hash(
    size_t m_len, const uint8_t m[static m_len], size_t ad_len,
    const uint8_t ad[static ad_len], uint8_t c[static CTXT_LEN(m_len)],
    const uint8_t key[static KEY_SIZE], const uint8_t nonce[NONCE_SIZE])
{
    __m128i store_mask;

    vec_ptr m_ptr = {.v = (__m128i *)m};
    vec_ptr ct_ptr = {.v = (__m128i *)c};
    
    __m128i ct[6] = {0};
    __m128i msg[6] = {0};
    __m128i iv[3][2];
    size_t to_enc = m_len;
    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    iv[0][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5], ((uint32_t *)nonce)[6], 0}}).v;
    iv[1][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[1][1] = _mm_add_epi32(iv[0][1], one.v);
    iv[2][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[2][1] = _mm_add_epi32(iv[0][1], two.v);

#ifndef __GNUC__
#pragma region ENCRYPTION
#endif
    /* PROCESS M */
    
    while (to_enc >= 3*BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        m_ptr.v += 6;
        to_enc -= 3*BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        rijndael256blockXOR(&ct[2], roundkeys, iv[1], &msg[2]);
        rijndael256blockXOR(&ct[4], roundkeys, iv[2], &msg[4]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        _mm_storeu_si128(ct_ptr.v + 4, ct[4]);
        _mm_storeu_si128(ct_ptr.v + 5, ct[5]);
        iv[0][1] = _mm_add_epi32(iv[0][1], three.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], three.v);
        iv[2][1] = _mm_add_epi32(iv[2][1], three.v);
        ct_ptr.v += 6;
    }
    if (to_enc >= 2*BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        m_ptr.v += 4;
        to_enc -= 2*BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        rijndael256blockXOR(&ct[2], roundkeys, iv[1], &msg[2]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        iv[0][1] = _mm_add_epi32(iv[0][1], two.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], two.v);
        ct_ptr.v += 4;
    }
    if (to_enc >= BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        m_ptr.v += 2;
        to_enc -= BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.v += 2;
    }
    if (to_enc > 16) {
        msg[0] = _mm_loadu_si128(m_ptr.v);
        msg[1] = _mm_setzero_si128();
        memcpy(&msg[1], m_ptr.v + 1, to_enc - 16);
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        store_mask = _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)to_enc - 16),
                                    BYTE_IDX.v);
        ct[1] = _mm_and_si128(ct[1], store_mask);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        memcpy(ct_ptr.v + 1, &ct[1], to_enc - 16);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.c += to_enc;
        to_enc -= to_enc;
    } else if (to_enc > 0) {
        msg[0] = _mm_setzero_si128();
        memcpy(&msg[0], m_ptr.v, to_enc);
        msg[1] = _mm_setzero_si128();
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        store_mask =
            _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)to_enc), BYTE_IDX.v);
        ct[0] = _mm_and_si128(ct[0], store_mask);
        memcpy(ct_ptr.v + 0, &ct[0], to_enc);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.c += to_enc;
        to_enc -= to_enc;
    }
#ifndef __GNUC__
#pragma endregion ENCRYPTION
#endif
    return;
}

void rijndael256x3x_no_hash(
    size_t m_len, const uint8_t m[static m_len], size_t ad_len,
    const uint8_t ad[static ad_len], uint8_t c[static CTXT_LEN(m_len)],
    const uint8_t key[static KEY_SIZE], const uint8_t nonce[NONCE_SIZE])
{
    __m128i store_mask;

    vec_ptr m_ptr = {.v = (__m128i *)m};
    vec_ptr ct_ptr = {.v = (__m128i *)c};
    
    __m128i ct[6] = {0};
    __m128i msg[6] = {0};
    __m128i iv[3][2];
    size_t to_enc = m_len;
    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    iv[0][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5], ((uint32_t *)nonce)[6], 0}}).v;
    iv[1][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[1][1] = _mm_add_epi32(iv[0][1], one.v);
    iv[2][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[2][1] = _mm_add_epi32(iv[0][1], two.v);

#ifndef __GNUC__
#pragma region ENCRYPTION
#endif
    /* PROCESS M */
    
    while (to_enc >= 3*BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        m_ptr.v += 6;
        to_enc -= 3*BLCKSIZE;
        rijndael256blockXORx3(&ct[0], roundkeys, (__m128i *)iv, &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        _mm_storeu_si128(ct_ptr.v + 4, ct[4]);
        _mm_storeu_si128(ct_ptr.v + 5, ct[5]);
        iv[0][1] = _mm_add_epi32(iv[0][1], three.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], three.v);
        iv[2][1] = _mm_add_epi32(iv[2][1], three.v);
        ct_ptr.v += 6;
    }
    if (to_enc >= 2*BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        m_ptr.v += 4;
        to_enc -= 2*BLCKSIZE;
        rijndael256blockXORx2(&ct[0], roundkeys, (__m128i *)iv, &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        iv[0][1] = _mm_add_epi32(iv[0][1], two.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], two.v);
        ct_ptr.v += 4;
    }
    if (to_enc >= BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        m_ptr.v += 2;
        to_enc -= BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.v += 2;
    }
    if (to_enc > 16) {
        msg[0] = _mm_loadu_si128(m_ptr.v);
        msg[1] = _mm_setzero_si128();
        memcpy(&msg[1], m_ptr.v + 1, to_enc - 16);
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        store_mask = _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)to_enc - 16),
                                    BYTE_IDX.v);
        ct[1] = _mm_and_si128(ct[1], store_mask);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        memcpy(ct_ptr.v + 1, &ct[1], to_enc - 16);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.c += to_enc;
        to_enc -= to_enc;
    } else if (to_enc > 0) {
        msg[0] = _mm_setzero_si128();
        memcpy(&msg[0], m_ptr.v, to_enc);
        msg[1] = _mm_setzero_si128();
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        store_mask =
            _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)to_enc), BYTE_IDX.v);
        ct[0] = _mm_and_si128(ct[0], store_mask);
        memcpy(ct_ptr.v + 0, &ct[0], to_enc);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.c += to_enc;
        to_enc -= to_enc;
    }
#ifndef __GNUC__
#pragma endregion ENCRYPTION
#endif
    return;
}

void rijndael256x4_no_hash(
    size_t m_len, const uint8_t m[static m_len], size_t ad_len,
    const uint8_t ad[static ad_len], uint8_t c[static CTXT_LEN(m_len)],
    const uint8_t key[static KEY_SIZE], const uint8_t nonce[NONCE_SIZE])
{
    __m128i store_mask;

    vec_ptr m_ptr = {.v = (__m128i *)m};
    vec_ptr ct_ptr = {.v = (__m128i *)c};
    
    __m128i ct[8] = {0};
    __m128i msg[8] = {0};
    __m128i iv[4][2];
    size_t to_enc = m_len;
    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    iv[0][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5], ((uint32_t *)nonce)[6], 0}}).v;
    iv[1][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[1][1] = _mm_add_epi32(iv[0][1], one.v);
    iv[2][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[2][1] = _mm_add_epi32(iv[0][1], two.v);
    iv[3][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[3][1] = _mm_add_epi32(iv[0][1], three.v);


#ifndef __GNUC__
#pragma region ENCRYPTION
#endif
    /* PROCESS M */
    while (to_enc >= 4*BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        msg[6] = _mm_loadu_si128(m_ptr.v + 6);
        msg[7] = _mm_loadu_si128(m_ptr.v + 7);
        m_ptr.v += 8;
        to_enc -= 4*BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        rijndael256blockXOR(&ct[2], roundkeys, iv[1], &msg[2]);
        rijndael256blockXOR(&ct[4], roundkeys, iv[2], &msg[4]);
        rijndael256blockXOR(&ct[6], roundkeys, iv[3], &msg[6]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        _mm_storeu_si128(ct_ptr.v + 4, ct[4]);
        _mm_storeu_si128(ct_ptr.v + 5, ct[5]);
        _mm_storeu_si128(ct_ptr.v + 6, ct[6]);
        _mm_storeu_si128(ct_ptr.v + 7, ct[7]);
        iv[0][1] = _mm_add_epi32(iv[0][1], four.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], four.v);
        iv[2][1] = _mm_add_epi32(iv[2][1], four.v);
        iv[3][1] = _mm_add_epi32(iv[3][1], four.v);
        ct_ptr.v += 8;
    }
    if (to_enc >= 3*BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        m_ptr.v += 6;
        to_enc -= 3*BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        rijndael256blockXOR(&ct[2], roundkeys, iv[1], &msg[2]);
        rijndael256blockXOR(&ct[4], roundkeys, iv[2], &msg[4]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        _mm_storeu_si128(ct_ptr.v + 4, ct[4]);
        _mm_storeu_si128(ct_ptr.v + 5, ct[5]);
        iv[0][1] = _mm_add_epi32(iv[0][1], three.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], three.v);
        iv[1][1] = _mm_add_epi32(iv[2][1], three.v);
        ct_ptr.v += 6;
    }
    if (to_enc >= 2*BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        m_ptr.v += 4;
        to_enc -= 2*BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        rijndael256blockXOR(&ct[2], roundkeys, iv[1], &msg[2]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        iv[0][1] = _mm_add_epi32(iv[0][1], two.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], two.v);
        ct_ptr.v += 4;
    }
    if (to_enc >= BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        m_ptr.v += 2;
        to_enc -= BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.v += 2;
    }
    if (to_enc > 16) {
        msg[0] = _mm_loadu_si128(m_ptr.v);
        msg[1] = _mm_setzero_si128();
        memcpy(&msg[1], m_ptr.v + 1, to_enc - 16);
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        store_mask = _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)to_enc - 16),
                                    BYTE_IDX.v);
        ct[1] = _mm_and_si128(ct[1], store_mask);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        memcpy(ct_ptr.v + 1, &ct[1], to_enc - 16);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.c += to_enc;
        to_enc -= to_enc;
    } else if (to_enc > 0) {
        msg[0] = _mm_setzero_si128();
        memcpy(&msg[0], m_ptr.v, to_enc);
        msg[1] = _mm_setzero_si128();
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        store_mask =
            _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)to_enc), BYTE_IDX.v);
        ct[0] = _mm_and_si128(ct[0], store_mask);
        memcpy(ct_ptr.v + 0, &ct[0], to_enc);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.c += to_enc;
        to_enc -= to_enc;
    }
#ifndef __GNUC__
#pragma endregion ENCRYPTION
#endif
    return;
}

void rijndael256x4x_no_hash(
    size_t m_len, const uint8_t m[static m_len], size_t ad_len,
    const uint8_t ad[static ad_len], uint8_t c[static CTXT_LEN(m_len)],
    const uint8_t key[static KEY_SIZE], const uint8_t nonce[NONCE_SIZE])
{
    __m128i store_mask;

    vec_ptr m_ptr = {.v = (__m128i *)m};
    vec_ptr ct_ptr = {.v = (__m128i *)c};
    
    __m128i ct[8] = {0};
    __m128i msg[8] = {0};
    __m128i iv[4][2];
    size_t to_enc = m_len;
    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    iv[0][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5], ((uint32_t *)nonce)[6], 0}}).v;
    iv[1][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[1][1] = _mm_add_epi32(iv[0][1], one.v);
    iv[2][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[2][1] = _mm_add_epi32(iv[0][1], two.v);
    iv[3][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[3][1] = _mm_add_epi32(iv[0][1], three.v);


#ifndef __GNUC__
#pragma region ENCRYPTION
#endif
    /* PROCESS M */
    while (to_enc >= 4*BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        msg[6] = _mm_loadu_si128(m_ptr.v + 6);
        msg[7] = _mm_loadu_si128(m_ptr.v + 7);
        m_ptr.v += 8;
        to_enc -= 4*BLCKSIZE;
        rijndael256blockXORx4(&ct[0], roundkeys, (__m128i *)iv, &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        _mm_storeu_si128(ct_ptr.v + 4, ct[4]);
        _mm_storeu_si128(ct_ptr.v + 5, ct[5]);
        _mm_storeu_si128(ct_ptr.v + 6, ct[6]);
        _mm_storeu_si128(ct_ptr.v + 7, ct[7]);
        iv[0][1] = _mm_add_epi32(iv[0][1], four.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], four.v);
        iv[2][1] = _mm_add_epi32(iv[2][1], four.v);
        iv[3][1] = _mm_add_epi32(iv[3][1], four.v);
        ct_ptr.v += 8;
    }
    if (to_enc >= 3*BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        m_ptr.v += 6;
        to_enc -= 3*BLCKSIZE;
        rijndael256blockXORx3(&ct[0], roundkeys, (__m128i *)iv, &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        _mm_storeu_si128(ct_ptr.v + 4, ct[4]);
        _mm_storeu_si128(ct_ptr.v + 5, ct[5]);
        iv[0][1] = _mm_add_epi32(iv[0][1], three.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], three.v);
        iv[1][1] = _mm_add_epi32(iv[2][1], three.v);
        ct_ptr.v += 6;
    }
    if (to_enc >= 2*BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        m_ptr.v += 4;
        to_enc -= 2*BLCKSIZE;
        rijndael256blockXORx2(&ct[0], roundkeys, (__m128i *)iv, &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        iv[0][1] = _mm_add_epi32(iv[0][1], two.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], two.v);
        ct_ptr.v += 4;
    }
    if (to_enc >= BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        m_ptr.v += 2;
        to_enc -= BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.v += 2;
    }
    if (to_enc > 16) {
        msg[0] = _mm_loadu_si128(m_ptr.v);
        msg[1] = _mm_setzero_si128();
        memcpy(&msg[1], m_ptr.v + 1, to_enc - 16);
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        store_mask = _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)to_enc - 16),
                                    BYTE_IDX.v);
        ct[1] = _mm_and_si128(ct[1], store_mask);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        memcpy(ct_ptr.v + 1, &ct[1], to_enc - 16);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.c += to_enc;
        to_enc -= to_enc;
    } else if (to_enc > 0) {
        msg[0] = _mm_setzero_si128();
        memcpy(&msg[0], m_ptr.v, to_enc);
        msg[1] = _mm_setzero_si128();
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        store_mask =
            _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)to_enc), BYTE_IDX.v);
        ct[0] = _mm_and_si128(ct[0], store_mask);
        memcpy(ct_ptr.v + 0, &ct[0], to_enc);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.c += to_enc;
        to_enc -= to_enc;
    }
#ifndef __GNUC__
#pragma endregion ENCRYPTION
#endif
    return;
}

void rijndael256x6_no_hash(
    size_t m_len, const uint8_t m[static m_len], size_t ad_len,
    const uint8_t ad[static ad_len], uint8_t c[static CTXT_LEN(m_len)],
    const uint8_t key[static KEY_SIZE], const uint8_t nonce[NONCE_SIZE])
{
    __m128i store_mask;

    vec_ptr m_ptr = {.v = (__m128i *)m};
    vec_ptr ct_ptr = {.v = (__m128i *)c};
    
    __m128i ct[12] = {0};
    __m128i msg[12] = {0};
    __m128i iv[6][2];
    size_t to_enc = m_len;
    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    iv[0][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5], ((uint32_t *)nonce)[6], 0}}).v;
    iv[1][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[1][1] = _mm_add_epi32(iv[0][1], one.v);
    iv[2][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[2][1] = _mm_add_epi32(iv[0][1], two.v);
    iv[3][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[3][1] = _mm_add_epi32(iv[0][1], three.v);
    iv[4][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[4][1] = _mm_add_epi32(iv[0][1], four.v);
    iv[5][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[5][1] = _mm_add_epi32(iv[0][1], five.v);

#ifndef __GNUC__
#pragma region ENCRYPTION
#endif
    /* PROCESS M */
    while (to_enc >= 6*BLCKSIZE) {
        msg[ 0] = _mm_loadu_si128(m_ptr.v +  0);
        msg[ 1] = _mm_loadu_si128(m_ptr.v +  1);
        msg[ 2] = _mm_loadu_si128(m_ptr.v +  2);
        msg[ 3] = _mm_loadu_si128(m_ptr.v +  3);
        msg[ 4] = _mm_loadu_si128(m_ptr.v +  4);
        msg[ 5] = _mm_loadu_si128(m_ptr.v +  5);
        msg[ 6] = _mm_loadu_si128(m_ptr.v +  6);
        msg[ 7] = _mm_loadu_si128(m_ptr.v +  7);
        msg[ 8] = _mm_loadu_si128(m_ptr.v +  8);
        msg[ 9] = _mm_loadu_si128(m_ptr.v +  9);
        msg[10] = _mm_loadu_si128(m_ptr.v + 10);
        msg[11] = _mm_loadu_si128(m_ptr.v + 11);
        m_ptr.v += 12;
        to_enc -= 6*BLCKSIZE;
        rijndael256blockXOR(&ct[ 0], roundkeys, iv[0], &msg[ 0]);
        rijndael256blockXOR(&ct[ 2], roundkeys, iv[1], &msg[ 2]);
        rijndael256blockXOR(&ct[ 4], roundkeys, iv[2], &msg[ 4]);
        rijndael256blockXOR(&ct[ 6], roundkeys, iv[3], &msg[ 6]);
        rijndael256blockXOR(&ct[ 8], roundkeys, iv[4], &msg[ 8]);
        rijndael256blockXOR(&ct[10], roundkeys, iv[5], &msg[10]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[ 0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[ 1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[ 2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[ 3]);
        _mm_storeu_si128(ct_ptr.v + 4, ct[ 4]);
        _mm_storeu_si128(ct_ptr.v + 5, ct[ 5]);
        _mm_storeu_si128(ct_ptr.v + 6, ct[ 6]);
        _mm_storeu_si128(ct_ptr.v + 7, ct[ 7]);
        _mm_storeu_si128(ct_ptr.v + 8, ct[ 8]);
        _mm_storeu_si128(ct_ptr.v + 9, ct[ 9]);
        _mm_storeu_si128(ct_ptr.v +10, ct[10]);
        _mm_storeu_si128(ct_ptr.v +11, ct[11]);
        iv[0][1] = _mm_add_epi32(iv[0][1], six.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], six.v);
        iv[2][1] = _mm_add_epi32(iv[2][1], six.v);
        iv[3][1] = _mm_add_epi32(iv[3][1], six.v);
        iv[4][1] = _mm_add_epi32(iv[4][1], six.v);
        iv[5][1] = _mm_add_epi32(iv[5][1], six.v);
        ct_ptr.v += 12;
    }
    if (to_enc >= 4*BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        msg[6] = _mm_loadu_si128(m_ptr.v + 6);
        msg[7] = _mm_loadu_si128(m_ptr.v + 7);
        m_ptr.v += 8;
        to_enc -= 4*BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        rijndael256blockXOR(&ct[2], roundkeys, iv[1], &msg[2]);
        rijndael256blockXOR(&ct[4], roundkeys, iv[2], &msg[4]);
        rijndael256blockXOR(&ct[6], roundkeys, iv[3], &msg[6]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        _mm_storeu_si128(ct_ptr.v + 4, ct[4]);
        _mm_storeu_si128(ct_ptr.v + 5, ct[5]);
        _mm_storeu_si128(ct_ptr.v + 6, ct[6]);
        _mm_storeu_si128(ct_ptr.v + 7, ct[7]);
        iv[0][1] = _mm_add_epi32(iv[0][1], four.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], four.v);
        iv[2][1] = _mm_add_epi32(iv[2][1], four.v);
        iv[3][1] = _mm_add_epi32(iv[3][1], four.v);
        ct_ptr.v += 8;
    }
    if (to_enc >= 3*BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        m_ptr.v += 6;
        to_enc -= 3*BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        rijndael256blockXOR(&ct[2], roundkeys, iv[1], &msg[2]);
        rijndael256blockXOR(&ct[4], roundkeys, iv[2], &msg[4]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        _mm_storeu_si128(ct_ptr.v + 4, ct[4]);
        _mm_storeu_si128(ct_ptr.v + 5, ct[5]);
        iv[0][1] = _mm_add_epi32(iv[0][1], three.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], three.v);
        iv[1][1] = _mm_add_epi32(iv[2][1], three.v);
        ct_ptr.v += 6;
    }
    if (to_enc >= 2*BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        m_ptr.v += 4;
        to_enc -= 2*BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        rijndael256blockXOR(&ct[2], roundkeys, iv[1], &msg[2]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        iv[0][1] = _mm_add_epi32(iv[0][1], two.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], two.v);
        ct_ptr.v += 4;
    }
    if (to_enc >= BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        m_ptr.v += 2;
        to_enc -= BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.v += 2;
    }
    if (to_enc > 16) {
        msg[0] = _mm_loadu_si128(m_ptr.v);
        msg[1] = _mm_setzero_si128();
        memcpy(&msg[1], m_ptr.v + 1, to_enc - 16);
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        store_mask = _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)to_enc - 16),
                                    BYTE_IDX.v);
        ct[1] = _mm_and_si128(ct[1], store_mask);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        memcpy(ct_ptr.v + 1, &ct[1], to_enc - 16);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.c += to_enc;
        to_enc -= to_enc;
    } else if (to_enc > 0) {
        msg[0] = _mm_setzero_si128();
        memcpy(&msg[0], m_ptr.v, to_enc);
        msg[1] = _mm_setzero_si128();
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        store_mask =
            _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)to_enc), BYTE_IDX.v);
        ct[0] = _mm_and_si128(ct[0], store_mask);
        memcpy(ct_ptr.v + 0, &ct[0], to_enc);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.c += to_enc;
        to_enc -= to_enc;
    }
#ifndef __GNUC__
#pragma endregion ENCRYPTION
#endif
    return;
}

void rijndael256x6x_no_hash(
    size_t m_len, const uint8_t m[static m_len], size_t ad_len,
    const uint8_t ad[static ad_len], uint8_t c[static CTXT_LEN(m_len)],
    const uint8_t key[static KEY_SIZE], const uint8_t nonce[NONCE_SIZE])
{
    __m128i store_mask;

    vec_ptr m_ptr = {.v = (__m128i *)m};
    vec_ptr ct_ptr = {.v = (__m128i *)c};
    
    __m128i ct[12] = {0};
    __m128i msg[12] = {0};
    __m128i iv[6][2];
    size_t to_enc = m_len;
    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    iv[0][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5], ((uint32_t *)nonce)[6], 0}}).v;
    iv[1][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[1][1] = _mm_add_epi32(iv[0][1], one.v);
    iv[2][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[2][1] = _mm_add_epi32(iv[0][1], two.v);
    iv[3][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[3][1] = _mm_add_epi32(iv[0][1], three.v);
    iv[4][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[4][1] = _mm_add_epi32(iv[0][1], four.v);
    iv[5][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[5][1] = _mm_add_epi32(iv[0][1], five.v);

#ifndef __GNUC__
#pragma region ENCRYPTION
#endif
    /* PROCESS M */
    while (to_enc >= 6*BLCKSIZE) {
        msg[ 0] = _mm_loadu_si128(m_ptr.v +  0);
        msg[ 1] = _mm_loadu_si128(m_ptr.v +  1);
        msg[ 2] = _mm_loadu_si128(m_ptr.v +  2);
        msg[ 3] = _mm_loadu_si128(m_ptr.v +  3);
        msg[ 4] = _mm_loadu_si128(m_ptr.v +  4);
        msg[ 5] = _mm_loadu_si128(m_ptr.v +  5);
        msg[ 6] = _mm_loadu_si128(m_ptr.v +  6);
        msg[ 7] = _mm_loadu_si128(m_ptr.v +  7);
        msg[ 8] = _mm_loadu_si128(m_ptr.v +  8);
        msg[ 9] = _mm_loadu_si128(m_ptr.v +  9);
        msg[10] = _mm_loadu_si128(m_ptr.v + 10);
        msg[11] = _mm_loadu_si128(m_ptr.v + 11);
        m_ptr.v += 12;
        to_enc -= 6*BLCKSIZE;
        rijndael256blockXORx6(&ct[ 0], roundkeys, (__m128i *)iv, &msg[ 0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[ 0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[ 1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[ 2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[ 3]);
        _mm_storeu_si128(ct_ptr.v + 4, ct[ 4]);
        _mm_storeu_si128(ct_ptr.v + 5, ct[ 5]);
        _mm_storeu_si128(ct_ptr.v + 6, ct[ 6]);
        _mm_storeu_si128(ct_ptr.v + 7, ct[ 7]);
        _mm_storeu_si128(ct_ptr.v + 8, ct[ 8]);
        _mm_storeu_si128(ct_ptr.v + 9, ct[ 9]);
        _mm_storeu_si128(ct_ptr.v +10, ct[10]);
        _mm_storeu_si128(ct_ptr.v +11, ct[11]);
        iv[0][1] = _mm_add_epi32(iv[0][1], six.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], six.v);
        iv[2][1] = _mm_add_epi32(iv[2][1], six.v);
        iv[3][1] = _mm_add_epi32(iv[3][1], six.v);
        iv[4][1] = _mm_add_epi32(iv[4][1], six.v);
        iv[5][1] = _mm_add_epi32(iv[5][1], six.v);
        ct_ptr.v += 12;
    }
    if (to_enc >= 4*BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        msg[6] = _mm_loadu_si128(m_ptr.v + 6);
        msg[7] = _mm_loadu_si128(m_ptr.v + 7);
        m_ptr.v += 8;
        to_enc -= 4*BLCKSIZE;
        rijndael256blockXORx4(&ct[0], roundkeys, (__m128i *)iv, &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        _mm_storeu_si128(ct_ptr.v + 4, ct[4]);
        _mm_storeu_si128(ct_ptr.v + 5, ct[5]);
        _mm_storeu_si128(ct_ptr.v + 6, ct[6]);
        _mm_storeu_si128(ct_ptr.v + 7, ct[7]);
        iv[0][1] = _mm_add_epi32(iv[0][1], four.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], four.v);
        iv[2][1] = _mm_add_epi32(iv[2][1], four.v);
        iv[3][1] = _mm_add_epi32(iv[3][1], four.v);
        ct_ptr.v += 8;
    }
    if (to_enc >= 3*BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        m_ptr.v += 6;
        to_enc -= 3*BLCKSIZE;
        rijndael256blockXORx3(&ct[0], roundkeys, (__m128i *)iv, &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        _mm_storeu_si128(ct_ptr.v + 4, ct[4]);
        _mm_storeu_si128(ct_ptr.v + 5, ct[5]);
        iv[0][1] = _mm_add_epi32(iv[0][1], three.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], three.v);
        iv[1][1] = _mm_add_epi32(iv[2][1], three.v);
        ct_ptr.v += 6;
    }
    if (to_enc >= 2*BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        m_ptr.v += 4;
        to_enc -= 2*BLCKSIZE;
        rijndael256blockXORx2(&ct[0], roundkeys, (__m128i *)iv, &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        iv[0][1] = _mm_add_epi32(iv[0][1], two.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], two.v);
        ct_ptr.v += 4;
    }
    if (to_enc >= BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        m_ptr.v += 2;
        to_enc -= BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.v += 2;
    }
    if (to_enc > 16) {
        msg[0] = _mm_loadu_si128(m_ptr.v);
        msg[1] = _mm_setzero_si128();
        memcpy(&msg[1], m_ptr.v + 1, to_enc - 16);
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        store_mask = _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)to_enc - 16),
                                    BYTE_IDX.v);
        ct[1] = _mm_and_si128(ct[1], store_mask);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        memcpy(ct_ptr.v + 1, &ct[1], to_enc - 16);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.c += to_enc;
        to_enc -= to_enc;
    } else if (to_enc > 0) {
        msg[0] = _mm_setzero_si128();
        memcpy(&msg[0], m_ptr.v, to_enc);
        msg[1] = _mm_setzero_si128();
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        store_mask =
            _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)to_enc), BYTE_IDX.v);
        ct[0] = _mm_and_si128(ct[0], store_mask);
        memcpy(ct_ptr.v + 0, &ct[0], to_enc);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.c += to_enc;
        to_enc -= to_enc;
    }
#ifndef __GNUC__
#pragma endregion ENCRYPTION
#endif
    return;
}

void rijndael256x8_no_hash(
    size_t m_len, const uint8_t m[static m_len], size_t ad_len,
    const uint8_t ad[static ad_len], uint8_t c[static CTXT_LEN(m_len)],
    const uint8_t key[static KEY_SIZE], const uint8_t nonce[NONCE_SIZE])
{
    __m128i store_mask;

    vec_ptr m_ptr = {.v = (__m128i *)m};
    vec_ptr ct_ptr = {.v = (__m128i *)c};
    
    __m128i ct[16] = {0};
    __m128i msg[16] = {0};
    __m128i iv[8][2];
    size_t to_enc = m_len;
    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    iv[0][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5], ((uint32_t *)nonce)[6], 0}}).v;
    iv[1][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[1][1] = _mm_add_epi32(iv[0][1], one.v);
    iv[2][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[2][1] = _mm_add_epi32(iv[0][1], two.v);
    iv[3][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[3][1] = _mm_add_epi32(iv[0][1], three.v);
    iv[4][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[4][1] = _mm_add_epi32(iv[0][1], four.v);
    iv[5][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[5][1] = _mm_add_epi32(iv[0][1], five.v);
    iv[6][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[6][1] = _mm_add_epi32(iv[0][1], six.v);
    iv[7][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[7][1] = _mm_add_epi32(iv[0][1], seven.v);

#ifndef __GNUC__
#pragma region ENCRYPTION
#endif
    /* PROCESS M */
    while (to_enc >= 8*BLCKSIZE) {
        msg[ 0] = _mm_loadu_si128(m_ptr.v +  0);
        msg[ 1] = _mm_loadu_si128(m_ptr.v +  1);
        msg[ 2] = _mm_loadu_si128(m_ptr.v +  2);
        msg[ 3] = _mm_loadu_si128(m_ptr.v +  3);
        msg[ 4] = _mm_loadu_si128(m_ptr.v +  4);
        msg[ 5] = _mm_loadu_si128(m_ptr.v +  5);
        msg[ 6] = _mm_loadu_si128(m_ptr.v +  6);
        msg[ 7] = _mm_loadu_si128(m_ptr.v +  7);
        msg[ 8] = _mm_loadu_si128(m_ptr.v +  8);
        msg[ 9] = _mm_loadu_si128(m_ptr.v +  9);
        msg[10] = _mm_loadu_si128(m_ptr.v + 10);
        msg[11] = _mm_loadu_si128(m_ptr.v + 11);
        msg[12] = _mm_loadu_si128(m_ptr.v + 12);
        msg[13] = _mm_loadu_si128(m_ptr.v + 13);
        msg[14] = _mm_loadu_si128(m_ptr.v + 14);
        msg[15] = _mm_loadu_si128(m_ptr.v + 15);
        m_ptr.v += 16;
        to_enc -= 8*BLCKSIZE;
        rijndael256blockXOR(&ct[ 0], roundkeys, iv[0], &msg[ 0]);
        rijndael256blockXOR(&ct[ 2], roundkeys, iv[1], &msg[ 2]);
        rijndael256blockXOR(&ct[ 4], roundkeys, iv[2], &msg[ 4]);
        rijndael256blockXOR(&ct[ 6], roundkeys, iv[3], &msg[ 6]);
        rijndael256blockXOR(&ct[ 8], roundkeys, iv[4], &msg[ 8]);
        rijndael256blockXOR(&ct[10], roundkeys, iv[5], &msg[10]);
        rijndael256blockXOR(&ct[12], roundkeys, iv[6], &msg[12]);
        rijndael256blockXOR(&ct[14], roundkeys, iv[7], &msg[14]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[ 0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[ 1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[ 2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[ 3]);
        _mm_storeu_si128(ct_ptr.v + 4, ct[ 4]);
        _mm_storeu_si128(ct_ptr.v + 5, ct[ 5]);
        _mm_storeu_si128(ct_ptr.v + 6, ct[ 6]);
        _mm_storeu_si128(ct_ptr.v + 7, ct[ 7]);
        _mm_storeu_si128(ct_ptr.v + 8, ct[ 8]);
        _mm_storeu_si128(ct_ptr.v + 9, ct[ 9]);
        _mm_storeu_si128(ct_ptr.v +10, ct[10]);
        _mm_storeu_si128(ct_ptr.v +11, ct[11]);
        _mm_storeu_si128(ct_ptr.v +12, ct[12]);
        _mm_storeu_si128(ct_ptr.v +13, ct[13]);
        _mm_storeu_si128(ct_ptr.v +14, ct[14]);
        _mm_storeu_si128(ct_ptr.v +15, ct[15]);
        iv[0][1] = _mm_add_epi32(iv[0][1], six.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], six.v);
        iv[2][1] = _mm_add_epi32(iv[2][1], six.v);
        iv[3][1] = _mm_add_epi32(iv[3][1], six.v);
        iv[4][1] = _mm_add_epi32(iv[4][1], six.v);
        iv[5][1] = _mm_add_epi32(iv[5][1], six.v);
        iv[6][1] = _mm_add_epi32(iv[6][1], six.v);
        iv[7][1] = _mm_add_epi32(iv[7][1], six.v);
        ct_ptr.v += 16;
    }
    if (to_enc >= 6*BLCKSIZE) {
        msg[ 0] = _mm_loadu_si128(m_ptr.v +  0);
        msg[ 1] = _mm_loadu_si128(m_ptr.v +  1);
        msg[ 2] = _mm_loadu_si128(m_ptr.v +  2);
        msg[ 3] = _mm_loadu_si128(m_ptr.v +  3);
        msg[ 4] = _mm_loadu_si128(m_ptr.v +  4);
        msg[ 5] = _mm_loadu_si128(m_ptr.v +  5);
        msg[ 6] = _mm_loadu_si128(m_ptr.v +  6);
        msg[ 7] = _mm_loadu_si128(m_ptr.v +  7);
        msg[ 8] = _mm_loadu_si128(m_ptr.v +  8);
        msg[ 9] = _mm_loadu_si128(m_ptr.v +  9);
        msg[10] = _mm_loadu_si128(m_ptr.v + 10);
        msg[11] = _mm_loadu_si128(m_ptr.v + 11);
        m_ptr.v += 12;
        to_enc -= 6*BLCKSIZE;
        rijndael256blockXOR(&ct[ 0], roundkeys, iv[0], &msg[ 0]);
        rijndael256blockXOR(&ct[ 2], roundkeys, iv[1], &msg[ 2]);
        rijndael256blockXOR(&ct[ 4], roundkeys, iv[2], &msg[ 4]);
        rijndael256blockXOR(&ct[ 6], roundkeys, iv[3], &msg[ 6]);
        rijndael256blockXOR(&ct[ 8], roundkeys, iv[4], &msg[ 8]);
        rijndael256blockXOR(&ct[10], roundkeys, iv[5], &msg[10]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[ 0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[ 1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[ 2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[ 3]);
        _mm_storeu_si128(ct_ptr.v + 4, ct[ 4]);
        _mm_storeu_si128(ct_ptr.v + 5, ct[ 5]);
        _mm_storeu_si128(ct_ptr.v + 6, ct[ 6]);
        _mm_storeu_si128(ct_ptr.v + 7, ct[ 7]);
        _mm_storeu_si128(ct_ptr.v + 8, ct[ 8]);
        _mm_storeu_si128(ct_ptr.v + 9, ct[ 9]);
        _mm_storeu_si128(ct_ptr.v +10, ct[10]);
        _mm_storeu_si128(ct_ptr.v +11, ct[11]);
        iv[0][1] = _mm_add_epi32(iv[0][1], six.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], six.v);
        iv[2][1] = _mm_add_epi32(iv[2][1], six.v);
        iv[3][1] = _mm_add_epi32(iv[3][1], six.v);
        iv[4][1] = _mm_add_epi32(iv[4][1], six.v);
        iv[5][1] = _mm_add_epi32(iv[5][1], six.v);
        ct_ptr.v += 12;
    }
    if (to_enc >= 4*BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        msg[6] = _mm_loadu_si128(m_ptr.v + 6);
        msg[7] = _mm_loadu_si128(m_ptr.v + 7);
        m_ptr.v += 8;
        to_enc -= 4*BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        rijndael256blockXOR(&ct[2], roundkeys, iv[1], &msg[2]);
        rijndael256blockXOR(&ct[4], roundkeys, iv[2], &msg[4]);
        rijndael256blockXOR(&ct[6], roundkeys, iv[3], &msg[6]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        _mm_storeu_si128(ct_ptr.v + 4, ct[4]);
        _mm_storeu_si128(ct_ptr.v + 5, ct[5]);
        _mm_storeu_si128(ct_ptr.v + 6, ct[6]);
        _mm_storeu_si128(ct_ptr.v + 7, ct[7]);
        iv[0][1] = _mm_add_epi32(iv[0][1], four.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], four.v);
        iv[2][1] = _mm_add_epi32(iv[2][1], four.v);
        iv[3][1] = _mm_add_epi32(iv[3][1], four.v);
        ct_ptr.v += 8;
    }
    if (to_enc >= 3*BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        m_ptr.v += 6;
        to_enc -= 3*BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        rijndael256blockXOR(&ct[2], roundkeys, iv[1], &msg[2]);
        rijndael256blockXOR(&ct[4], roundkeys, iv[2], &msg[4]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        _mm_storeu_si128(ct_ptr.v + 4, ct[4]);
        _mm_storeu_si128(ct_ptr.v + 5, ct[5]);
        iv[0][1] = _mm_add_epi32(iv[0][1], three.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], three.v);
        iv[1][1] = _mm_add_epi32(iv[2][1], three.v);
        ct_ptr.v += 6;
    }
    if (to_enc >= 2*BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        m_ptr.v += 4;
        to_enc -= 2*BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        rijndael256blockXOR(&ct[2], roundkeys, iv[1], &msg[2]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        iv[0][1] = _mm_add_epi32(iv[0][1], two.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], two.v);
        ct_ptr.v += 4;
    }
    if (to_enc >= BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        m_ptr.v += 2;
        to_enc -= BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.v += 2;
    }
    if (to_enc > 16) {
        msg[0] = _mm_loadu_si128(m_ptr.v);
        msg[1] = _mm_setzero_si128();
        memcpy(&msg[1], m_ptr.v + 1, to_enc - 16);
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        store_mask = _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)to_enc - 16),
                                    BYTE_IDX.v);
        ct[1] = _mm_and_si128(ct[1], store_mask);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        memcpy(ct_ptr.v + 1, &ct[1], to_enc - 16);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.c += to_enc;
        to_enc -= to_enc;
    } else if (to_enc > 0) {
        msg[0] = _mm_setzero_si128();
        memcpy(&msg[0], m_ptr.v, to_enc);
        msg[1] = _mm_setzero_si128();
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        store_mask =
            _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)to_enc), BYTE_IDX.v);
        ct[0] = _mm_and_si128(ct[0], store_mask);
        memcpy(ct_ptr.v + 0, &ct[0], to_enc);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.c += to_enc;
        to_enc -= to_enc;
    }
#ifndef __GNUC__
#pragma endregion ENCRYPTION
#endif
    return;
}

void rijndael256x8x_no_hash(
    size_t m_len, const uint8_t m[static m_len], size_t ad_len,
    const uint8_t ad[static ad_len], uint8_t c[static CTXT_LEN(m_len)],
    const uint8_t key[static KEY_SIZE], const uint8_t nonce[NONCE_SIZE])
{
    __m128i store_mask;

    vec_ptr m_ptr = {.v = (__m128i *)m};
    vec_ptr ct_ptr = {.v = (__m128i *)c};
    
    __m128i ct[16] = {0};
    __m128i msg[16] = {0};
    __m128i iv[8][2];
    size_t to_enc = m_len;
    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    iv[0][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5], ((uint32_t *)nonce)[6], 0}}).v;
    iv[1][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[1][1] = _mm_add_epi32(iv[0][1], one.v);
    iv[2][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[2][1] = _mm_add_epi32(iv[0][1], two.v);
    iv[3][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[3][1] = _mm_add_epi32(iv[0][1], three.v);
    iv[4][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[4][1] = _mm_add_epi32(iv[0][1], four.v);
    iv[5][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[5][1] = _mm_add_epi32(iv[0][1], five.v);
    iv[6][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[6][1] = _mm_add_epi32(iv[0][1], six.v);
    iv[7][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[7][1] = _mm_add_epi32(iv[0][1], seven.v);

#ifndef __GNUC__
#pragma region ENCRYPTION
#endif
    /* PROCESS M */
    while (to_enc >= 8*BLCKSIZE) {
        msg[ 0] = _mm_loadu_si128(m_ptr.v +  0);
        msg[ 1] = _mm_loadu_si128(m_ptr.v +  1);
        msg[ 2] = _mm_loadu_si128(m_ptr.v +  2);
        msg[ 3] = _mm_loadu_si128(m_ptr.v +  3);
        msg[ 4] = _mm_loadu_si128(m_ptr.v +  4);
        msg[ 5] = _mm_loadu_si128(m_ptr.v +  5);
        msg[ 6] = _mm_loadu_si128(m_ptr.v +  6);
        msg[ 7] = _mm_loadu_si128(m_ptr.v +  7);
        msg[ 8] = _mm_loadu_si128(m_ptr.v +  8);
        msg[ 9] = _mm_loadu_si128(m_ptr.v +  9);
        msg[10] = _mm_loadu_si128(m_ptr.v + 10);
        msg[11] = _mm_loadu_si128(m_ptr.v + 11);
        msg[12] = _mm_loadu_si128(m_ptr.v + 12);
        msg[13] = _mm_loadu_si128(m_ptr.v + 13);
        msg[14] = _mm_loadu_si128(m_ptr.v + 14);
        msg[15] = _mm_loadu_si128(m_ptr.v + 15);
        m_ptr.v += 16;
        to_enc -= 8*BLCKSIZE;
        rijndael256blockXORx8(&ct[ 0], roundkeys, (__m128i *)iv, &msg[ 0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[ 0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[ 1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[ 2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[ 3]);
        _mm_storeu_si128(ct_ptr.v + 4, ct[ 4]);
        _mm_storeu_si128(ct_ptr.v + 5, ct[ 5]);
        _mm_storeu_si128(ct_ptr.v + 6, ct[ 6]);
        _mm_storeu_si128(ct_ptr.v + 7, ct[ 7]);
        _mm_storeu_si128(ct_ptr.v + 8, ct[ 8]);
        _mm_storeu_si128(ct_ptr.v + 9, ct[ 9]);
        _mm_storeu_si128(ct_ptr.v +10, ct[10]);
        _mm_storeu_si128(ct_ptr.v +11, ct[11]);
        _mm_storeu_si128(ct_ptr.v +12, ct[12]);
        _mm_storeu_si128(ct_ptr.v +13, ct[13]);
        _mm_storeu_si128(ct_ptr.v +14, ct[14]);
        _mm_storeu_si128(ct_ptr.v +15, ct[15]);
        iv[0][1] = _mm_add_epi32(iv[0][1], six.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], six.v);
        iv[2][1] = _mm_add_epi32(iv[2][1], six.v);
        iv[3][1] = _mm_add_epi32(iv[3][1], six.v);
        iv[4][1] = _mm_add_epi32(iv[4][1], six.v);
        iv[5][1] = _mm_add_epi32(iv[5][1], six.v);
        iv[6][1] = _mm_add_epi32(iv[6][1], six.v);
        iv[7][1] = _mm_add_epi32(iv[7][1], six.v);
        ct_ptr.v += 16;
    }
    if (to_enc >= 6*BLCKSIZE) {
        msg[ 0] = _mm_loadu_si128(m_ptr.v +  0);
        msg[ 1] = _mm_loadu_si128(m_ptr.v +  1);
        msg[ 2] = _mm_loadu_si128(m_ptr.v +  2);
        msg[ 3] = _mm_loadu_si128(m_ptr.v +  3);
        msg[ 4] = _mm_loadu_si128(m_ptr.v +  4);
        msg[ 5] = _mm_loadu_si128(m_ptr.v +  5);
        msg[ 6] = _mm_loadu_si128(m_ptr.v +  6);
        msg[ 7] = _mm_loadu_si128(m_ptr.v +  7);
        msg[ 8] = _mm_loadu_si128(m_ptr.v +  8);
        msg[ 9] = _mm_loadu_si128(m_ptr.v +  9);
        msg[10] = _mm_loadu_si128(m_ptr.v + 10);
        msg[11] = _mm_loadu_si128(m_ptr.v + 11);
        m_ptr.v += 12;
        to_enc -= 6*BLCKSIZE;
        rijndael256blockXORx6(&ct[ 0], roundkeys, (__m128i *)iv, &msg[ 0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[ 0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[ 1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[ 2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[ 3]);
        _mm_storeu_si128(ct_ptr.v + 4, ct[ 4]);
        _mm_storeu_si128(ct_ptr.v + 5, ct[ 5]);
        _mm_storeu_si128(ct_ptr.v + 6, ct[ 6]);
        _mm_storeu_si128(ct_ptr.v + 7, ct[ 7]);
        _mm_storeu_si128(ct_ptr.v + 8, ct[ 8]);
        _mm_storeu_si128(ct_ptr.v + 9, ct[ 9]);
        _mm_storeu_si128(ct_ptr.v +10, ct[10]);
        _mm_storeu_si128(ct_ptr.v +11, ct[11]);
        iv[0][1] = _mm_add_epi32(iv[0][1], six.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], six.v);
        iv[2][1] = _mm_add_epi32(iv[2][1], six.v);
        iv[3][1] = _mm_add_epi32(iv[3][1], six.v);
        iv[4][1] = _mm_add_epi32(iv[4][1], six.v);
        iv[5][1] = _mm_add_epi32(iv[5][1], six.v);
        ct_ptr.v += 12;
    }
    if (to_enc >= 4*BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        msg[6] = _mm_loadu_si128(m_ptr.v + 6);
        msg[7] = _mm_loadu_si128(m_ptr.v + 7);
        m_ptr.v += 8;
        to_enc -= 4*BLCKSIZE;
        rijndael256blockXORx4(&ct[0], roundkeys, (__m128i *)iv, &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        _mm_storeu_si128(ct_ptr.v + 4, ct[4]);
        _mm_storeu_si128(ct_ptr.v + 5, ct[5]);
        _mm_storeu_si128(ct_ptr.v + 6, ct[6]);
        _mm_storeu_si128(ct_ptr.v + 7, ct[7]);
        iv[0][1] = _mm_add_epi32(iv[0][1], four.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], four.v);
        iv[2][1] = _mm_add_epi32(iv[2][1], four.v);
        iv[3][1] = _mm_add_epi32(iv[3][1], four.v);
        ct_ptr.v += 8;
    }
    if (to_enc >= 3*BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        m_ptr.v += 6;
        to_enc -= 3*BLCKSIZE;
        rijndael256blockXORx3(&ct[0], roundkeys, (__m128i *)iv, &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        _mm_storeu_si128(ct_ptr.v + 4, ct[4]);
        _mm_storeu_si128(ct_ptr.v + 5, ct[5]);
        iv[0][1] = _mm_add_epi32(iv[0][1], three.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], three.v);
        iv[1][1] = _mm_add_epi32(iv[2][1], three.v);
        ct_ptr.v += 6;
    }
    if (to_enc >= 2*BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        m_ptr.v += 4;
        to_enc -= 2*BLCKSIZE;
        rijndael256blockXORx2(&ct[0], roundkeys, (__m128i *)iv, &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        iv[0][1] = _mm_add_epi32(iv[0][1], two.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], two.v);
        ct_ptr.v += 4;
    }
    if (to_enc >= BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        m_ptr.v += 2;
        to_enc -= BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.v += 2;
    }
    if (to_enc > 16) {
        msg[0] = _mm_loadu_si128(m_ptr.v);
        msg[1] = _mm_setzero_si128();
        memcpy(&msg[1], m_ptr.v + 1, to_enc - 16);
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        store_mask = _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)to_enc - 16),
                                    BYTE_IDX.v);
        ct[1] = _mm_and_si128(ct[1], store_mask);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        memcpy(ct_ptr.v + 1, &ct[1], to_enc - 16);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.c += to_enc;
        to_enc -= to_enc;
    } else if (to_enc > 0) {
        msg[0] = _mm_setzero_si128();
        memcpy(&msg[0], m_ptr.v, to_enc);
        msg[1] = _mm_setzero_si128();
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        store_mask =
            _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)to_enc), BYTE_IDX.v);
        ct[0] = _mm_and_si128(ct[0], store_mask);
        memcpy(ct_ptr.v + 0, &ct[0], to_enc);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        ct_ptr.c += to_enc;
        to_enc -= to_enc;
    }
#ifndef __GNUC__
#pragma endregion ENCRYPTION
#endif
    return;
}
