#include "rijndael256/rijndael256x_poly256.h"
#include "fieldArith/bf_arithmetic_256_64_64_64_64_64_schoolbook.h"
#include "fieldArith/fieldArith.h"
#include "rijndael256/rijndael256.h"
#include "rijndael256_impl.h"
#include <immintrin.h>
#include <inttypes.h>
#include <stdio.h>

const vector BYTE_IDX = {
    .i8 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}};

void rijndael256x_poly256_corex2(size_t m_len, const uint8_t m[static m_len],
                                 size_t ad_len, const uint8_t ad[static ad_len],
                                 uint8_t c[static CTXT_LEN(m_len)],
                                 const uint8_t key[static KEY_SIZE],
                                 const uint8_t nonce[NONCE_SIZE])
{
    __m128i *m_ptr = (__m128i *)m;
    __m128i *ct_ptr = (__m128i *)c;
    __m128i *ad_ptr = (__m128i *)ad;
    field_elem_t wc_key = {0};
    __m128i input[2] = {0};
    field_elem_t len_blck = {0}; // stores the AEAD length encoding
    field_elem_t len = {0}; // stores the length of the hash input (including
                            // AEAD length encoding)

    __m128i store_mask;

    field_elem_t hash_key[2] = {0};
    field_elem_t acc = {0};

    field_elem_t ct[2] = {0};
    field_elem_t msg[2] = {0};
    dfield_elem_t dacc1 = {0};
    __m128i iv[2][2];

    dfield_elem_t dacc2 = {0};

    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    size_t int_len = ad_len + PAD_LEN(ad_len) + m_len + PAD_LEN(m_len) +
                     2 * sizeof(uint64_t);
    memcpy(&len, &int_len, sizeof(size_t));
    len_blck.val[0] = _mm_set_epi64x((uint64_t)m_len, (uint64_t)ad_len);
    len_blck.val[1] = _mm_setzero_si128();

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
    rijndael256block(hash_key[0].val, roundkeys, input);
    rijndael256block(wc_key.val, roundkeys, iv[0]);
    iv[0][1] = _mm_add_epi32(iv[0][1], one.v);

    field_mul(&hash_key[1], &hash_key[0], &hash_key[0]);
#ifndef __GNUC__
#pragma region AD
#endif
    while (ad_len >= (2 * BLCKSIZE)) {
        msg[0].val[0] = _mm_loadu_si128(ad_ptr + 0 + 0);
        msg[0].val[1] = _mm_loadu_si128(ad_ptr + 0 + 1);
        msg[1].val[0] = _mm_loadu_si128(ad_ptr + 2 + 0);
        msg[1].val[1] = _mm_loadu_si128(ad_ptr + 2 + 1);
        ad_ptr += 4;
        ad_len -= (2 * BLCKSIZE);
        field_mul_no_carry(&dacc1, &msg[0], &hash_key[1]);
        field_mul_no_carry(&dacc2, &msg[1], &hash_key[0]);
        field_add_dbl(&dacc1, &dacc1, &dacc2);
        field_mul_no_carry(&dacc2, &acc, &hash_key[1]);
        field_add_dbl(&dacc1, &dacc1, &dacc2);
        carry_round(&acc, &dacc1);
    }
    if (ad_len >= BLCKSIZE) {
        msg[0].val[0] = _mm_loadu_si128(ad_ptr + 0);
        msg[0].val[1] = _mm_loadu_si128(ad_ptr + 1);
        ad_ptr += 2;
        ad_len -= BLCKSIZE;
        field_add(&acc, &acc, &msg[0]);
        field_mul(&acc, &acc, &hash_key[0]);
    }
    if (ad_len > 0) {
        msg[0].val[0] = _mm_setzero_si128();
        msg[0].val[1] = _mm_setzero_si128();
        memcpy(msg[0].val, ad_ptr, ad_len);
        ad_len = 0;
        field_add(&acc, &acc, &msg[0]);
        field_mul(&acc, &acc, &hash_key[0]);
    }
#ifndef __GNUC__
#pragma endregion AD
#endif

#ifndef __GNUC__
#pragma region Encryption
#endif
    while (m_len >= (2 * BLCKSIZE)) {
        msg[0].val[0] = _mm_loadu_si128(m_ptr + 0 + 0);
        msg[0].val[1] = _mm_loadu_si128(m_ptr + 0 + 1);
        msg[1].val[0] = _mm_loadu_si128(m_ptr + 2 + 0);
        msg[1].val[1] = _mm_loadu_si128(m_ptr + 2 + 1);
        m_ptr += 4;
        m_len -= (2 * BLCKSIZE);
        rijndael256blockXORx2((__m128i *)(&ct[0]), roundkeys, (__m128i *)iv,
                              (__m128i *)(&msg[0]));
        _mm_storeu_si128(ct_ptr + 0 + 0, ct[0].val[0]);
        _mm_storeu_si128(ct_ptr + 0 + 1, ct[0].val[1]);
        _mm_storeu_si128(ct_ptr + 2 + 0, ct[1].val[0]);
        _mm_storeu_si128(ct_ptr + 2 + 1, ct[1].val[1]);

        field_mul_no_carry(&dacc1, &ct[0], &hash_key[1]);
        field_mul_no_carry(&dacc2, &ct[1], &hash_key[0]);
        field_add_dbl(&dacc1, &dacc1, &dacc2);
        field_mul_no_carry(&dacc2, &acc, &hash_key[1]);
        field_add_dbl(&dacc1, &dacc1, &dacc2);
        carry_round(&acc, &dacc1);
        ct_ptr += 4;
        iv[0][1] = _mm_add_epi32(iv[0][1], two.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], two.v);
    }
    if (m_len >= BLCKSIZE) {
        msg[0].val[0] = _mm_loadu_si128(m_ptr + 0);
        msg[0].val[1] = _mm_loadu_si128(m_ptr + 1);
        m_ptr += 2;
        m_len -= BLCKSIZE;
        rijndael256blockXOR(ct[0].val, roundkeys, iv[0], msg[0].val);
        field_add(&acc, &acc, &ct[0]);
        field_mul(&acc, &acc, &hash_key[0]);
        _mm_storeu_si128(ct_ptr + 0, ct[0].val[0]);
        _mm_storeu_si128(ct_ptr + 1, ct[0].val[1]);
        ct_ptr += 2;
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
    }
    if (m_len > 0) {
        msg[0].val[0] = _mm_setzero_si128();
        msg[0].val[1] = _mm_setzero_si128();
        memcpy(msg[0].val, m_ptr, m_len);
        rijndael256blockXOR(ct[0].val, roundkeys, iv[0], msg[0].val);
        if (m_len > 16) {
            store_mask = _mm_cmpgt_epi8(
                _mm_set1_epi8((unsigned char)m_len - 16), BYTE_IDX.v);
            ct[0].val[1] = _mm_and_si128(ct[0].val[1], store_mask);
        } else {
            store_mask =
                _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)m_len), BYTE_IDX.v);
            ct[0].val[0] = _mm_and_si128(ct[0].val[0], store_mask);
        }
        _mm_storeu_si128(ct_ptr + 0, ct[0].val[0]);
        _mm_storeu_si128(ct_ptr + 1, ct[0].val[1]);
        ct_ptr += 2;
        m_len = 0;
        field_add(&acc, &acc, &ct[0]);
        field_mul(&acc, &acc, &hash_key[0]);
    }
