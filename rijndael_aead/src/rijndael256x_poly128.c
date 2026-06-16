#include "rijndael256/rijndael256x_poly128.h"
#include "fieldArith/bf_arithmetic_128_64_64_64_schoolbook.h"
#include "fieldArith/fieldArith.h"
#include "rijndael256/rijndael256.h"
#include "rijndael256_impl.h"
#include <immintrin.h>
#include <inttypes.h>
#include <stdio.h>

const vector BYTE_IDX = {
    .i8 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}};

typedef struct {
    field_elem_t hash;
    field_elem_t unused;
    field_elem_t wc;
    field_elem_t unused2;
} keys_t;

void rijndael256x_poly128_corex2(size_t m_len, const uint8_t m[static m_len],
                                 size_t ad_len, const uint8_t ad[static ad_len],
                                 uint8_t c[static CTXT_LEN(m_len)],
                                 const uint8_t key[static KEY_SIZE],
                                 const uint8_t nonce[NONCE_SIZE])
{
    __m128i *m_ptr = (__m128i *)m;
    __m128i *ct_ptr = (__m128i *)c;
    __m128i *ad_ptr = (__m128i *)ad;
    keys_t keys = {0};
    __m128i input[2] = {0};
    field_elem_t hash_key[4] = {0};
    field_elem_t acc = {0};
    field_elem_t len = {0}; // stores the length of the hash input (including
                            // AEAD length encoding)
    field_elem_t len_blck;  // stores the AEAD length encoding

    __m128i store_mask;

    field_elem_t ct[4] = {0};
    field_elem_t msg[4] = {0};
    dfield_elem_t dacc[4] = {0};
    __m128i iv[2][2];

    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    size_t int_len = ad_len + PAD_LEN(ad_len) + m_len + PAD_LEN(m_len) +
                     2 * sizeof(uint64_t);
    memcpy(&len, &int_len, sizeof(size_t));
    len_blck.val[0] = _mm_set_epi64x((uint64_t)m_len, (uint64_t)ad_len);

    iv[0][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[1][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5],
                                 ((uint32_t *)nonce)[6], 0}})
                   .v;
    iv[1][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5],
                                 ((uint32_t *)nonce)[6], 0}})
                   .v;
    iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
    iv[1][1] = _mm_add_epi32(iv[1][1], three.v);
    rijndael256block(&((__m128i *)&keys)[2], roundkeys, iv[0]);
    iv[0][1] = _mm_add_epi32(iv[0][1], one.v);

    rijndael256block(((__m128i *)&keys), roundkeys, input);

    hash_key[0] = keys.hash;
    field_mul(&hash_key[1], &hash_key[0], &hash_key[0]);
    field_mul(&hash_key[2], &hash_key[1], &hash_key[0]);
    field_mul(&hash_key[3], &hash_key[2], &hash_key[0]);
