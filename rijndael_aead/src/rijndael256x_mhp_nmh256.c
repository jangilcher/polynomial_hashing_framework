#include <assert.h>
#include <immintrin.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "fieldArith/bf_arithmetic_256_64_64_64_64_64_schoolbook.h"
#include "fieldArith/fieldArith.h"
#include "rijndael256/rijndael256.h"
#include "rijndael256/rijndael256x_mhp_nmh256.h"
#include "rijndael256_impl.h"

#define SPRBLCKSIZE 24

const vector BYTE_IDX = {
    .i8 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}};

typedef union {
    __m256i *lv;
    __m128i *v;
    __uint8_t *c;
} vec_ptr;

void rijndael256x_mhp_nmh256x2(size_t m_len, const uint8_t m[static m_len],
                               size_t ad_len, const uint8_t ad[static ad_len],
                               uint8_t c[static CTXT_LEN(m_len)],
                               const uint8_t key[static KEY_SIZE],
                               const uint8_t nonce[NONCE_SIZE])
{
    size_t blck_num = 0;
    dfield_elem_t dacc = {0};
    field_elem_t acc = {0};
    dfield_elem_t tmp = {0};
    field_elem_t odd = {0};
    field_elem_t evn = {0};
    field_elem_t h_msg[2] = {0};

    field_elem_t inner_keys[SPRBLCKSIZE] = {0};
    field_elem_t outer_key = {0};
    field_elem_t le_key = {0};
    field_elem_t wc_key = {0};
    field_elem_t len = {0}; // stores the length of the hash input (including
                            // AEAD length encoding)
    field_elem_t len_blck;  // stores the AEAD length encoding

    vec_ptr m_ptr = {.v = (__m128i *)m};
    vec_ptr ad_ptr = {.v = (__m128i *)ad};
    vec_ptr ct_ptr = {.v = (__m128i *)c};
    vec_ptr ct_r_ptr = {.v = (__m128i *)c};

    __m128i store_mask;

    __m128i ct[4] = {0};
    __m128i msg[4] = {0};
    __m128i iv[2][2];
    

    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    int len_added = 0;
    size_t to_enc = m_len;

    size_t int_len = ad_len + PAD_LEN(ad_len) + m_len + PAD_LEN(m_len) +
                     2 * sizeof(uint64_t);
    memcpy(&len, &int_len, sizeof(size_t));
    len_blck.val[0] = _mm_set_epi64x((uint64_t)m_len, (uint64_t)ad_len);
    len_blck.val[1] = _mm_setzero_si128();

    iv[0][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5],
                                 ((uint32_t *)nonce)[6], 0}})
                   .v;
    iv[0][1] = _mm_add_epi32(iv[0][1], three.v);

    rijndael256block((__m128i *)&wc_key, roundkeys, iv[0]);
    iv[1][0] = iv[0][0];
    iv[1][1] = _mm_add_epi32(iv[0][1], two.v);
    iv[0][1] = _mm_add_epi32(iv[0][1], one.v);

    __m128i input[2];
    input[0] = zero.v;
    input[1] = zero.v;
    rijndael256block((__m128i *)&inner_keys[0], roundkeys, input);
    input[1] = one.v;
    rijndael256block((__m128i *)&outer_key, roundkeys, input);
    input[1] = two.v;
    rijndael256block((__m128i *)&le_key, roundkeys, input);

    PRINT_FIELD_ELEM(inner_keys[0]);
    PRINT_FIELD_ELEM(outer_key);
    PRINT_FIELD_ELEM(le_key);
    PRINT_FIELD_ELEM(wc_key);

    for (int j = 2; j < SPRBLCKSIZE; j += 2) {
        field_mul(&inner_keys[j], &inner_keys[j - 2], &inner_keys[0]);
    }
    field_mul(&inner_keys[SPRBLCKSIZE - 1], &inner_keys[SPRBLCKSIZE - 2],
              &inner_keys[0]);
    for (int j = SPRBLCKSIZE - 3; j > 0; j -= 2) {
        field_mul(&inner_keys[j], &inner_keys[j + 2], &inner_keys[0]);
    }