#ifndef __GNUC__
#pragma endregion Encryption
#endif
    field_add(&acc, &acc, &len_blck);
    field_mul(&acc, &acc, &hash_key[0]);
    field_add(&acc, &acc, &len);
    field_mul(&acc, &acc, &hash_key[0]);
    field_add(&acc, &acc, &wc_key);
    _mm_storeu_si128(ct_ptr + 0, acc.val[0]);
    _mm_storeu_si128(ct_ptr + 1, acc.val[1]);
    return;
}

void rijndael256x_poly256_corex3(size_t m_len, const uint8_t m[static m_len],
                                 size_t ad_len, const uint8_t ad[static ad_len],
                                 uint8_t c[static CTXT_LEN(m_len)],
                                 const uint8_t key[static KEY_SIZE],
                                 const uint8_t nonce[NONCE_SIZE])
{
    __m128i *m_ptr = (__m128i *)m;
    __m128i *ct_ptr = (__m128i *)c;
    __m128i *ad_ptr = (__m128i *)ad;
    field_elem_t wc_key = {0};
    __m128i input[2] = {0};
    field_elem_t len_blck = {0}; // stores the AEAD length encoding
    field_elem_t len = {0}; // stores the length of the hash input (including
                            // AEAD length encoding)

    __m128i store_mask;

    field_elem_t acc = {0};
    field_elem_t hash_key[3] = {0};

    field_elem_t ct[3] = {0};
    field_elem_t msg[3] = {0};
    dfield_elem_t dacc1 = {0};
    __m128i iv[3][2];

    dfield_elem_t dacc2 = {0};

    dfield_elem_t dacc3 = {0};

    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);
    size_t int_len = ad_len + PAD_LEN(ad_len) + m_len + PAD_LEN(m_len) +
                     2 * sizeof(uint64_t);
    memcpy(&len, &int_len, sizeof(size_t));

    len_blck.val[0] = _mm_set_epi64x((uint64_t)m_len, (uint64_t)ad_len);
    len_blck.val[1] = _mm_setzero_si128();

    iv[0][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[1][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[2][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5],
                                 ((uint32_t *)nonce)[6], 0}})
                   .v;
    iv[1][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5],
                                 ((uint32_t *)nonce)[6], 0}})
                   .v;
    iv[2][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5],
                                 ((uint32_t *)nonce)[6], 0}})
                   .v;
    iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
    iv[1][1] = _mm_add_epi32(iv[1][1], three.v);
    iv[2][1] = _mm_add_epi32(iv[2][1], four.v);
    rijndael256block(hash_key[0].val, roundkeys, input);
    rijndael256block(wc_key.val, roundkeys, iv[0]);
    iv[0][1] = _mm_add_epi32(iv[0][1], one.v);

    field_mul(&hash_key[1], &hash_key[0], &hash_key[0]);
    field_mul(&hash_key[2], &hash_key[1], &hash_key[0]);