#ifndef __GNUC__
#pragma region AD
#endif
    while (ad_len >= (4 * HASHBLCK_SIZE)) {
        msg[0].val[0] = _mm_loadu_si128(ad_ptr + 0);
        msg[1].val[0] = _mm_loadu_si128(ad_ptr + 1);
        msg[2].val[0] = _mm_loadu_si128(ad_ptr + 2);
        msg[3].val[0] = _mm_loadu_si128(ad_ptr + 3);
        ad_ptr += 4;
        ad_len -= (4 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &msg[0], &hash_key[3]);
        field_mul_no_carry(&dacc[1], &msg[1], &hash_key[2]);
        field_mul_no_carry(&dacc[2], &msg[2], &hash_key[1]);
        field_mul_no_carry(&dacc[3], &msg[3], &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_add_dbl(&dacc[2], &dacc[3], &dacc[2]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[2]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[3]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len >= (3 * HASHBLCK_SIZE)) {
        msg[0].val[0] = _mm_loadu_si128(ad_ptr + 0);
        msg[1].val[0] = _mm_loadu_si128(ad_ptr + 1);
        msg[2].val[0] = _mm_loadu_si128(ad_ptr + 2);
        ad_ptr += 3;
        ad_len -= (3 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &msg[0], &hash_key[2]);
        field_mul_no_carry(&dacc[1], &msg[1], &hash_key[1]);
        field_mul_no_carry(&dacc[2], &msg[2], &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[2]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[2]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len >= (2 * HASHBLCK_SIZE)) {
        msg[0].val[0] = _mm_loadu_si128(ad_ptr + 0);
        msg[1].val[0] = _mm_loadu_si128(ad_ptr + 1);
        ad_ptr += 2;
        ad_len -= (2 * HASHBLCK_SIZE);
        field_mul_no_carry(&dacc[0], &msg[0], &hash_key[1]);
        field_mul_no_carry(&dacc[1], &msg[1], &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[1]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len >= HASHBLCK_SIZE) {
        msg[0].val[0] = _mm_loadu_si128(ad_ptr + 0);
        ad_ptr += 1;
        ad_len -= HASHBLCK_SIZE;
        field_add(&acc, &acc, &msg[0]);
        field_mul(&acc, &acc, &hash_key[0]);
    }
    if (ad_len > 0) {
        msg[0].val[0] = _mm_setzero_si128();
        memcpy(msg[0].val, ad_ptr, ad_len);
        ad_len = 0;
        field_add(&acc, &acc, &msg[0]);
        field_mul(&acc, &acc, &hash_key[0]);
    }
#ifndef __GNUC__
#pragma endregion AD
#endif

#ifndef __GNUC__
#pragma region ENCRYPTION
#endif
    while (m_len >= (2 * BLCKSIZE)) {
        msg[0].val[0] = _mm_loadu_si128(m_ptr + 0);
        msg[1].val[0] = _mm_loadu_si128(m_ptr + 1);
        msg[2].val[0] = _mm_loadu_si128(m_ptr + 2);
        msg[3].val[0] = _mm_loadu_si128(m_ptr + 3);
        m_ptr += 4;
        m_len -= (2 * BLCKSIZE);
        rijndael256blockXORx2((__m128i *)&ct[0], roundkeys, (__m128i *)iv, (__m128i *)&msg[0]);
        _mm_storeu_si128(ct_ptr + 0, ct[0].val[0]);
        _mm_storeu_si128(ct_ptr + 1, ct[1].val[0]);
        _mm_storeu_si128(ct_ptr + 2, ct[2].val[0]);
        _mm_storeu_si128(ct_ptr + 3, ct[3].val[0]);
        ct_ptr += 4;

        field_mul_no_carry(&dacc[0], &ct[0], &hash_key[3]);
        field_mul_no_carry(&dacc[1], &ct[1], &hash_key[2]);
        field_mul_no_carry(&dacc[2], &ct[2], &hash_key[1]);
        field_mul_no_carry(&dacc[3], &ct[3], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_add_dbl(&dacc[2], &dacc[3], &dacc[2]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[2]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[3]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);

        carry_round(&acc, &dacc[0]);
        iv[0][1] = _mm_add_epi32(iv[0][1], two.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], two.v);
    }
    if (m_len >= BLCKSIZE) {
        msg[0].val[0] = _mm_loadu_si128(m_ptr + 0);
        msg[1].val[0] = _mm_loadu_si128(m_ptr + 1);
        m_ptr += 2;
        m_len -= BLCKSIZE;
        rijndael256blockXOR((__m128i *)ct, roundkeys, iv[0], (__m128i *)msg);
        field_mul_no_carry(&dacc[0], &ct[0], &hash_key[1]);
        field_mul_no_carry(&dacc[1], &ct[1], &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[1]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
        _mm_storeu_si128(ct_ptr + 0, ct[0].val[0]);
        _mm_storeu_si128(ct_ptr + 1, ct[1].val[0]);
        ct_ptr += 2;
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
    }
    if (m_len > 16) {
        msg[0].val[0] = _mm_loadu_si128(m_ptr + 0);
        msg[1].val[0] = _mm_setzero_si128();
        memcpy(msg[1].val, m_ptr + 1, m_len - 16);
        rijndael256blockXOR((__m128i *)ct, roundkeys, iv[0], (__m128i *)msg);
        store_mask = _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)m_len - 16),
                                    BYTE_IDX.v);
        ct[1].val[0] = _mm_and_si128(ct[1].val[0], store_mask);
        field_mul_no_carry(&dacc[0], &ct[0], &hash_key[1]);
        field_mul_no_carry(&dacc[1], &ct[1], &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[1]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
        _mm_storeu_si128(ct_ptr + 0, ct[0].val[0]);
        _mm_storeu_si128(ct_ptr + 1, ct[1].val[0]);
        ct_ptr += 2;
        m_len = 0;
    } else if (m_len > 0) {
        msg[0].val[0] = _mm_setzero_si128();
        msg[1].val[0] = _mm_setzero_si128();
        memcpy(msg[0].val, m_ptr, m_len);
        rijndael256blockXOR((__m128i *)ct, roundkeys, iv[0], (__m128i *)msg);
        store_mask =
            _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)m_len), BYTE_IDX.v);
        ct[0].val[0] = _mm_and_si128(ct[0].val[0], store_mask);
        _mm_storeu_si128(ct_ptr + 0, ct[0].val[0]);
        ct_ptr += 1;
        m_len = 0;
        field_add(&acc, &acc, &ct[0]);
        field_mul(&acc, &acc, &keys.hash);
    }