#ifndef __GNUC__
#pragma region AD
#endif
    /*PROCESS AD */
    while (ad_len >= (2 * HASHBLCK_SIZE)) {

        h_msg[0].val[0] = _mm_loadu_si128(ad_ptr.v + 0 + 0);
        h_msg[0].val[1] = _mm_loadu_si128(ad_ptr.v + 0 + 1);
        h_msg[1].val[0] = _mm_loadu_si128(ad_ptr.v + 2 + 0);
        h_msg[1].val[1] = _mm_loadu_si128(ad_ptr.v + 2 + 1);
        ad_ptr.v += 4;
        ad_len -= (2 * HASHBLCK_SIZE);
        field_add(&evn, &h_msg[0], &(inner_keys[blck_num + 0]));
        field_add(&odd, &h_msg[1], &(inner_keys[blck_num + 1]));
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
    }
    if (ad_len >= HASHBLCK_SIZE) {
        h_msg[0].val[0] = _mm_loadu_si128(ad_ptr.v);
        h_msg[0].val[1] = _mm_loadu_si128(ad_ptr.v + 1);
        h_msg[1].val[0] = _mm_setzero_si128();
        h_msg[1].val[1] = _mm_setzero_si128();
        memcpy(&h_msg[1], ad_ptr.c + HASHBLCK_SIZE, ad_len - HASHBLCK_SIZE);

        field_add(&evn, &h_msg[0], &(inner_keys[blck_num]));
        field_add(&odd, &h_msg[1], &(inner_keys[blck_num + 1]));
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
    } else if (ad_len > 0) {

        h_msg[0].val[0] = _mm_setzero_si128();
        h_msg[0].val[1] = _mm_setzero_si128();
        memcpy(&h_msg[0], ad_ptr.c, ad_len);
        field_add(&evn, &h_msg[0], &(inner_keys[blck_num]));
        // AD was uneven so we add on messageblock so that we are even again
        if (to_enc > 0) {
            // If we have some message left, encrypt and add to hash.
            if (to_enc >= BLCKSIZE) {
                msg[0] = _mm_loadu_si128(m_ptr.v);
                msg[1] = _mm_loadu_si128(m_ptr.v);
                m_ptr.v += 2;
                to_enc -= BLCKSIZE;
                rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
                _mm_storeu_si128(ct_ptr.v, ct[0]);
                _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
                ct_ptr.v += 2;
            } else if (to_enc >= 16) {
                msg[1] = _mm_setzero_si128();
                memcpy(&msg[0], m_ptr.v, to_enc);
                rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
                store_mask = _mm_cmpgt_epi8(
                    _mm_set1_epi8((unsigned char)to_enc - 16), BYTE_IDX.v);
                ct[1] = _mm_and_si128(ct[1], store_mask);
                memcpy(ct_ptr.v, &ct[0], to_enc);
                ct_ptr.c += to_enc;
                to_enc = 0;
            } else {
                msg[0] = _mm_setzero_si128();
                msg[1] = _mm_setzero_si128();
                memcpy(&msg[0], m_ptr.v, to_enc);
                rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
                store_mask = _mm_cmpgt_epi8(
                    _mm_set1_epi8((unsigned char)to_enc), BYTE_IDX.v);
                ct[0] = _mm_and_si128(ct[0], store_mask);
                memcpy(ct_ptr.v, &ct[0], to_enc);
                ct_ptr.c += to_enc;
                to_enc = 0;
            }
            if (ct_r_ptr.c + HASHBLCK_SIZE <= ct_ptr.c) {
                h_msg[1].val[0] = _mm_loadu_si128(ct_r_ptr.v);
                h_msg[1].val[1] = _mm_loadu_si128(ct_r_ptr.v + 1);
                ct_r_ptr.c += HASHBLCK_SIZE;
            } else {
                h_msg[1].val[0] = _mm_setzero_si128();
                h_msg[1].val[1] = _mm_setzero_si128();
                memcpy(&h_msg[1], ct_r_ptr.c, ct_ptr.c - ct_r_ptr.c);
                ct_r_ptr.c = ct_ptr.c;
            }
            field_add(&odd, &h_msg[1], &(inner_keys[blck_num + 1]));

            iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
            iv[1][1] = _mm_add_epi32(iv[1][1], one.v);
        } else {
            // We are out of message so we add the last block containing AEAD
            // length encoding
            // PRINT_FIELD_ELEM(len_blck);
            field_add(&odd, &len_blck, &(inner_keys[blck_num + 1]));
            // DEBUG_PRINTF("blck_num: %zu\n", blck_num);
            // PRINT_FIELD_ELEM(odd);
            len_added = 1;
        }
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
    }

#ifndef __GNUC__
#pragma endregion AD
#endif

#ifndef __GNUC__
#pragma region ENCRYPTION
#endif
    while (to_enc >= 64) {
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        m_ptr.v += 4;
        to_enc -= 64;

        rijndael256blockXORx2(&ct[0], roundkeys, (__m128i *)iv, &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);

        iv[0][1] = _mm_add_epi32(iv[0][1], two.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], two.v);
        ct_ptr.v += 4;

        h_msg[0].val[0] = _mm_loadu_si128(ct_r_ptr.v + 0 + 0);
        h_msg[0].val[1] = _mm_loadu_si128(ct_r_ptr.v + 0 + 1);
        h_msg[1].val[0] = _mm_loadu_si128(ct_r_ptr.v + 2 + 0);
        h_msg[1].val[1] = _mm_loadu_si128(ct_r_ptr.v + 2 + 1);
        ct_r_ptr.v += 4;
        field_add(&evn, &h_msg[0], &(inner_keys[blck_num + 0]));
        field_add(&odd, &h_msg[1], &(inner_keys[blck_num + 1]));
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
    }

    if (to_enc >= BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0 + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 0 + 1);
        m_ptr.v += 2;
        to_enc -= BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], one.v);
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

/* Handle remaining hash*/
#ifndef __GNUC__
#pragma region HASH FINALIZE
#endif
    while (ct_r_ptr.c + (2 * HASHBLCK_SIZE) <= ct_ptr.c) {
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
        h_msg[0].val[0] = _mm_loadu_si128(ct_r_ptr.v + 0 + 0);
        h_msg[0].val[1] = _mm_loadu_si128(ct_r_ptr.v + 0 + 1);
        h_msg[1].val[0] = _mm_loadu_si128(ct_r_ptr.v + 2 + 0);
        h_msg[1].val[1] = _mm_loadu_si128(ct_r_ptr.v + 2 + 1);
        ct_r_ptr.v += 4;
        field_add(&evn, &h_msg[0], &(inner_keys[blck_num + 0]));
        field_add(&odd, &h_msg[1], &(inner_keys[blck_num + 1]));
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
    }
    if (ct_r_ptr.c + (HASHBLCK_SIZE) < ct_ptr.c) {
        HERE;
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }

        h_msg[0].val[0] = _mm_loadu_si128(ct_r_ptr.v + 0 + 0);
        h_msg[0].val[1] = _mm_loadu_si128(ct_r_ptr.v + 0 + 1);

        h_msg[1].val[0] = _mm_setzero_si128();
        h_msg[1].val[1] = _mm_setzero_si128();
        memcpy(&h_msg[1], ct_r_ptr.c + HASHBLCK_SIZE,
               ct_ptr.c - (ct_r_ptr.c + HASHBLCK_SIZE));
        ct_r_ptr.c = ct_ptr.c;

        field_add(&evn, &h_msg[0], &(inner_keys[blck_num + 0]));
        field_add(&odd, &h_msg[1], &(inner_keys[blck_num + 1]));
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;

    } else if ((ct_r_ptr.c < ct_ptr.c)) {
        // DEBUG_PRINTF("ct_r_ptr.c < ct_ptr\n");
        // DEBUG_PRINTF("blck_num: %zu\n", blck_num);
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }

        h_msg[0].val[0] = _mm_setzero_si128();
        h_msg[0].val[1] = _mm_setzero_si128();
        memcpy(&h_msg[0], ct_r_ptr.c, ct_ptr.c - ct_r_ptr.c);

        ct_r_ptr.c = ct_ptr.c;

        PRINT_FIELD_ELEM(h_msg[0]);
        PRINT_FIELD_ELEM(len_blck);
        PRINT_FIELD_ELEM(inner_keys[blck_num + 0]);
        PRINT_FIELD_ELEM(inner_keys[blck_num + 1]);
        field_add(&evn, &h_msg[0], &(inner_keys[blck_num + 0]));
        field_add(&odd, &len_blck, &(inner_keys[blck_num + 1]));
        PRINT_FIELD_ELEM(evn);
        PRINT_FIELD_ELEM(odd);
        DEBUG_PRINTF("blck_num: %zu\n", blck_num);
        field_mul_no_carry(&tmp, &evn, &odd);
        // PRINT_DFIELD_ELEM(tmp);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
        len_added = 1;
    }
    if (!len_added) {
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
        // PRINT_FIELD_ELEM(len_blck);
        // PRINT_DFIELD_ELEM(dacc);
        field_add_mix(&dacc, &dacc, &len_blck);
        // PRINT_DFIELD_ELEM(dacc);
    }