#ifndef __GNUC__
#pragma region AD
#endif
    while (ad_len >= (3 * BLCKSIZE)) {
        msg[0].val[0] = _mm_loadu_si128(ad_ptr + 0 + 0);
        msg[0].val[1] = _mm_loadu_si128(ad_ptr + 0 + 1);
        msg[1].val[0] = _mm_loadu_si128(ad_ptr + 2 + 0);
        msg[1].val[1] = _mm_loadu_si128(ad_ptr + 2 + 1);
        msg[2].val[0] = _mm_loadu_si128(ad_ptr + 4 + 0);
        msg[2].val[1] = _mm_loadu_si128(ad_ptr + 4 + 1);
        ad_ptr += 6;
        ad_len -= (3 * BLCKSIZE);

        field_mul_no_carry(&dacc1, &msg[0], &hash_key[2]);
        field_mul_no_carry(&dacc2, &msg[1], &hash_key[1]);
        field_mul_no_carry(&dacc3, &msg[2], &hash_key[0]);
        field_add_dbl(&dacc1, &dacc1, &dacc2);
        field_add_dbl(&dacc1, &dacc1, &dacc3);

        field_mul_no_carry(&dacc2, &acc, &hash_key[2]);
        field_add_dbl(&dacc1, &dacc1, &dacc2);
        carry_round(&acc, &dacc1);
    }
    if (ad_len >= (2 * BLCKSIZE)) {
        msg[0].val[0] = _mm_loadu_si128(ad_ptr + 0 + 0);
        msg[0].val[1] = _mm_loadu_si128(ad_ptr + 0 + 1);
        msg[1].val[0] = _mm_loadu_si128(ad_ptr + 2 + 0);
        msg[1].val[1] = _mm_loadu_si128(ad_ptr + 2 + 1);
        ad_ptr += 4;
        ad_len -= (2 * BLCKSIZE);
        field_mul_no_carry(&dacc1, &msg[0], &hash_key[1]);
        field_mul_no_carry(&dacc2, &msg[1], &hash_key[0]);
        field_add_dbl(&dacc1, &dacc1, &dacc2);
        field_mul_no_carry(&dacc2, &acc, &hash_key[1]);
        field_add_dbl(&dacc1, &dacc1, &dacc2);
        carry_round(&acc, &dacc1);
    }
    if (ad_len >= BLCKSIZE) {
        msg[0].val[0] = _mm_loadu_si128(ad_ptr + 0);
        msg[0].val[1] = _mm_loadu_si128(ad_ptr + 1);
        ad_ptr += 2;
        ad_len -= BLCKSIZE;
        field_add(&acc, &acc, &msg[0]);
        field_mul(&acc, &acc, &hash_key[0]);
    }
    if (ad_len > 0) {
        msg[0].val[0] = _mm_setzero_si128();
        msg[0].val[1] = _mm_setzero_si128();
        memcpy(msg[0].val, ad_ptr, ad_len);
        ad_len = 0;
        field_add(&acc, &acc, &msg[0]);
        field_mul(&acc, &acc, &hash_key[0]);
    }