#ifndef __GNUC__
#pragma endregion ENCRYPTION
#endif
    field_add(&acc, &acc, &len_blck);
    field_mul(&acc, &acc, &keys.hash);
    field_add(&acc, &acc, &len);
    field_mul(&acc, &acc, &keys.hash);
    field_add(&acc, &acc, &keys.wc);
    _mm_storeu_si128(ct_ptr + PADDED_LEN(m_len), acc.val[0]);
    return;
}

void rijndael256x_poly128_corex4(size_t m_len, const uint8_t m[static m_len],
                                 size_t ad_len, const uint8_t ad[static ad_len],
                                 uint8_t c[static CTXT_LEN(m_len)],
                                 const uint8_t key[static KEY_SIZE],
                                 const uint8_t nonce[NONCE_SIZE])
{
    __m128i *m_ptr = (__m128i *)m;
    __m128i *ct_ptr = (__m128i *)c;
    __m128i *ad_ptr = (__m128i *)ad;
    keys_t keys = {0};
    __m128i input[2] = {0};
    field_elem_t hash_key[8] = {0};
    field_elem_t acc = {0};
    field_elem_t len = {0}; // stores the length of the hash input (including
                            // AEAD length encoding)
    field_elem_t len_blck;  // stores the AEAD length encoding

    __m128i store_mask;

    field_elem_t ct[8] = {0};
    field_elem_t msg[8] = {0};
    dfield_elem_t dacc[8] = {0};
    __m128i iv[4][2];

    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    size_t int_len = ad_len + PAD_LEN(ad_len) + m_len + PAD_LEN(m_len) +
                     2 * sizeof(uint64_t);
    memcpy(&len, &int_len, sizeof(size_t));
    len_blck.val[0] = _mm_set_epi64x((uint64_t)m_len, (uint64_t)ad_len);

    iv[0][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[1][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[2][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[3][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5],
                                 ((uint32_t *)nonce)[6], 0}})
                   .v;
    iv[1][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5],
                                 ((uint32_t *)nonce)[6], 0}})
                   .v;
    iv[2][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5],
                                 ((uint32_t *)nonce)[6], 0}})
                   .v;
    iv[3][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5],
                                 ((uint32_t *)nonce)[6], 0}})
                   .v;
    iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
    iv[1][1] = _mm_add_epi32(iv[1][1], three.v);
    iv[2][1] = _mm_add_epi32(iv[2][1], four.v);
    iv[3][1] = _mm_add_epi32(iv[3][1], five.v);
    rijndael256block(&((__m128i *)&keys)[2], roundkeys, iv[0]);
    iv[0][1] = _mm_add_epi32(iv[0][1], one.v);

    rijndael256block(((__m128i *)&keys), roundkeys, input);

    hash_key[0] = keys.hash;
    field_mul(&hash_key[1], &hash_key[0], &hash_key[0]);
    field_mul(&hash_key[2], &hash_key[1], &hash_key[0]);
    field_mul(&hash_key[3], &hash_key[2], &hash_key[0]);
    field_mul(&hash_key[4], &hash_key[3], &hash_key[0]);
    field_mul(&hash_key[5], &hash_key[4], &hash_key[0]);
    field_mul(&hash_key[6], &hash_key[5], &hash_key[0]);
    field_mul(&hash_key[7], &hash_key[6], &hash_key[0]);