#ifndef __GNUC__
#pragma endregion HASH FINALIZE
#endif
    field_add_mix(&dacc, &dacc, &le_key);
    PRINT_DFIELD_ELEM(dacc);
    carry_round(&acc, &dacc);
    PRINT_FIELD_ELEM(acc);
    field_add(&evn, &inner_keys[0], &len);
    PRINT_FIELD_ELEM(len);
    PRINT_FIELD_ELEM(evn);
    field_mul(&acc, &acc, &evn);
    PRINT_FIELD_ELEM(acc);
    field_add(&acc, &acc, &wc_key);
    PRINT_FIELD_ELEM(acc);
    memcpy(c + PADDED_LEN(m_len), &acc, TAG_SIZE);
    return;
}

void rijndael256x_mhp_nmh256x4(size_t m_len, const uint8_t m[static m_len],
                               size_t ad_len, const uint8_t ad[static ad_len],
                               uint8_t c[static CTXT_LEN(m_len)],
                               const uint8_t key[static KEY_SIZE],
                               const uint8_t nonce[NONCE_SIZE])
{
    size_t blck_num = 0;
    dfield_elem_t dacc = {0};
    field_elem_t acc = {0};
    dfield_elem_t tmp = {0};
    field_elem_t odd = {0};
    field_elem_t evn = {0};
    field_elem_t h_msg[2] = {0};

    field_elem_t inner_keys[SPRBLCKSIZE] = {0};
    field_elem_t outer_key = {0};
    field_elem_t le_key = {0};
    field_elem_t wc_key = {0};
    field_elem_t len = {0}; // stores the length of the hash input (including
                            // AEAD length encoding)
    field_elem_t len_blck;  // stores the AEAD length encoding

    vec_ptr m_ptr = {.v = (__m128i *)m};
    vec_ptr ad_ptr = {.v = (__m128i *)ad};
    vec_ptr ct_ptr = {.v = (__m128i *)c};
    vec_ptr ct_r_ptr = {.v = (__m128i *)c};

    __m128i store_mask;

    __m128i ct[8] = {0};
    __m128i msg[8] = {0};
    __m128i iv[4][2];

    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    int len_added = 0;
    size_t to_enc = m_len;

    size_t int_len = ad_len + PAD_LEN(ad_len) + m_len + PAD_LEN(m_len) +
                     2 * sizeof(uint64_t);
    memcpy(&len, &int_len, sizeof(size_t));
    len_blck.val[0] = _mm_set_epi64x((uint64_t)m_len, (uint64_t)ad_len);
    len_blck.val[1] = _mm_setzero_si128();

    iv[0][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5],
                                 ((uint32_t *)nonce)[6], 0}})
                   .v;
    iv[0][1] = _mm_add_epi32(iv[0][1], three.v);

    rijndael256block((__m128i *)&wc_key, roundkeys, iv[0]);
    iv[1][0] = iv[0][0];
    iv[2][0] = iv[0][0];
    iv[3][0] = iv[0][0];
    iv[3][1] = _mm_add_epi32(iv[0][1], four.v);
    iv[2][1] = _mm_add_epi32(iv[0][1], three.v);
    iv[1][1] = _mm_add_epi32(iv[0][1], two.v);
    iv[0][1] = _mm_add_epi32(iv[0][1], one.v);

    __m128i input[2];
    input[0] = zero.v;
    input[1] = zero.v;
    rijndael256block((__m128i *)&inner_keys[0], roundkeys, input);
    input[1] = one.v;
    rijndael256block((__m128i *)&outer_key, roundkeys, input);
    input[1] = two.v;
    rijndael256block((__m128i *)&le_key, roundkeys, input);

    for (int j = 2; j < SPRBLCKSIZE; j += 2) {
        field_mul(&inner_keys[j], &inner_keys[j - 2], &inner_keys[0]);
    }
    field_mul(&inner_keys[SPRBLCKSIZE - 1], &inner_keys[SPRBLCKSIZE - 2],
              &inner_keys[0]);
    for (int j = SPRBLCKSIZE - 3; j > 0; j -= 2) {
        field_mul(&inner_keys[j], &inner_keys[j + 2], &inner_keys[0]);
    }