#ifndef __GNUC__
#pragma endregion AD
#endif

#ifndef __GNUC__
#pragma region Encryption
#endif
    while (m_len >= (3 * BLCKSIZE)) {
        msg[0].val[0] = _mm_loadu_si128(m_ptr + 0 + 0);
        msg[0].val[1] = _mm_loadu_si128(m_ptr + 0 + 1);
        msg[1].val[0] = _mm_loadu_si128(m_ptr + 2 + 0);
        msg[1].val[1] = _mm_loadu_si128(m_ptr + 2 + 1);
        msg[2].val[0] = _mm_loadu_si128(m_ptr + 4 + 0);
        msg[2].val[1] = _mm_loadu_si128(m_ptr + 4 + 1);
        m_ptr += 6;
        m_len -= (3 * BLCKSIZE);
        rijndael256blockXORx3((__m128i *)(&ct[0]), roundkeys, (__m128i *)iv,
                              (__m128i *)(&msg[0]));
        _mm_storeu_si128(ct_ptr + 0 + 0, ct[0].val[0]);
        _mm_storeu_si128(ct_ptr + 0 + 1, ct[0].val[1]);
        _mm_storeu_si128(ct_ptr + 2 + 0, ct[1].val[0]);
        _mm_storeu_si128(ct_ptr + 2 + 1, ct[1].val[1]);
        _mm_storeu_si128(ct_ptr + 4 + 0, ct[2].val[0]);
        _mm_storeu_si128(ct_ptr + 4 + 1, ct[2].val[1]);
        ct_ptr += 6;

        field_mul_no_carry(&dacc1, &ct[0], &hash_key[2]);
        field_mul_no_carry(&dacc2, &ct[1], &hash_key[1]);
        field_mul_no_carry(&dacc3, &ct[2], &hash_key[0]);

        field_add_dbl(&dacc1, &dacc1, &dacc2);
        field_add_dbl(&dacc1, &dacc1, &dacc3);

        field_mul_no_carry(&dacc2, &acc, &hash_key[2]);
        field_add_dbl(&dacc1, &dacc1, &dacc2);

        carry_round(&acc, &dacc1);
        iv[0][1] = _mm_add_epi32(iv[0][1], three.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], three.v);
        iv[2][1] = _mm_add_epi32(iv[2][1], three.v);
    }
    if (m_len >= (2 * BLCKSIZE)) {
        msg[0].val[0] = _mm_loadu_si128(m_ptr + 0 + 0);
        msg[0].val[1] = _mm_loadu_si128(m_ptr + 0 + 1);
        msg[1].val[0] = _mm_loadu_si128(m_ptr + 2 + 0);
        msg[1].val[1] = _mm_loadu_si128(m_ptr + 2 + 1);
        m_ptr += 4;
        m_len -= (2 * BLCKSIZE);
        rijndael256blockXORx2((__m128i *)(&ct[0]), roundkeys, (__m128i *)iv,
                              (__m128i *)(&msg[0]));

        _mm_storeu_si128(ct_ptr + 0 + 0, ct[0].val[0]);
        _mm_storeu_si128(ct_ptr + 0 + 1, ct[0].val[1]);
        _mm_storeu_si128(ct_ptr + 2 + 0, ct[1].val[0]);
        _mm_storeu_si128(ct_ptr + 2 + 1, ct[1].val[1]);

        field_mul_no_carry(&dacc1, &ct[0], &hash_key[1]);
        field_mul_no_carry(&dacc2, &ct[1], &hash_key[0]);
        field_add_dbl(&dacc1, &dacc1, &dacc2);
        field_mul_no_carry(&dacc2, &acc, &hash_key[1]);
        field_add_dbl(&dacc1, &dacc1, &dacc2);
        carry_round(&acc, &dacc1);
        ct_ptr += 4;
        iv[0][1] = _mm_add_epi32(iv[0][1], two.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], two.v);
    }
    if (m_len >= BLCKSIZE) {
        msg[0].val[0] = _mm_loadu_si128(m_ptr + 0);
        msg[0].val[1] = _mm_loadu_si128(m_ptr + 1);
        m_ptr += 2;
        m_len -= BLCKSIZE;
        rijndael256blockXOR(ct[0].val, roundkeys, iv[0], msg[0].val);
        field_add(&acc, &acc, &ct[0]);
        field_mul(&acc, &acc, &hash_key[0]);
        _mm_storeu_si128(ct_ptr + 0, ct[0].val[0]);
        _mm_storeu_si128(ct_ptr + 1, ct[0].val[1]);
        ct_ptr += 2;
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
    }
    if (m_len > 0) {
        msg[0].val[0] = _mm_setzero_si128();
        msg[0].val[1] = _mm_setzero_si128();
        memcpy(msg[0].val, m_ptr, m_len);
        rijndael256blockXOR(ct[0].val, roundkeys, iv[0], msg[0].val);
        if (m_len > 16) {
            store_mask = _mm_cmpgt_epi8(
                _mm_set1_epi8((unsigned char)m_len - 16), BYTE_IDX.v);
            ct[0].val[1] = _mm_and_si128(ct[0].val[1], store_mask);
        } else {
            store_mask =
                _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)m_len), BYTE_IDX.v);
            ct[0].val[0] = _mm_and_si128(ct[0].val[0], store_mask);
        }
        _mm_storeu_si128(ct_ptr + 0, ct[0].val[0]);
        _mm_storeu_si128(ct_ptr + 1, ct[0].val[1]);
        ct_ptr += 2;
        m_len = 0;
        field_add(&acc, &acc, &ct[0]);
        field_mul(&acc, &acc, &hash_key[0]);
    }