#ifndef __GNUC__
#pragma region AD
#endif
    while (ad_len >= (8 * HASHBLCK_SIZE)) {
        msg[0].val[0] = _mm_loadu_si128(ad_ptr + 0);
        msg[1].val[0] = _mm_loadu_si128(ad_ptr + 1);
        msg[2].val[0] = _mm_loadu_si128(ad_ptr + 2);
        msg[3].val[0] = _mm_loadu_si128(ad_ptr + 3);
        msg[4].val[0] = _mm_loadu_si128(ad_ptr + 4);
        msg[5].val[0] = _mm_loadu_si128(ad_ptr + 5);
        msg[6].val[0] = _mm_loadu_si128(ad_ptr + 6);
        msg[7].val[0] = _mm_loadu_si128(ad_ptr + 7);
        ad_ptr += 8;
        ad_len -= (8 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &msg[0], &hash_key[7]);
        field_mul_no_carry(&dacc[1], &msg[1], &hash_key[6]);
        field_mul_no_carry(&dacc[2], &msg[2], &hash_key[5]);
        field_mul_no_carry(&dacc[3], &msg[3], &hash_key[4]);
        field_mul_no_carry(&dacc[4], &msg[4], &hash_key[3]);
        field_mul_no_carry(&dacc[5], &msg[5], &hash_key[2]);
        field_mul_no_carry(&dacc[6], &msg[6], &hash_key[1]);
        field_mul_no_carry(&dacc[7], &msg[7], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_add_dbl(&dacc[2], &dacc[2], &dacc[3]);
        field_add_dbl(&dacc[4], &dacc[4], &dacc[5]);
        field_add_dbl(&dacc[6], &dacc[6], &dacc[7]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[2]);
        field_add_dbl(&dacc[4], &dacc[4], &dacc[6]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[4]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[7]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);

        carry_round(&acc, &dacc[0]);
    }
    if (ad_len >= (4 * HASHBLCK_SIZE)) {
        msg[0].val[0] = _mm_loadu_si128(ad_ptr + 0);
        msg[1].val[0] = _mm_loadu_si128(ad_ptr + 1);
        msg[2].val[0] = _mm_loadu_si128(ad_ptr + 2);
        msg[3].val[0] = _mm_loadu_si128(ad_ptr + 3);
        ad_ptr += 4;
        ad_len -= (4 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &msg[0], &hash_key[3]);
        field_mul_no_carry(&dacc[1], &msg[1], &hash_key[2]);
        field_mul_no_carry(&dacc[2], &msg[2], &hash_key[1]);
        field_mul_no_carry(&dacc[3], &msg[3], &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_add_dbl(&dacc[2], &dacc[3], &dacc[2]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[2]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[3]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len >= (3 * HASHBLCK_SIZE)) {
        msg[0].val[0] = _mm_loadu_si128(ad_ptr + 0);
        msg[1].val[0] = _mm_loadu_si128(ad_ptr + 1);
        msg[2].val[0] = _mm_loadu_si128(ad_ptr + 2);
        ad_ptr += 3;
        ad_len -= (3 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &msg[0], &hash_key[2]);
        field_mul_no_carry(&dacc[1], &msg[1], &hash_key[1]);
        field_mul_no_carry(&dacc[2], &msg[2], &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[2]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[2]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len >= (2 * HASHBLCK_SIZE)) {
        msg[0].val[0] = _mm_loadu_si128(ad_ptr + 0);
        msg[1].val[0] = _mm_loadu_si128(ad_ptr + 1);
        ad_ptr += 2;
        ad_len -= (2 * HASHBLCK_SIZE);
        field_mul_no_carry(&dacc[0], &msg[0], &hash_key[1]);
        field_mul_no_carry(&dacc[1], &msg[1], &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[1]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len >= HASHBLCK_SIZE) {
        msg[0].val[0] = _mm_loadu_si128(ad_ptr + 0);
        ad_ptr += 1;
        ad_len -= HASHBLCK_SIZE;
        field_add(&acc, &acc, &msg[0]);
        field_mul(&acc, &acc, &hash_key[0]);
    }
    if (ad_len > 0) {
        msg[0].val[0] = _mm_setzero_si128();
        memcpy(msg[0].val, ad_ptr, ad_len);
        ad_len = 0;
        field_add(&acc, &acc, &msg[0]);
        field_mul(&acc, &acc, &hash_key[0]);
    }
#ifndef __GNUC__
#pragma endregion AD
#endif

#ifndef __GNUC__
#pragma region ENCRYPTION
#endif
    while (m_len >= (8 * BLCKSIZE)) {
        msg[0].val[0] = _mm_loadu_si128(m_ptr + 0);
        msg[1].val[0] = _mm_loadu_si128(m_ptr + 1);
        msg[2].val[0] = _mm_loadu_si128(m_ptr + 2);
        msg[3].val[0] = _mm_loadu_si128(m_ptr + 3);
        msg[4].val[0] = _mm_loadu_si128(m_ptr + 4);
        msg[5].val[0] = _mm_loadu_si128(m_ptr + 5);
        msg[6].val[0] = _mm_loadu_si128(m_ptr + 6);
        msg[7].val[0] = _mm_loadu_si128(m_ptr + 7);
        m_ptr += 8;
        m_len -= (4 * BLCKSIZE);
        rijndael256blockXORx4((__m128i *)&ct[0], roundkeys, (__m128i *)iv, (__m128i *)&msg[0]);

        _mm_storeu_si128(ct_ptr + 0, ct[0].val[0]);
        _mm_storeu_si128(ct_ptr + 1, ct[1].val[0]);
        _mm_storeu_si128(ct_ptr + 2, ct[2].val[0]);
        _mm_storeu_si128(ct_ptr + 3, ct[3].val[0]);
        _mm_storeu_si128(ct_ptr + 4, ct[4].val[0]);
        _mm_storeu_si128(ct_ptr + 5, ct[5].val[0]);
        _mm_storeu_si128(ct_ptr + 6, ct[6].val[0]);
        _mm_storeu_si128(ct_ptr + 7, ct[7].val[0]);
        ct_ptr += 8;

        field_mul_no_carry(&dacc[0], &ct[0], &hash_key[7]);
        field_mul_no_carry(&dacc[1], &ct[1], &hash_key[6]);
        field_mul_no_carry(&dacc[2], &ct[2], &hash_key[5]);
        field_mul_no_carry(&dacc[3], &ct[3], &hash_key[4]);
        field_mul_no_carry(&dacc[4], &ct[4], &hash_key[3]);
        field_mul_no_carry(&dacc[5], &ct[5], &hash_key[2]);
        field_mul_no_carry(&dacc[6], &ct[6], &hash_key[1]);
        field_mul_no_carry(&dacc[7], &ct[7], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_add_dbl(&dacc[2], &dacc[2], &dacc[3]);
        field_add_dbl(&dacc[4], &dacc[4], &dacc[5]);
        field_add_dbl(&dacc[6], &dacc[6], &dacc[7]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[2]);
        field_add_dbl(&dacc[4], &dacc[4], &dacc[6]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[4]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[7]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);

        carry_round(&acc, &dacc[0]);
        iv[0][1] = _mm_add_epi32(iv[0][1], four.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], four.v);
        iv[2][1] = _mm_add_epi32(iv[2][1], four.v);
        iv[3][1] = _mm_add_epi32(iv[3][1], four.v);
    }
    while (m_len >= (2 * BLCKSIZE)) {
        msg[0].val[0] = _mm_loadu_si128(m_ptr + 0);
        msg[1].val[0] = _mm_loadu_si128(m_ptr + 1);
        msg[2].val[0] = _mm_loadu_si128(m_ptr + 2);
        msg[3].val[0] = _mm_loadu_si128(m_ptr + 3);
        m_ptr += 4;
        m_len -= (2 * BLCKSIZE);
        rijndael256blockXORx2((__m128i *)&ct[0], roundkeys, (__m128i *)iv, (__m128i *)&msg[0]);

        _mm_storeu_si128(ct_ptr + 0, ct[0].val[0]);
        _mm_storeu_si128(ct_ptr + 1, ct[1].val[0]);
        _mm_storeu_si128(ct_ptr + 2, ct[2].val[0]);
        _mm_storeu_si128(ct_ptr + 3, ct[3].val[0]);
        ct_ptr += 4;

        field_mul_no_carry(&dacc[0], &ct[0], &hash_key[3]);
        field_mul_no_carry(&dacc[1], &ct[1], &hash_key[2]);
        field_mul_no_carry(&dacc[2], &ct[2], &hash_key[1]);
        field_mul_no_carry(&dacc[3], &ct[3], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_add_dbl(&dacc[2], &dacc[3], &dacc[2]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[2]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[3]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);

        carry_round(&acc, &dacc[0]);
        iv[0][1] = _mm_add_epi32(iv[0][1], two.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], two.v);
    }
    if (m_len >= BLCKSIZE) {
        msg[0].val[0] = _mm_loadu_si128(m_ptr + 0);
        msg[1].val[0] = _mm_loadu_si128(m_ptr + 1);
        m_ptr += 2;
        m_len -= BLCKSIZE;
        rijndael256blockXOR((__m128i *)ct, roundkeys, iv[0], (__m128i *)msg);
        field_mul_no_carry(&dacc[0], &ct[0], &hash_key[1]);
        field_mul_no_carry(&dacc[1], &ct[1], &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[1]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
        _mm_storeu_si128(ct_ptr + 0, ct[0].val[0]);
        _mm_storeu_si128(ct_ptr + 1, ct[1].val[0]);
        ct_ptr += 2;
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
    }
    if (m_len > 16) {
        msg[0].val[0] = _mm_loadu_si128(m_ptr + 0);
        msg[1].val[0] = _mm_setzero_si128();
        memcpy(msg[1].val, m_ptr + 1, m_len - 16);
        rijndael256blockXOR((__m128i *)ct, roundkeys, iv[0], (__m128i *)msg);
        store_mask = _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)m_len - 16),
                                    BYTE_IDX.v);
        ct[1].val[0] = _mm_and_si128(ct[1].val[0], store_mask);
        field_mul_no_carry(&dacc[0], &ct[0], &hash_key[1]);
        field_mul_no_carry(&dacc[1], &ct[1], &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[1]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
        _mm_storeu_si128(ct_ptr + 0, ct[0].val[0]);
        _mm_storeu_si128(ct_ptr + 1, ct[1].val[0]);
        ct_ptr += 2;
        m_len = 0;
    } else if (m_len > 0) {
        msg[0].val[0] = _mm_setzero_si128();
        msg[1].val[0] = _mm_setzero_si128();
        memcpy(msg[0].val, m_ptr, m_len);
        rijndael256blockXOR((__m128i *)ct, roundkeys, iv[0], (__m128i *)msg);
        store_mask =
            _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)m_len), BYTE_IDX.v);
        ct[0].val[0] = _mm_and_si128(ct[0].val[0], store_mask);
        _mm_storeu_si128(ct_ptr + 0, ct[0].val[0]);
        ct_ptr += 1;
        m_len = 0;
        field_add(&acc, &acc, &ct[0]);
        field_mul(&acc, &acc, &keys.hash);
    }
#ifndef __GNUC__
#pragma endregion ENCRYPTION
#endif
    field_add(&acc, &acc, &len_blck);
    field_mul(&acc, &acc, &keys.hash);
    field_add(&acc, &acc, &len);
    field_mul(&acc, &acc, &keys.hash);
    field_add(&acc, &acc, &keys.wc);
    _mm_storeu_si128(ct_ptr + PADDED_LEN(m_len), acc.val[0]);
    return;
}