#ifndef __GNUC__
#pragma region AD
#endif
    /*PROCESS AD */
    while (ad_len >= (2 * HASHBLCK_SIZE)) {

        h_msg[0].val[0] = _mm_loadu_si128(ad_ptr.v + 0 + 0);
        h_msg[0].val[1] = _mm_loadu_si128(ad_ptr.v + 0 + 1);
        h_msg[1].val[0] = _mm_loadu_si128(ad_ptr.v + 2 + 0);
        h_msg[1].val[1] = _mm_loadu_si128(ad_ptr.v + 2 + 1);
        ad_ptr.v += 4;
        ad_len -= (2 * HASHBLCK_SIZE);
        field_add(&evn, &h_msg[0], &(inner_keys[blck_num + 0]));
        field_add(&odd, &h_msg[1], &(inner_keys[blck_num + 1]));
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
    }
    if (ad_len >= HASHBLCK_SIZE) {
        h_msg[0].val[0] = _mm_loadu_si128(ad_ptr.v);
        h_msg[0].val[1] = _mm_loadu_si128(ad_ptr.v + 1);
        h_msg[1].val[0] = _mm_setzero_si128();
        h_msg[1].val[1] = _mm_setzero_si128();
        memcpy(&h_msg[1], ad_ptr.c + HASHBLCK_SIZE, ad_len - HASHBLCK_SIZE);

        field_add(&evn, &h_msg[0], &(inner_keys[blck_num]));
        field_add(&odd, &h_msg[1], &(inner_keys[blck_num + 1]));
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
    } else if (ad_len > 0) {

        h_msg[0].val[0] = _mm_setzero_si128();
        h_msg[0].val[1] = _mm_setzero_si128();
        memcpy(&h_msg[0], ad_ptr.c, ad_len);
        field_add(&evn, &h_msg[0], &(inner_keys[blck_num]));
        // AD was uneven so we add on messageblock so that we are even again
        if (to_enc > 0) {
            // If we have some message left, encrypt and add to hash.
            if (to_enc >= BLCKSIZE) {
                msg[0] = _mm_loadu_si128(m_ptr.v);
                msg[1] = _mm_loadu_si128(m_ptr.v);
                m_ptr.v += 2;
                to_enc -= BLCKSIZE;
                rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
                _mm_storeu_si128(ct_ptr.v, ct[0]);
                _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
                ct_ptr.v += 2;
            } else if (to_enc >= 16) {
                msg[1] = _mm_setzero_si128();
                memcpy(&msg[0], m_ptr.v, to_enc);
                rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
                store_mask = _mm_cmpgt_epi8(
                    _mm_set1_epi8((unsigned char)to_enc - 16), BYTE_IDX.v);
                ct[1] = _mm_and_si128(ct[1], store_mask);
                memcpy(ct_ptr.v, &ct[0], to_enc);
                ct_ptr.c += to_enc;
                to_enc = 0;
            } else {
                msg[0] = _mm_setzero_si128();
                msg[1] = _mm_setzero_si128();
                memcpy(&msg[0], m_ptr.v, to_enc);
                rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
                store_mask = _mm_cmpgt_epi8(
                    _mm_set1_epi8((unsigned char)to_enc), BYTE_IDX.v);
                ct[0] = _mm_and_si128(ct[0], store_mask);
                memcpy(ct_ptr.v, &ct[0], to_enc);
                ct_ptr.c += to_enc;
                to_enc = 0;
            }
            if (ct_r_ptr.c + HASHBLCK_SIZE <= ct_ptr.c) {
                h_msg[1].val[0] = _mm_loadu_si128(ct_r_ptr.v);
                h_msg[1].val[1] = _mm_loadu_si128(ct_r_ptr.v + 1);
                ct_r_ptr.c += HASHBLCK_SIZE;
            } else {
                h_msg[1].val[0] = _mm_setzero_si128();
                h_msg[1].val[1] = _mm_setzero_si128();
                memcpy(&h_msg[1], ct_r_ptr.c, ct_ptr.c - ct_r_ptr.c);
                ct_r_ptr.c = ct_ptr.c;
            }
            field_add(&odd, &h_msg[1], &(inner_keys[blck_num + 1]));

            iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
            iv[1][1] = _mm_add_epi32(iv[1][1], one.v);
            iv[2][1] = _mm_add_epi32(iv[2][1], one.v);
            iv[3][1] = _mm_add_epi32(iv[3][1], one.v);
        } else {
            // We are out of message so we add the last block containing AEAD
            // length encoding
            // PRINT_FIELD_ELEM(len_blck);
            field_add(&odd, &len_blck, &(inner_keys[blck_num + 1]));
            // DEBUG_PRINTF("blck_num: %zu\n", blck_num);
            // PRINT_FIELD_ELEM(odd);
            len_added = 1;
        }
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
    }

#ifndef __GNUC__
#pragma endregion AD
#endif