#ifndef __GNUC__
#pragma endregion Encryption
#endif
    field_add(&acc, &acc, &len_blck);
    field_mul(&acc, &acc, &hash_key[0]);
    field_add(&acc, &acc, &len);
    field_mul(&acc, &acc, &hash_key[0]);
    field_add(&acc, &acc, &wc_key);
    _mm_storeu_si128(ct_ptr + 0, acc.val[0]);
    _mm_storeu_si128(ct_ptr + 1, acc.val[1]);
    return;
}

void rijndael256x_poly256_corex4(size_t m_len, const uint8_t m[static m_len],
                                 size_t ad_len, const uint8_t ad[static ad_len],
                                 uint8_t c[static CTXT_LEN(m_len)],
                                 const uint8_t key[static KEY_SIZE],
                                 const uint8_t nonce[NONCE_SIZE])
{
    __m128i *m_ptr = (__m128i *)m;
    __m128i *ct_ptr = (__m128i *)c;
    __m128i *ad_ptr = (__m128i *)ad;
    field_elem_t wc_key = {0};
    __m128i input[2] = {0};
    field_elem_t len_blck = {0}; // stores the AEAD length encoding
    field_elem_t len = {0}; // stores the length of the hash input (including
                            // AEAD length encoding)

    __m128i store_mask;

    field_elem_t acc = {0};
    field_elem_t hash_key[4] = {0};

    field_elem_t ct[4] = {0};
    field_elem_t msg[4] = {0};
    dfield_elem_t dacc1 = {0};
    __m128i iv[4][2];

    dfield_elem_t dacc2 = {0};
    dfield_elem_t dacc3 = {0};
    dfield_elem_t dacc4 = {0};

    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);
    size_t int_len = ad_len + PAD_LEN(ad_len) + m_len + PAD_LEN(m_len) +
                     2 * sizeof(uint64_t);
    memcpy(&len, &int_len, sizeof(size_t));

    len_blck.val[0] = _mm_set_epi64x((uint64_t)m_len, (uint64_t)ad_len);
    len_blck.val[1] = _mm_setzero_si128();

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
    rijndael256block(hash_key[0].val, roundkeys, input);
    rijndael256block(wc_key.val, roundkeys, iv[0]);
    iv[0][1] = _mm_add_epi32(iv[0][1], one.v);

    field_mul(&hash_key[1], &hash_key[0], &hash_key[0]);
    field_mul(&hash_key[2], &hash_key[1], &hash_key[0]);
    field_mul(&hash_key[3], &hash_key[2], &hash_key[0]);

#ifndef __GNUC__
#pragma region AD
#endif
    while (ad_len >= (4 * BLCKSIZE)) {
        msg[0].val[0] = _mm_loadu_si128(ad_ptr + 0 + 0);
        msg[0].val[1] = _mm_loadu_si128(ad_ptr + 0 + 1);
        msg[1].val[0] = _mm_loadu_si128(ad_ptr + 2 + 0);
        msg[1].val[1] = _mm_loadu_si128(ad_ptr + 2 + 1);
        msg[2].val[0] = _mm_loadu_si128(ad_ptr + 4 + 0);
        msg[2].val[1] = _mm_loadu_si128(ad_ptr + 4 + 1);
        msg[3].val[0] = _mm_loadu_si128(ad_ptr + 6 + 0);
        msg[3].val[1] = _mm_loadu_si128(ad_ptr + 6 + 1);
        ad_ptr += 8;
        ad_len -= (4 * BLCKSIZE);

        field_mul_no_carry(&dacc1, &msg[0], &hash_key[3]);
        field_mul_no_carry(&dacc2, &msg[1], &hash_key[2]);
        field_mul_no_carry(&dacc3, &msg[2], &hash_key[1]);
        field_mul_no_carry(&dacc4, &msg[3], &hash_key[0]);
        field_add_dbl(&dacc1, &dacc1, &dacc2);
        field_add_dbl(&dacc3, &dacc4, &dacc3);
        field_add_dbl(&dacc1, &dacc1, &dacc3);
        field_mul_no_carry(&dacc2, &acc, &hash_key[3]);
        field_add_dbl(&dacc1, &dacc1, &dacc2);
        carry_round(&acc, &dacc1);
    }
    if (ad_len >= (3 * BLCKSIZE)) {
        msg[0].val[0] = _mm_loadu_si128(ad_ptr + 0 + 0);
        msg[0].val[1] = _mm_loadu_si128(ad_ptr + 0 + 1);
        msg[1].val[0] = _mm_loadu_si128(ad_ptr + 2 + 0);
        msg[1].val[1] = _mm_loadu_si128(ad_ptr + 2 + 1);
        msg[2].val[0] = _mm_loadu_si128(ad_ptr + 4 + 0);
        msg[2].val[1] = _mm_loadu_si128(ad_ptr + 4 + 1);
        ad_ptr += 6;
        ad_len -= (3 * BLCKSIZE);

        field_mul_no_carry(&dacc1, &msg[0], &hash_key[2]);
        field_mul_no_carry(&dacc2, &msg[1], &hash_key[1]);
        field_mul_no_carry(&dacc3, &msg[2], &hash_key[0]);
        field_add_dbl(&dacc1, &dacc1, &dacc2);
        field_add_dbl(&dacc1, &dacc1, &dacc3);

        field_mul_no_carry(&dacc2, &acc, &hash_key[2]);
        field_add_dbl(&dacc1, &dacc1, &dacc2);
        carry_round(&acc, &dacc1);
    }
    if (ad_len >= (2 * BLCKSIZE)) {
        msg[0].val[0] = _mm_loadu_si128(ad_ptr + 0 + 0);
        msg[0].val[1] = _mm_loadu_si128(ad_ptr + 0 + 1);
        msg[1].val[0] = _mm_loadu_si128(ad_ptr + 2 + 0);
        msg[1].val[1] = _mm_loadu_si128(ad_ptr + 2 + 1);
        ad_ptr += 4;
        ad_len -= (2 * BLCKSIZE);
        field_mul_no_carry(&dacc1, &msg[0], &hash_key[1]);
        field_mul_no_carry(&dacc2, &msg[1], &hash_key[0]);
        field_add_dbl(&dacc1, &dacc1, &dacc2);
        field_mul_no_carry(&dacc2, &acc, &hash_key[1]);
        field_add_dbl(&dacc1, &dacc1, &dacc2);
        carry_round(&acc, &dacc1);
    }
    if (ad_len >= BLCKSIZE) {
        msg[0].val[0] = _mm_loadu_si128(ad_ptr + 0);
        msg[0].val[1] = _mm_loadu_si128(ad_ptr + 1);
        ad_ptr += 2;
        ad_len -= BLCKSIZE;
        field_add(&acc, &acc, &msg[0]);
        field_mul(&acc, &acc, &hash_key[0]);
    }
    if (ad_len > 0) {
        msg[0].val[0] = _mm_setzero_si128();
        msg[0].val[1] = _mm_setzero_si128();
        memcpy(msg[0].val, ad_ptr, ad_len);
        ad_len = 0;
        field_add(&acc, &acc, &msg[0]);
        field_mul(&acc, &acc, &hash_key[0]);
    }
#ifndef __GNUC__
#pragma endregion AD
#endif