#ifndef __GNUC__
#pragma region ENCRYPTION
#endif
    /* PROCESS M */
    while (to_enc >= 4 * BLCKSIZE) {
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        msg[6] = _mm_loadu_si128(m_ptr.v + 6);
        msg[7] = _mm_loadu_si128(m_ptr.v + 7);
        m_ptr.v += 2 * 4;
        to_enc -= 4 * BLCKSIZE;

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
        ct_ptr.v += 2 * 4;

        for (int i = 0; i < 2; i++) {
            h_msg[0].val[0] = _mm_loadu_si128(ct_r_ptr.v + 0 + 0);
            h_msg[0].val[1] = _mm_loadu_si128(ct_r_ptr.v + 0 + 1);
            h_msg[1].val[0] = _mm_loadu_si128(ct_r_ptr.v + 2 + 0);
            h_msg[1].val[1] = _mm_loadu_si128(ct_r_ptr.v + 2 + 1);
            ct_r_ptr.v += 4;
            field_add(&evn, &h_msg[0], &(inner_keys[blck_num + 0]));
            field_add(&odd, &h_msg[1], &(inner_keys[blck_num + 1]));
            field_mul_no_carry(&tmp, &evn, &odd);
            field_add_dbl(&dacc, &dacc, &tmp);
            blck_num += 2;
            if (blck_num == SPRBLCKSIZE) {
                carry_round(&acc, &dacc);
                field_mul_no_carry(&dacc, &acc, &outer_key);
                blck_num = 0;
            }
        }
    }
    while (to_enc >= 64) {
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        m_ptr.v += 4;
        to_enc -= 64;

        rijndael256blockXORx2(&ct[0], roundkeys, (__m128i *)iv, &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);

        iv[0][1] = _mm_add_epi32(iv[0][1], two.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], two.v);
        ct_ptr.v += 4;

        h_msg[0].val[0] = _mm_loadu_si128(ct_r_ptr.v + 0 + 0);
        h_msg[0].val[1] = _mm_loadu_si128(ct_r_ptr.v + 0 + 1);
        h_msg[1].val[0] = _mm_loadu_si128(ct_r_ptr.v + 2 + 0);
        h_msg[1].val[1] = _mm_loadu_si128(ct_r_ptr.v + 2 + 1);
        ct_r_ptr.v += 4;
        field_add(&evn, &h_msg[0], &(inner_keys[blck_num + 0]));
        field_add(&odd, &h_msg[1], &(inner_keys[blck_num + 1]));
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
    }

    if (to_enc >= BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0 + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 0 + 1);
        m_ptr.v += 2;
        to_enc -= BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], one.v);
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

/* Handle remaining hash*/
#ifndef __GNUC__
#pragma region HASH FINALIZE
#endif
    while (ct_r_ptr.c + (2 * HASHBLCK_SIZE) <= ct_ptr.c) {
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
        h_msg[0].val[0] = _mm_loadu_si128(ct_r_ptr.v + 0 + 0);
        h_msg[0].val[1] = _mm_loadu_si128(ct_r_ptr.v + 0 + 1);
        h_msg[1].val[0] = _mm_loadu_si128(ct_r_ptr.v + 2 + 0);
        h_msg[1].val[1] = _mm_loadu_si128(ct_r_ptr.v + 2 + 1);
        ct_r_ptr.v += 4;
        field_add(&evn, &h_msg[0], &(inner_keys[blck_num + 0]));
        field_add(&odd, &h_msg[1], &(inner_keys[blck_num + 1]));
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
    }
    if (ct_r_ptr.c + (HASHBLCK_SIZE) < ct_ptr.c) {
        HERE;
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }

        h_msg[0].val[0] = _mm_loadu_si128(ct_r_ptr.v + 0 + 0);
        h_msg[0].val[1] = _mm_loadu_si128(ct_r_ptr.v + 0 + 1);

        h_msg[1].val[0] = _mm_setzero_si128();
        h_msg[1].val[1] = _mm_setzero_si128();
        memcpy(&h_msg[1], ct_r_ptr.c + HASHBLCK_SIZE,
               ct_ptr.c - (ct_r_ptr.c + HASHBLCK_SIZE));
        ct_r_ptr.c = ct_ptr.c;

        field_add(&evn, &h_msg[0], &(inner_keys[blck_num + 0]));
        field_add(&odd, &h_msg[1], &(inner_keys[blck_num + 1]));
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;

    } else if ((ct_r_ptr.c < ct_ptr.c)) {
        // DEBUG_PRINTF("ct_r_ptr.c < ct_ptr\n");
        // DEBUG_PRINTF("blck_num: %zu\n", blck_num);
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }

        h_msg[0].val[0] = _mm_setzero_si128();
        h_msg[0].val[1] = _mm_setzero_si128();
        memcpy(&h_msg[0], ct_r_ptr.c, ct_ptr.c - ct_r_ptr.c);

        ct_r_ptr.c = ct_ptr.c;

        PRINT_FIELD_ELEM(h_msg[0]);
        PRINT_FIELD_ELEM(len_blck);
        PRINT_FIELD_ELEM(inner_keys[blck_num + 0]);
        PRINT_FIELD_ELEM(inner_keys[blck_num + 1]);
        field_add(&evn, &h_msg[0], &(inner_keys[blck_num + 0]));
        field_add(&odd, &len_blck, &(inner_keys[blck_num + 1]));
        PRINT_FIELD_ELEM(evn);
        PRINT_FIELD_ELEM(odd);
        DEBUG_PRINTF("blck_num: %zu\n", blck_num);
        field_mul_no_carry(&tmp, &evn, &odd);
        // PRINT_DFIELD_ELEM(tmp);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
        len_added = 1;
    }
    if (!len_added) {
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
        // PRINT_FIELD_ELEM(len_blck);
        // PRINT_DFIELD_ELEM(dacc);
        field_add_mix(&dacc, &dacc, &len_blck);
        // PRINT_DFIELD_ELEM(dacc);
    }
#ifndef __GNUC__
#pragma endregion HASH FINALIZE
#endif
    field_add_mix(&dacc, &dacc, &le_key);
    carry_round(&acc, &dacc);
    field_add(&evn, &inner_keys[0], &len);
    field_mul(&acc, &acc, &evn);
    field_add(&acc, &acc, &wc_key);
    memcpy(c + PADDED_LEN(m_len), &acc, TAG_SIZE);
    return;
}