#ifndef __GNUC__
#pragma region Encryption
#endif
    while (m_len >= (4 * BLCKSIZE)) {
        msg[0].val[0] = _mm_loadu_si128(m_ptr + 0 + 0);
        msg[0].val[1] = _mm_loadu_si128(m_ptr + 0 + 1);
        msg[1].val[0] = _mm_loadu_si128(m_ptr + 2 + 0);
        msg[1].val[1] = _mm_loadu_si128(m_ptr + 2 + 1);
        msg[2].val[0] = _mm_loadu_si128(m_ptr + 4 + 0);
        msg[2].val[1] = _mm_loadu_si128(m_ptr + 4 + 1);
        msg[3].val[0] = _mm_loadu_si128(m_ptr + 6 + 0);
        msg[3].val[1] = _mm_loadu_si128(m_ptr + 6 + 1);
        m_ptr += 8;
        m_len -= (4 * BLCKSIZE);

        rijndael256blockXORx4((__m128i *)(&ct[0]), roundkeys, (__m128i *)iv,
                              (__m128i *)(&msg[0]));

        _mm_storeu_si128(ct_ptr + 0 + 0, ct[0].val[0]);
        _mm_storeu_si128(ct_ptr + 0 + 1, ct[0].val[1]);
        _mm_storeu_si128(ct_ptr + 2 + 0, ct[1].val[0]);
        _mm_storeu_si128(ct_ptr + 2 + 1, ct[1].val[1]);
        _mm_storeu_si128(ct_ptr + 4 + 0, ct[2].val[0]);
        _mm_storeu_si128(ct_ptr + 4 + 1, ct[2].val[1]);
        _mm_storeu_si128(ct_ptr + 6 + 0, ct[3].val[0]);
        _mm_storeu_si128(ct_ptr + 6 + 1, ct[3].val[1]);
        ct_ptr += 8;

        field_mul_no_carry(&dacc1, &ct[0], &hash_key[3]);
        field_mul_no_carry(&dacc2, &ct[1], &hash_key[2]);
        field_mul_no_carry(&dacc3, &ct[2], &hash_key[1]);
        field_mul_no_carry(&dacc4, &ct[3], &hash_key[0]);

        field_add_dbl(&dacc1, &dacc1, &dacc2);
        field_add_dbl(&dacc3, &dacc4, &dacc3);
        field_add_dbl(&dacc1, &dacc1, &dacc3);

        field_mul_no_carry(&dacc2, &acc, &hash_key[3]);
        field_add_dbl(&dacc1, &dacc1, &dacc2);

        carry_round(&acc, &dacc1);
        iv[0][1] = _mm_add_epi32(iv[0][1], four.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], four.v);
        iv[2][1] = _mm_add_epi32(iv[2][1], four.v);
        iv[3][1] = _mm_add_epi32(iv[3][1], four.v);
    }
    if (m_len >= (3 * BLCKSIZE)) {
        msg[0].val[0] = _mm_loadu_si128(m_ptr + 0 + 0);
        msg[0].val[1] = _mm_loadu_si128(m_ptr + 0 + 1);
        msg[1].val[0] = _mm_loadu_si128(m_ptr + 2 + 0);
        msg[1].val[1] = _mm_loadu_si128(m_ptr + 2 + 1);
        msg[2].val[0] = _mm_loadu_si128(m_ptr + 4 + 0);
        msg[2].val[1] = _mm_loadu_si128(m_ptr + 4 + 1);
        m_ptr += 6;
        m_len -= (3 * BLCKSIZE);

        rijndael256blockXORx3((__m128i *)(&ct[0]), roundkeys, (__m128i *)iv,
                              (__m128i *)(&msg[0]));

        _mm_storeu_si128(ct_ptr + 0 + 0, ct[0].val[0]);
        _mm_storeu_si128(ct_ptr + 0 + 1, ct[0].val[1]);
        _mm_storeu_si128(ct_ptr + 2 + 0, ct[1].val[0]);
        _mm_storeu_si128(ct_ptr + 2 + 1, ct[1].val[1]);
        _mm_storeu_si128(ct_ptr + 4 + 0, ct[2].val[0]);
        _mm_storeu_si128(ct_ptr + 4 + 1, ct[2].val[1]);
        ct_ptr += 6;

        field_mul_no_carry(&dacc1, &ct[0], &hash_key[2]);
        field_mul_no_carry(&dacc2, &ct[1], &hash_key[1]);
        field_mul_no_carry(&dacc3, &ct[2], &hash_key[0]);

        field_add_dbl(&dacc1, &dacc1, &dacc2);
        field_add_dbl(&dacc1, &dacc1, &dacc3);

        field_mul_no_carry(&dacc2, &acc, &hash_key[2]);
        field_add_dbl(&dacc1, &dacc1, &dacc2);

        carry_round(&acc, &dacc1);
        iv[0][1] = _mm_add_epi32(iv[0][1], three.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], three.v);
        iv[2][1] = _mm_add_epi32(iv[2][1], three.v);
    }
    if (m_len >= (2 * BLCKSIZE)) {
        msg[0].val[0] = _mm_loadu_si128(m_ptr + 0 + 0);
        msg[0].val[1] = _mm_loadu_si128(m_ptr + 0 + 1);
        msg[1].val[0] = _mm_loadu_si128(m_ptr + 2 + 0);
        msg[1].val[1] = _mm_loadu_si128(m_ptr + 2 + 1);
        m_ptr += 4;
        m_len -= (2 * BLCKSIZE);

        rijndael256blockXORx2((__m128i *)(&ct[0]), roundkeys, (__m128i *)iv,
                              (__m128i *)(&msg[0]));

        _mm_storeu_si128(ct_ptr + 0 + 0, ct[0].val[0]);
        _mm_storeu_si128(ct_ptr + 0 + 1, ct[0].val[1]);
        _mm_storeu_si128(ct_ptr + 2 + 0, ct[1].val[0]);
        _mm_storeu_si128(ct_ptr + 2 + 1, ct[1].val[1]);

        field_mul_no_carry(&dacc1, &ct[0], &hash_key[1]);
        field_mul_no_carry(&dacc2, &ct[1], &hash_key[0]);
        field_add_dbl(&dacc1, &dacc1, &dacc2);
        field_mul_no_carry(&dacc2, &acc, &hash_key[1]);
        field_add_dbl(&dacc1, &dacc1, &dacc2);
        carry_round(&acc, &dacc1);
        ct_ptr += 4;
        iv[0][1] = _mm_add_epi32(iv[0][1], two.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], two.v);
    }
    if (m_len >= BLCKSIZE) {
        msg[0].val[0] = _mm_loadu_si128(m_ptr + 0);
        msg[0].val[1] = _mm_loadu_si128(m_ptr + 1);
        m_ptr += 2;
        m_len -= BLCKSIZE;
        rijndael256blockXOR(ct[0].val, roundkeys, iv[0], msg[0].val);
        field_add(&acc, &acc, &ct[0]);
        field_mul(&acc, &acc, &hash_key[0]);
        _mm_storeu_si128(ct_ptr + 0, ct[0].val[0]);
        _mm_storeu_si128(ct_ptr + 1, ct[0].val[1]);
        ct_ptr += 2;
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
    }
    if (m_len > 0) {
        msg[0].val[0] = _mm_setzero_si128();
        msg[0].val[1] = _mm_setzero_si128();
        memcpy(msg[0].val, m_ptr, m_len);
        rijndael256blockXOR(ct[0].val, roundkeys, iv[0], msg[0].val);
        if (m_len > 16) {
            store_mask = _mm_cmpgt_epi8(
                _mm_set1_epi8((unsigned char)m_len - 16), BYTE_IDX.v);
            ct[0].val[1] = _mm_and_si128(ct[0].val[1], store_mask);
        } else {
            store_mask =
                _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)m_len), BYTE_IDX.v);
            ct[0].val[0] = _mm_and_si128(ct[0].val[0], store_mask);
        }
        _mm_storeu_si128(ct_ptr + 0, ct[0].val[0]);
        _mm_storeu_si128(ct_ptr + 1, ct[0].val[1]);
        ct_ptr += 2;
        m_len = 0;
        field_add(&acc, &acc, &ct[0]);
        field_mul(&acc, &acc, &hash_key[0]);
    }

#ifndef __GNUC__
#pragma endregion Encryption
#endif
    field_add(&acc, &acc, &len_blck);
    field_mul(&acc, &acc, &hash_key[0]);
    field_add(&acc, &acc, &len);
    field_mul(&acc, &acc, &hash_key[0]);
    field_add(&acc, &acc, &wc_key);
    _mm_storeu_si128(ct_ptr + 0, acc.val[0]);
    _mm_storeu_si128(ct_ptr + 1, acc.val[1]);
    return;
}