void rijndael256x_mhp_nmh256x6(size_t m_len, const uint8_t m[static m_len],
                               size_t ad_len, const uint8_t ad[static ad_len],
                               uint8_t c[static CTXT_LEN(m_len)],
                               const uint8_t key[static KEY_SIZE],
                               const uint8_t nonce[NONCE_SIZE])
{
    size_t blck_num = 0;
    dfield_elem_t dacc = {0};
    field_elem_t acc = {0};
    dfield_elem_t tmp = {0};
    field_elem_t odd = {0};
    field_elem_t evn = {0};
    field_elem_t h_msg[2] = {0};

    field_elem_t inner_keys[SPRBLCKSIZE] = {0};
    field_elem_t outer_key = {0};
    field_elem_t le_key = {0};
    field_elem_t wc_key = {0};
    field_elem_t len = {0}; // stores the length of the hash input (including
                            // AEAD length encoding)
    field_elem_t len_blck;  // stores the AEAD length encoding

    vec_ptr m_ptr = {.v = (__m128i *)m};
    vec_ptr ad_ptr = {.v = (__m128i *)ad};
    vec_ptr ct_ptr = {.v = (__m128i *)c};
    vec_ptr ct_r_ptr = {.v = (__m128i *)c};

    __m128i store_mask;

    __m128i ct[12] = {0};
    __m128i msg[12] = {0};
    __m128i iv[6][2];

    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    int len_added = 0;
    size_t to_enc = m_len;

    size_t int_len = ad_len + PAD_LEN(ad_len) + m_len + PAD_LEN(m_len) +
                     2 * sizeof(uint64_t);
    memcpy(&len, &int_len, sizeof(size_t));
    len_blck.val[0] = _mm_set_epi64x((uint64_t)m_len, (uint64_t)ad_len);
    len_blck.val[1] = _mm_setzero_si128();

    iv[0][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5],
                                 ((uint32_t *)nonce)[6], 0}})
                   .v;
    iv[0][1] = _mm_add_epi32(iv[0][1], three.v);

    rijndael256block((__m128i *)&wc_key, roundkeys, iv[0]);
    iv[1][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[2][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[3][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[4][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[5][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[5][1] = _mm_add_epi32(iv[0][1], six.v);
    iv[4][1] = _mm_add_epi32(iv[0][1], five.v);
    iv[3][1] = _mm_add_epi32(iv[0][1], four.v);
    iv[2][1] = _mm_add_epi32(iv[0][1], three.v);
    iv[1][1] = _mm_add_epi32(iv[0][1], two.v);
    iv[0][1] = _mm_add_epi32(iv[0][1], one.v);

    __m128i input[2];
    input[0] = zero.v;
    input[1] = zero.v;
    rijndael256block((__m128i *)&inner_keys[0], roundkeys, input);
    input[1] = one.v;
    rijndael256block((__m128i *)&outer_key, roundkeys, input);
    input[1] = two.v;
    rijndael256block((__m128i *)&le_key, roundkeys, input);

    for (int j = 2; j < SPRBLCKSIZE; j += 2) {
        field_mul(&inner_keys[j], &inner_keys[j - 2], &inner_keys[0]);
    }
    field_mul(&inner_keys[SPRBLCKSIZE - 1], &inner_keys[SPRBLCKSIZE - 2],
              &inner_keys[0]);
    for (int j = SPRBLCKSIZE - 3; j > 0; j -= 2) {
        field_mul(&inner_keys[j], &inner_keys[j + 2], &inner_keys[0]);
    }
#ifndef __GNUC__
#pragma region AD
#endif
    /*PROCESS AD */
    while (ad_len >= (2 * HASHBLCK_SIZE)) {

        h_msg[0].val[0] = _mm_loadu_si128(ad_ptr.v + 0 + 0);
        h_msg[0].val[1] = _mm_loadu_si128(ad_ptr.v + 0 + 1);
        h_msg[1].val[0] = _mm_loadu_si128(ad_ptr.v + 2 + 0);
        h_msg[1].val[1] = _mm_loadu_si128(ad_ptr.v + 2 + 1);
        ad_ptr.v += 4;
        ad_len -= (2 * HASHBLCK_SIZE);
        field_add(&evn, &h_msg[0], &(inner_keys[blck_num + 0]));
        field_add(&odd, &h_msg[1], &(inner_keys[blck_num + 1]));
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
    }
    if (ad_len >= HASHBLCK_SIZE) {
        h_msg[0].val[0] = _mm_loadu_si128(ad_ptr.v);
        h_msg[0].val[1] = _mm_loadu_si128(ad_ptr.v + 1);
        h_msg[1].val[0] = _mm_setzero_si128();
        h_msg[1].val[1] = _mm_setzero_si128();
        memcpy(&h_msg[1], ad_ptr.c + HASHBLCK_SIZE, ad_len - HASHBLCK_SIZE);

        field_add(&evn, &h_msg[0], &(inner_keys[blck_num]));
        field_add(&odd, &h_msg[1], &(inner_keys[blck_num + 1]));
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
    } else if (ad_len > 0) {

        h_msg[0].val[0] = _mm_setzero_si128();
        h_msg[0].val[1] = _mm_setzero_si128();
        memcpy(&h_msg[0], ad_ptr.c, ad_len);
        field_add(&evn, &h_msg[0], &(inner_keys[blck_num]));
        // AD was uneven so we add on messageblock so that we are even again
        if (to_enc > 0) {
            // If we have some message left, encrypt and add to hash.
            if (to_enc >= BLCKSIZE) {
                msg[0] = _mm_loadu_si128(m_ptr.v);
                msg[1] = _mm_loadu_si128(m_ptr.v);
                m_ptr.v += 2;
                to_enc -= BLCKSIZE;
                rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
                _mm_storeu_si128(ct_ptr.v, ct[0]);
                _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
                ct_ptr.v += 2;
            } else if (to_enc >= 16) {
                msg[1] = _mm_setzero_si128();
                memcpy(&msg[0], m_ptr.v, to_enc);
                rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
                store_mask = _mm_cmpgt_epi8(
                    _mm_set1_epi8((unsigned char)to_enc - 16), BYTE_IDX.v);
                ct[1] = _mm_and_si128(ct[1], store_mask);
                memcpy(ct_ptr.v, &ct[0], to_enc);
                ct_ptr.c += to_enc;
                to_enc = 0;
            } else {
                msg[0] = _mm_setzero_si128();
                msg[1] = _mm_setzero_si128();
                memcpy(&msg[0], m_ptr.v, to_enc);
                rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
                store_mask = _mm_cmpgt_epi8(
                    _mm_set1_epi8((unsigned char)to_enc), BYTE_IDX.v);
                ct[0] = _mm_and_si128(ct[0], store_mask);
                memcpy(ct_ptr.v, &ct[0], to_enc);
                ct_ptr.c += to_enc;
                to_enc = 0;
            }
            if (ct_r_ptr.c + HASHBLCK_SIZE <= ct_ptr.c) {
                h_msg[1].val[0] = _mm_loadu_si128(ct_r_ptr.v);
                h_msg[1].val[1] = _mm_loadu_si128(ct_r_ptr.v + 1);
                ct_r_ptr.c += HASHBLCK_SIZE;
            } else {
                h_msg[1].val[0] = _mm_setzero_si128();
                h_msg[1].val[1] = _mm_setzero_si128();
                memcpy(&h_msg[1], ct_r_ptr.c, ct_ptr.c - ct_r_ptr.c);
                ct_r_ptr.c = ct_ptr.c;
            }
            field_add(&odd, &h_msg[1], &(inner_keys[blck_num + 1]));

            iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
            iv[1][1] = _mm_add_epi32(iv[1][1], one.v);
            iv[2][1] = _mm_add_epi32(iv[2][1], one.v);
            iv[3][1] = _mm_add_epi32(iv[3][1], one.v);
        } else {
            // We are out of message so we add the last block containing AEAD
            // length encoding
            // PRINT_FIELD_ELEM(len_blck);
            field_add(&odd, &len_blck, &(inner_keys[blck_num + 1]));
            // DEBUG_PRINTF("blck_num: %zu\n", blck_num);
            // PRINT_FIELD_ELEM(odd);
            len_added = 1;
        }
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
    }

#ifndef __GNUC__
#pragma endregion AD
#endif

#ifndef __GNUC__
#pragma region ENCRYPTION
#endif
    /* PROCESS M */
    while (to_enc >= 6 * BLCKSIZE) {
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        msg[6] = _mm_loadu_si128(m_ptr.v + 6);
        msg[7] = _mm_loadu_si128(m_ptr.v + 7);
        msg[8] = _mm_loadu_si128(m_ptr.v + 8);
        msg[9] = _mm_loadu_si128(m_ptr.v + 9);
        msg[10] = _mm_loadu_si128(m_ptr.v + 10);
        msg[11] = _mm_loadu_si128(m_ptr.v + 11);
        m_ptr.v += 12;
        to_enc -= 6 * BLCKSIZE;

        rijndael256blockXORx6(&ct[0], roundkeys, (__m128i *)iv, &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);
        _mm_storeu_si128(ct_ptr.v + 4, ct[4]);
        _mm_storeu_si128(ct_ptr.v + 5, ct[5]);
        _mm_storeu_si128(ct_ptr.v + 6, ct[6]);
        _mm_storeu_si128(ct_ptr.v + 7, ct[7]);
        _mm_storeu_si128(ct_ptr.v + 8, ct[8]);
        _mm_storeu_si128(ct_ptr.v + 9, ct[9]);
        _mm_storeu_si128(ct_ptr.v + 10, ct[10]);
        _mm_storeu_si128(ct_ptr.v + 11, ct[11]);
        iv[0][1] = _mm_add_epi32(iv[0][1], six.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], six.v);
        iv[2][1] = _mm_add_epi32(iv[2][1], six.v);
        iv[3][1] = _mm_add_epi32(iv[3][1], six.v);
        iv[4][1] = _mm_add_epi32(iv[4][1], six.v);
        iv[5][1] = _mm_add_epi32(iv[5][1], six.v);
        ct_ptr.v += 12;

        for (int i = 0; i < 2; i++) {
            h_msg[0].val[0] = _mm_loadu_si128(ct_r_ptr.v + 0 + 0);
            h_msg[0].val[1] = _mm_loadu_si128(ct_r_ptr.v + 0 + 1);
            h_msg[1].val[0] = _mm_loadu_si128(ct_r_ptr.v + 2 + 0);
            h_msg[1].val[1] = _mm_loadu_si128(ct_r_ptr.v + 2 + 1);
            ct_r_ptr.v += 4;
            field_add(&evn, &h_msg[0], &(inner_keys[blck_num + 0]));
            field_add(&odd, &h_msg[1], &(inner_keys[blck_num + 1]));
            field_mul_no_carry(&tmp, &evn, &odd);
            field_add_dbl(&dacc, &dacc, &tmp);
            blck_num += 2;
            if (blck_num == SPRBLCKSIZE) {
                carry_round(&acc, &dacc);
                field_mul_no_carry(&dacc, &acc, &outer_key);
                blck_num = 0;
            }
        }
    }
    if (to_enc >= 4 * BLCKSIZE) {
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        msg[6] = _mm_loadu_si128(m_ptr.v + 6);
        msg[7] = _mm_loadu_si128(m_ptr.v + 7);
        m_ptr.v += 2 * 4;
        to_enc -= 4 * BLCKSIZE;

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
        ct_ptr.v += 2 * 4;

        for (int i = 0; i < 2; i++) {
            h_msg[0].val[0] = _mm_loadu_si128(ct_r_ptr.v + 0 + 0);
            h_msg[0].val[1] = _mm_loadu_si128(ct_r_ptr.v + 0 + 1);
            h_msg[1].val[0] = _mm_loadu_si128(ct_r_ptr.v + 2 + 0);
            h_msg[1].val[1] = _mm_loadu_si128(ct_r_ptr.v + 2 + 1);
            ct_r_ptr.v += 4;
            field_add(&evn, &h_msg[0], &(inner_keys[blck_num + 0]));
            field_add(&odd, &h_msg[1], &(inner_keys[blck_num + 1]));
            field_mul_no_carry(&tmp, &evn, &odd);
            field_add_dbl(&dacc, &dacc, &tmp);
            blck_num += 2;
            if (blck_num == SPRBLCKSIZE) {
                carry_round(&acc, &dacc);
                field_mul_no_carry(&dacc, &acc, &outer_key);
                blck_num = 0;
            }
        }
    }
    while (to_enc >= 64) {
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        m_ptr.v += 4;
        to_enc -= 64;

        rijndael256blockXORx2(&ct[0], roundkeys, (__m128i *)iv, &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        _mm_storeu_si128(ct_ptr.v + 2, ct[2]);
        _mm_storeu_si128(ct_ptr.v + 3, ct[3]);

        iv[0][1] = _mm_add_epi32(iv[0][1], two.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], two.v);
        ct_ptr.v += 4;

        h_msg[0].val[0] = _mm_loadu_si128(ct_r_ptr.v + 0 + 0);
        h_msg[0].val[1] = _mm_loadu_si128(ct_r_ptr.v + 0 + 1);
        h_msg[1].val[0] = _mm_loadu_si128(ct_r_ptr.v + 2 + 0);
        h_msg[1].val[1] = _mm_loadu_si128(ct_r_ptr.v + 2 + 1);
        ct_r_ptr.v += 4;
        field_add(&evn, &h_msg[0], &(inner_keys[blck_num + 0]));
        field_add(&odd, &h_msg[1], &(inner_keys[blck_num + 1]));
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
    }

    if (to_enc >= BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0 + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 0 + 1);
        m_ptr.v += 2;
        to_enc -= BLCKSIZE;
        rijndael256blockXOR(&ct[0], roundkeys, iv[0], &msg[0]);
        _mm_storeu_si128(ct_ptr.v + 0, ct[0]);
        _mm_storeu_si128(ct_ptr.v + 1, ct[1]);
        iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
        iv[1][1] = _mm_add_epi32(iv[1][1], one.v);
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

/* Handle remaining hash*/
#ifndef __GNUC__
#pragma region HASH FINALIZE
#endif
    while (ct_r_ptr.c + (2 * HASHBLCK_SIZE) <= ct_ptr.c) {
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
        h_msg[0].val[0] = _mm_loadu_si128(ct_r_ptr.v + 0 + 0);
        h_msg[0].val[1] = _mm_loadu_si128(ct_r_ptr.v + 0 + 1);
        h_msg[1].val[0] = _mm_loadu_si128(ct_r_ptr.v + 2 + 0);
        h_msg[1].val[1] = _mm_loadu_si128(ct_r_ptr.v + 2 + 1);
        ct_r_ptr.v += 4;
        field_add(&evn, &h_msg[0], &(inner_keys[blck_num + 0]));
        field_add(&odd, &h_msg[1], &(inner_keys[blck_num + 1]));
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
    }
    if (ct_r_ptr.c + (HASHBLCK_SIZE) < ct_ptr.c) {
        HERE;
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }

        h_msg[0].val[0] = _mm_loadu_si128(ct_r_ptr.v + 0 + 0);
        h_msg[0].val[1] = _mm_loadu_si128(ct_r_ptr.v + 0 + 1);

        h_msg[1].val[0] = _mm_setzero_si128();
        h_msg[1].val[1] = _mm_setzero_si128();
        memcpy(&h_msg[1], ct_r_ptr.c + HASHBLCK_SIZE,
               ct_ptr.c - (ct_r_ptr.c + HASHBLCK_SIZE));
        ct_r_ptr.c = ct_ptr.c;

        field_add(&evn, &h_msg[0], &(inner_keys[blck_num + 0]));
        field_add(&odd, &h_msg[1], &(inner_keys[blck_num + 1]));
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;

    } else if ((ct_r_ptr.c < ct_ptr.c)) {
        // DEBUG_PRINTF("ct_r_ptr.c < ct_ptr\n");
        // DEBUG_PRINTF("blck_num: %zu\n", blck_num);
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }

        h_msg[0].val[0] = _mm_setzero_si128();
        h_msg[0].val[1] = _mm_setzero_si128();
        memcpy(&h_msg[0], ct_r_ptr.c, ct_ptr.c - ct_r_ptr.c);

        ct_r_ptr.c = ct_ptr.c;

        PRINT_FIELD_ELEM(h_msg[0]);
        PRINT_FIELD_ELEM(len_blck);
        PRINT_FIELD_ELEM(inner_keys[blck_num + 0]);
        PRINT_FIELD_ELEM(inner_keys[blck_num + 1]);
        field_add(&evn, &h_msg[0], &(inner_keys[blck_num + 0]));
        field_add(&odd, &len_blck, &(inner_keys[blck_num + 1]));
        PRINT_FIELD_ELEM(evn);
        PRINT_FIELD_ELEM(odd);
        DEBUG_PRINTF("blck_num: %zu\n", blck_num);
        field_mul_no_carry(&tmp, &evn, &odd);
        // PRINT_DFIELD_ELEM(tmp);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
        len_added = 1;
    }
    if (!len_added) {
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
        // PRINT_FIELD_ELEM(len_blck);
        // PRINT_DFIELD_ELEM(dacc);
        field_add_mix(&dacc, &dacc, &len_blck);
        // PRINT_DFIELD_ELEM(dacc);
    }
#ifndef __GNUC__
#pragma endregion HASH FINALIZE
#endif
    field_add_mix(&dacc, &dacc, &le_key);
    carry_round(&acc, &dacc);
    field_add(&evn, &inner_keys[0], &len);
    field_mul(&acc, &acc, &evn);
    field_add(&acc, &acc, &wc_key);
    memcpy(c + PADDED_LEN(m_len), &acc, TAG_SIZE);
    return;
}
