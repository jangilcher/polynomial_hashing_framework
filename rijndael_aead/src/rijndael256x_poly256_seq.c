// #define DEBUG 1
#include <assert.h>
#include <immintrin.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "fieldArith/bf_arithmetic_256_64_64_64_64_64_schoolbook.h"
#include "fieldArith/fieldArith.h"
#include "rijndael256/rijndael256.h"
#include "rijndael256/rijndael256x_poly256_seq.h"
#include "rijndael256_impl.h"

typedef union {
    __m256i *lv;
    __m128i *v;
    __uint8_t *c;
} vec_ptr;

const vector BYTE_IDX = {
    .i8 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}};

#define SPRBLCKSIZE 36
#define CARRYPROPCYCLE 2
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

void rijndael256x4x_poly256_seq(size_t m_len, const uint8_t m[static m_len],
                                size_t ad_len, const uint8_t ad[static ad_len],
                                uint8_t c[static CTXT_LEN(m_len)],
                                const uint8_t key[static KEY_SIZE],
                                const uint8_t nonce[NONCE_SIZE])
{
    vector key_buff[4] = {0};
    __m128i store_mask;

    vec_ptr m_ptr = {.v = (__m128i *)m};
    vec_ptr ad_ptr = {.v = (__m128i *)ad};
    vec_ptr ct_ptr = {.v = (__m128i *)c};
    vec_ptr ct_r_ptr = {.v = (__m128i *)c};
    __m128i ct[8] = {0};
    __m128i msg[8] = {0};
    __m128i iv[4][2];
    size_t to_enc = m_len;
    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    iv[0][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5],
                                 ((uint32_t *)nonce)[6], 0}})
                   .v;
    iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
    rijndael256block(&key_buff[2].v, roundkeys, iv[0]);
    iv[1][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[2][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[3][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[3][1] = _mm_add_epi32(iv[0][1], four.v);
    iv[2][1] = _mm_add_epi32(iv[0][1], three.v);
    iv[1][1] = _mm_add_epi32(iv[0][1], two.v);
    iv[0][1] = _mm_add_epi32(iv[0][1], one.v);

    __m128i input[2];
    input[0] = zero.v;
    input[1] = zero.v;
    rijndael256block(&key_buff[0].v, roundkeys, input);
    DEBUG_PRINTF("keybuff\n");
    for (size_t i = 0; i < 6 * 16; i++) {
        if (i > 0 && (i % 32 == 0))
            DEBUG_PRINTF("\n");
        DEBUG_PRINTF("%02" PRIx8 "", ((uint8_t *)key_buff[0].i8)[i]);
    }
    DEBUG_PRINTF("\n");

    // Hash Variables
    field_elem_t wc_key;
    field_elem_t hash_key[1] = {0};
    field_elem_t h_msg[1] = {0};
    dfield_elem_t dacc[2] = {0};
    field_elem_t acc = {0};
    field_elem_t len = {0}; // stores the length of the hash input (including
                            // AEAD length encoding)
    size_t int_len =
        PADDED_LEN(ad_len) + PADDED_LEN(m_len) + 2 * AEAD_LEN_BYTES;
    memcpy(&len, &int_len, sizeof(size_t));
    field_elem_t len_blck; // stores the AEAD length encoding
    // auth_keys_t keys = {0};
    uint8_t len_buff[BUFFSIZE] = {0};
    memcpy(len_buff, &ad_len, AEAD_LEN_BYTES);
    memcpy(len_buff + AEAD_LEN_BYTES, &m_len, AEAD_LEN_BYTES);

    unpack_field_elem(&len_blck, len_buff);
    memset(c + m_len, 0, PAD_LEN(m_len));
    // memset(key_buff, 0, sizeof(key_buff));
    // key_buff[0].i8[0] = 0x0f;
    // *(key_buff[0].i8 + 28) = 1;
    // *(key_buff[0].i8 + 56) = 1;
    // key_buff[6].i8[0] = 1;
    unpack_and_encode_key(&hash_key[0], key_buff[0].i8 + 0);
    unpack_and_encode_key(&wc_key, key_buff[2].i8);

    for (int j = 1; j < 1; j++) {
        field_mul(&hash_key[j], &hash_key[j - 1], &hash_key[0]);
    }

    PRINT_FIELD_ELEM(hash_key[0]);
    PRINT_FIELD_ELEM(wc_key);
#ifndef __GNUC__
#pragma region ENCRYPTION
#endif
    /* PROCESS M */
    while (to_enc >= 4 * BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        msg[6] = _mm_loadu_si128(m_ptr.v + 6);
        msg[7] = _mm_loadu_si128(m_ptr.v + 7);
        m_ptr.v += 8;
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
        ct_ptr.v += 8;
    }
    if (to_enc >= 3 * BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        m_ptr.v += 6;
        to_enc -= 3 * BLCKSIZE;
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
    if (to_enc >= 2 * BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        m_ptr.v += 4;
        to_enc -= 2 * BLCKSIZE;
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

#ifndef __GNUC__
#pragma region HASH
#endif
    while (ad_len >= HASHBLCK_SIZE) {
        unpack_and_encode_field_elem(&h_msg[0], ad_ptr.c);
        ad_ptr.c += HASHBLCK_SIZE;
        ad_len -= HASHBLCK_SIZE;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len > 0) {
        uint8_t buff[BUFFSIZE] = {0};
        memcpy(buff, ad_ptr.c, ad_len);
        unpack_and_encode_field_elem(&h_msg[0], buff);
        ad_ptr.c += ad_len;
        ad_len -= ad_len;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }

    while (ct_r_ptr.c + HASHBLCK_SIZE <= ct_ptr.c) {
        unpack_and_encode_field_elem(&h_msg[0], ct_r_ptr.c);
        ct_r_ptr.c += HASHBLCK_SIZE;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ct_r_ptr.c < ct_ptr.c) {
        uint8_t buff[BUFFSIZE] = {0};
        memcpy(buff, ct_r_ptr.c, ct_ptr.c - ct_r_ptr.c);
        unpack_and_encode_field_elem(&h_msg[0], buff);
        ct_r_ptr.c = ct_ptr.c;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
#ifndef __GNUC__
#pragma endregion HASH
#endif

    PRINT_FIELD_ELEM(acc);
    field_mul_no_carry(&dacc[0], &len_blck, &hash_key[0]);
    field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
    field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
    carry_round(&acc, &dacc[0]);
    field_add(&acc, &acc, &len);
    field_mul(&acc, &acc, &hash_key[0]);
    PRINT_FIELD_ELEM(acc);
    field_add(&acc, &acc, &wc_key);
    _carry_round(&acc, &acc);
    PRINT_FIELD_ELEM(acc);
    reduce(&acc, &acc);
    PRINT_FIELD_ELEM(acc);
    uint8_t buff[BUFFSIZE] = {0};
    pack_field_elem(buff, &acc);
    memcpy(c + PADDED_LEN(m_len), buff, TAG_SIZE);
    return;
}

void rijndael256x4x_poly256_seqx2(size_t m_len, const uint8_t m[static m_len],
                                  size_t ad_len,
                                  const uint8_t ad[static ad_len],
                                  uint8_t c[static CTXT_LEN(m_len)],
                                  const uint8_t key[static KEY_SIZE],
                                  const uint8_t nonce[NONCE_SIZE])
{
    vector key_buff[4] = {0};
    __m128i store_mask;

    vec_ptr m_ptr = {.v = (__m128i *)m};
    vec_ptr ad_ptr = {.v = (__m128i *)ad};
    vec_ptr ct_ptr = {.v = (__m128i *)c};
    vec_ptr ct_r_ptr = {.v = (__m128i *)c};
    __m128i ct[8] = {0};
    __m128i msg[8] = {0};
    __m128i iv[4][2];
    size_t to_enc = m_len;
    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    iv[0][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5],
                                 ((uint32_t *)nonce)[6], 0}})
                   .v;
    iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
    rijndael256block(&key_buff[2].v, roundkeys, iv[0]);
    iv[1][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[2][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[3][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[3][1] = _mm_add_epi32(iv[0][1], four.v);
    iv[2][1] = _mm_add_epi32(iv[0][1], three.v);
    iv[1][1] = _mm_add_epi32(iv[0][1], two.v);
    iv[0][1] = _mm_add_epi32(iv[0][1], one.v);

    __m128i input[2];
    input[0] = zero.v;
    input[1] = zero.v;
    rijndael256block(&key_buff[0].v, roundkeys, input);
    DEBUG_PRINTF("keybuff\n");
    for (size_t i = 0; i < 6 * 16; i++) {
        if (i > 0 && (i % 32 == 0))
            DEBUG_PRINTF("\n");
        DEBUG_PRINTF("%02" PRIx8 "", ((uint8_t *)key_buff[0].i8)[i]);
    }
    DEBUG_PRINTF("\n");

    // Hash Variables
    field_elem_t wc_key;
    field_elem_t hash_key[2] = {0};
    field_elem_t h_msg[2] = {0};
    dfield_elem_t dacc[2] = {0};
    field_elem_t acc = {0};
    field_elem_t len = {0}; // stores the length of the hash input (including
                            // AEAD length encoding)
    size_t int_len =
        PADDED_LEN(ad_len) + PADDED_LEN(m_len) + 2 * AEAD_LEN_BYTES;
    memcpy(&len, &int_len, sizeof(size_t));
    field_elem_t len_blck; // stores the AEAD length encoding
    // auth_keys_t keys = {0};
    uint8_t len_buff[BUFFSIZE] = {0};
    memcpy(len_buff, &ad_len, AEAD_LEN_BYTES);
    memcpy(len_buff + AEAD_LEN_BYTES, &m_len, AEAD_LEN_BYTES);

    unpack_field_elem(&len_blck, len_buff);
    memset(c + m_len, 0, PAD_LEN(m_len));
    // memset(key_buff, 0, sizeof(key_buff));
    // key_buff[0].i8[0] = 0x0f;
    // *(key_buff[0].i8 + 28) = 1;
    // *(key_buff[0].i8 + 56) = 1;
    // key_buff[6].i8[0] = 1;
    unpack_and_encode_key(&hash_key[0], key_buff[0].i8 + 0);
    unpack_and_encode_key(&wc_key, key_buff[2].i8);

    for (int j = 1; j < 2; j++) {
        field_mul(&hash_key[j], &hash_key[j - 1], &hash_key[0]);
    }

    PRINT_FIELD_ELEM(hash_key[0]);
    PRINT_FIELD_ELEM(wc_key);
#ifndef __GNUC__
#pragma region ENCRYPTION
#endif
    /* PROCESS M */
    while (to_enc >= 4 * BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        msg[6] = _mm_loadu_si128(m_ptr.v + 6);
        msg[7] = _mm_loadu_si128(m_ptr.v + 7);
        m_ptr.v += 8;
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
        ct_ptr.v += 8;
    }
    if (to_enc >= 3 * BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        m_ptr.v += 6;
        to_enc -= 3 * BLCKSIZE;
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
    if (to_enc >= 2 * BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        m_ptr.v += 4;
        to_enc -= 2 * BLCKSIZE;
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

#ifndef __GNUC__
#pragma region HASH
#endif
    while (ad_len >= (2 * HASHBLCK_SIZE)) {
        unpack_and_encode_field_elem(&h_msg[0], ad_ptr.c + 0 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[1], ad_ptr.c + 1 * HASHBLCK_SIZE);
        ad_ptr.c += (2 * HASHBLCK_SIZE);
        ad_len -= (2 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[1]);
        field_mul_no_carry(&dacc[1], &h_msg[1], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[1]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len >= HASHBLCK_SIZE) {
        unpack_and_encode_field_elem(&h_msg[0], ad_ptr.c);
        ad_ptr.c += HASHBLCK_SIZE;
        ad_len -= HASHBLCK_SIZE;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len > 0) {
        uint8_t buff[BUFFSIZE] = {0};
        memcpy(buff, ad_ptr.c, ad_len);
        unpack_and_encode_field_elem(&h_msg[0], buff);
        ad_ptr.c += ad_len;
        ad_len -= ad_len;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }

    while (ct_r_ptr.c + (2 * HASHBLCK_SIZE) <= ct_ptr.c) {
        unpack_and_encode_field_elem(&h_msg[0], ct_r_ptr.c + 0 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[1], ct_r_ptr.c + 1 * HASHBLCK_SIZE);
        ct_r_ptr.c += (2 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[1]);
        field_mul_no_carry(&dacc[1], &h_msg[1], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[1]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ct_r_ptr.c + HASHBLCK_SIZE <= ct_ptr.c) {
        unpack_and_encode_field_elem(&h_msg[0], ct_r_ptr.c);
        ct_r_ptr.c += HASHBLCK_SIZE;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ct_r_ptr.c < ct_ptr.c) {
        uint8_t buff[BUFFSIZE] = {0};
        memcpy(buff, ct_r_ptr.c, ct_ptr.c - ct_r_ptr.c);
        unpack_and_encode_field_elem(&h_msg[0], buff);
        ct_r_ptr.c = ct_ptr.c;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
#ifndef __GNUC__
#pragma endregion HASH
#endif

    PRINT_FIELD_ELEM(acc);
    field_mul_no_carry(&dacc[0], &len_blck, &hash_key[0]);
    field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
    field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
    carry_round(&acc, &dacc[0]);
    field_add(&acc, &acc, &len);
    field_mul(&acc, &acc, &hash_key[0]);
    PRINT_FIELD_ELEM(acc);
    field_add(&acc, &acc, &wc_key);
    _carry_round(&acc, &acc);
    PRINT_FIELD_ELEM(acc);
    reduce(&acc, &acc);
    PRINT_FIELD_ELEM(acc);
    uint8_t buff[BUFFSIZE] = {0};
    pack_field_elem(buff, &acc);
    memcpy(c + PADDED_LEN(m_len), buff, TAG_SIZE);
    return;
}

void rijndael256x4x_poly256_seqx4(size_t m_len, const uint8_t m[static m_len],
                                  size_t ad_len,
                                  const uint8_t ad[static ad_len],
                                  uint8_t c[static CTXT_LEN(m_len)],
                                  const uint8_t key[static KEY_SIZE],
                                  const uint8_t nonce[NONCE_SIZE])
{
    vector key_buff[4] = {0};
    __m128i store_mask;

    vec_ptr m_ptr = {.v = (__m128i *)m};
    vec_ptr ad_ptr = {.v = (__m128i *)ad};
    vec_ptr ct_ptr = {.v = (__m128i *)c};
    vec_ptr ct_r_ptr = {.v = (__m128i *)c};
    __m128i ct[8] = {0};
    __m128i msg[8] = {0};
    __m128i iv[4][2];
    size_t to_enc = m_len;
    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    iv[0][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5],
                                 ((uint32_t *)nonce)[6], 0}})
                   .v;
    iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
    rijndael256block(&key_buff[2].v, roundkeys, iv[0]);
    iv[1][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[2][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[3][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[3][1] = _mm_add_epi32(iv[0][1], four.v);
    iv[2][1] = _mm_add_epi32(iv[0][1], three.v);
    iv[1][1] = _mm_add_epi32(iv[0][1], two.v);
    iv[0][1] = _mm_add_epi32(iv[0][1], one.v);

    __m128i input[2];
    input[0] = zero.v;
    input[1] = zero.v;
    rijndael256block(&key_buff[0].v, roundkeys, input);
    DEBUG_PRINTF("keybuff\n");
    for (size_t i = 0; i < 6 * 16; i++) {
        if (i > 0 && (i % 32 == 0))
            DEBUG_PRINTF("\n");
        DEBUG_PRINTF("%02" PRIx8 "", ((uint8_t *)key_buff[0].i8)[i]);
    }
    DEBUG_PRINTF("\n");

    // Hash Variables
    field_elem_t wc_key;
    field_elem_t hash_key[4] = {0};
    field_elem_t h_msg[4] = {0};
    dfield_elem_t dacc[4] = {0};
    field_elem_t acc = {0};
    field_elem_t len = {0}; // stores the length of the hash input (including
                            // AEAD length encoding)
    size_t int_len =
        PADDED_LEN(ad_len) + PADDED_LEN(m_len) + 2 * AEAD_LEN_BYTES;
    memcpy(&len, &int_len, sizeof(size_t));
    field_elem_t len_blck; // stores the AEAD length encoding
    // auth_keys_t keys = {0};
    uint8_t len_buff[BUFFSIZE] = {0};
    memcpy(len_buff, &ad_len, AEAD_LEN_BYTES);
    memcpy(len_buff + AEAD_LEN_BYTES, &m_len, AEAD_LEN_BYTES);

    unpack_field_elem(&len_blck, len_buff);
    memset(c + m_len, 0, PAD_LEN(m_len));
    // memset(key_buff, 0, sizeof(key_buff));
    // key_buff[0].i8[0] = 0x0f;
    // *(key_buff[0].i8 + 28) = 1;
    // *(key_buff[0].i8 + 56) = 1;
    // key_buff[6].i8[0] = 1;
    unpack_and_encode_key(&hash_key[0], key_buff[0].i8 + 0);
    unpack_and_encode_key(&wc_key, key_buff[2].i8);

    for (int j = 1; j < 4; j++) {
        field_mul(&hash_key[j], &hash_key[j - 1], &hash_key[0]);
    }

    PRINT_FIELD_ELEM(hash_key[0]);
    PRINT_FIELD_ELEM(wc_key);
#ifndef __GNUC__
#pragma region ENCRYPTION
#endif
    /* PROCESS M */
    while (to_enc >= 4 * BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        msg[6] = _mm_loadu_si128(m_ptr.v + 6);
        msg[7] = _mm_loadu_si128(m_ptr.v + 7);
        m_ptr.v += 8;
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
        ct_ptr.v += 8;
    }
    if (to_enc >= 3 * BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        m_ptr.v += 6;
        to_enc -= 3 * BLCKSIZE;
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
    if (to_enc >= 2 * BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        m_ptr.v += 4;
        to_enc -= 2 * BLCKSIZE;
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

#ifndef __GNUC__
#pragma region HASH
#endif
    while (ad_len >= (4 * HASHBLCK_SIZE)) {
        unpack_and_encode_field_elem(&h_msg[0], ad_ptr.c + 0 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[1], ad_ptr.c + 1 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[2], ad_ptr.c + 2 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[3], ad_ptr.c + 3 * HASHBLCK_SIZE);
        ad_ptr.c += (4 * HASHBLCK_SIZE);
        ad_len -= (4 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[3]);
        field_mul_no_carry(&dacc[1], &h_msg[1], &hash_key[2]);
        field_mul_no_carry(&dacc[2], &h_msg[2], &hash_key[1]);
        field_mul_no_carry(&dacc[3], &h_msg[3], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_add_dbl(&dacc[2], &dacc[3], &dacc[2]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[2]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[3]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len >= (2 * HASHBLCK_SIZE)) {
        unpack_and_encode_field_elem(&h_msg[0], ad_ptr.c + 0 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[1], ad_ptr.c + 1 * HASHBLCK_SIZE);
        ad_ptr.c += (2 * HASHBLCK_SIZE);
        ad_len -= (2 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[1]);
        field_mul_no_carry(&dacc[1], &h_msg[1], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[1]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len >= HASHBLCK_SIZE) {
        unpack_and_encode_field_elem(&h_msg[0], ad_ptr.c);
        ad_ptr.c += HASHBLCK_SIZE;
        ad_len -= HASHBLCK_SIZE;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len > 0) {
        uint8_t buff[BUFFSIZE] = {0};
        memcpy(buff, ad_ptr.c, ad_len);
        unpack_and_encode_field_elem(&h_msg[0], buff);
        ad_ptr.c += ad_len;
        ad_len -= ad_len;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }

    while (ct_r_ptr.c + (4 * HASHBLCK_SIZE) <= ct_ptr.c) {
        unpack_and_encode_field_elem(&h_msg[0], ct_r_ptr.c + 0 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[1], ct_r_ptr.c + 1 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[2], ct_r_ptr.c + 2 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[3], ct_r_ptr.c + 3 * HASHBLCK_SIZE);
        ct_r_ptr.c += (4 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[3]);
        field_mul_no_carry(&dacc[1], &h_msg[1], &hash_key[2]);
        field_mul_no_carry(&dacc[2], &h_msg[2], &hash_key[1]);
        field_mul_no_carry(&dacc[3], &h_msg[3], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_add_dbl(&dacc[2], &dacc[3], &dacc[2]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[2]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[3]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ct_r_ptr.c + (2 * HASHBLCK_SIZE) <= ct_ptr.c) {
        unpack_and_encode_field_elem(&h_msg[0], ct_r_ptr.c + 0 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[1], ct_r_ptr.c + 1 * HASHBLCK_SIZE);
        ct_r_ptr.c += (2 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[1]);
        field_mul_no_carry(&dacc[1], &h_msg[1], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[1]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ct_r_ptr.c + HASHBLCK_SIZE <= ct_ptr.c) {
        unpack_and_encode_field_elem(&h_msg[0], ct_r_ptr.c);
        ct_r_ptr.c += HASHBLCK_SIZE;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ct_r_ptr.c < ct_ptr.c) {
        uint8_t buff[BUFFSIZE] = {0};
        memcpy(buff, ct_r_ptr.c, ct_ptr.c - ct_r_ptr.c);
        unpack_and_encode_field_elem(&h_msg[0], buff);
        ct_r_ptr.c = ct_ptr.c;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
#ifndef __GNUC__
#pragma endregion HASH
#endif

    PRINT_FIELD_ELEM(acc);
    field_mul_no_carry(&dacc[0], &len_blck, &hash_key[0]);
    field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
    field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
    carry_round(&acc, &dacc[0]);
    field_add(&acc, &acc, &len);
    field_mul(&acc, &acc, &hash_key[0]);
    PRINT_FIELD_ELEM(acc);
    field_add(&acc, &acc, &wc_key);
    _carry_round(&acc, &acc);
    PRINT_FIELD_ELEM(acc);
    reduce(&acc, &acc);
    PRINT_FIELD_ELEM(acc);
    uint8_t buff[BUFFSIZE] = {0};
    pack_field_elem(buff, &acc);
    memcpy(c + PADDED_LEN(m_len), buff, TAG_SIZE);
    return;
}

void rijndael256x4x_poly256_seqx8(size_t m_len, const uint8_t m[static m_len],
                                  size_t ad_len,
                                  const uint8_t ad[static ad_len],
                                  uint8_t c[static CTXT_LEN(m_len)],
                                  const uint8_t key[static KEY_SIZE],
                                  const uint8_t nonce[NONCE_SIZE])
{
    vector key_buff[4] = {0};
    __m128i store_mask;

    vec_ptr m_ptr = {.v = (__m128i *)m};
    vec_ptr ad_ptr = {.v = (__m128i *)ad};
    vec_ptr ct_ptr = {.v = (__m128i *)c};
    vec_ptr ct_r_ptr = {.v = (__m128i *)c};
    __m128i ct[8] = {0};
    __m128i msg[8] = {0};
    __m128i iv[4][2];
    size_t to_enc = m_len;
    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    iv[0][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5],
                                 ((uint32_t *)nonce)[6], 0}})
                   .v;
    iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
    rijndael256block(&key_buff[2].v, roundkeys, iv[0]);
    iv[1][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[2][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[3][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[3][1] = _mm_add_epi32(iv[0][1], four.v);
    iv[2][1] = _mm_add_epi32(iv[0][1], three.v);
    iv[1][1] = _mm_add_epi32(iv[0][1], two.v);
    iv[0][1] = _mm_add_epi32(iv[0][1], one.v);

    __m128i input[2];
    input[0] = zero.v;
    input[1] = zero.v;
    rijndael256block(&key_buff[0].v, roundkeys, input);
    DEBUG_PRINTF("keybuff\n");
    for (size_t i = 0; i < 6 * 16; i++) {
        if (i > 0 && (i % 32 == 0))
            DEBUG_PRINTF("\n");
        DEBUG_PRINTF("%02" PRIx8 "", ((uint8_t *)key_buff[0].i8)[i]);
    }
    DEBUG_PRINTF("\n");

    // Hash Variables
    field_elem_t wc_key;
    field_elem_t hash_key[8] = {0};
    field_elem_t h_msg[8] = {0};
    dfield_elem_t dacc[8] = {0};
    field_elem_t acc = {0};
    field_elem_t len = {0}; // stores the length of the hash input (including
                            // AEAD length encoding)
    size_t int_len =
        PADDED_LEN(ad_len) + PADDED_LEN(m_len) + 2 * AEAD_LEN_BYTES;
    memcpy(&len, &int_len, sizeof(size_t));
    field_elem_t len_blck; // stores the AEAD length encoding
    // auth_keys_t keys = {0};
    uint8_t len_buff[BUFFSIZE] = {0};
    memcpy(len_buff, &ad_len, AEAD_LEN_BYTES);
    memcpy(len_buff + AEAD_LEN_BYTES, &m_len, AEAD_LEN_BYTES);

    unpack_field_elem(&len_blck, len_buff);
    memset(c + m_len, 0, PAD_LEN(m_len));
    // memset(key_buff, 0, sizeof(key_buff));
    // key_buff[0].i8[0] = 0x0f;
    // *(key_buff[0].i8 + 28) = 1;
    // *(key_buff[0].i8 + 56) = 1;
    // key_buff[6].i8[0] = 1;
    unpack_and_encode_key(&hash_key[0], key_buff[0].i8 + 0);
    unpack_and_encode_key(&wc_key, key_buff[2].i8);

    for (int j = 1; j < 8; j++) {
        field_mul(&hash_key[j], &hash_key[j - 1], &hash_key[0]);
    }

    PRINT_FIELD_ELEM(hash_key[0]);
    PRINT_FIELD_ELEM(wc_key);
#ifndef __GNUC__
#pragma region ENCRYPTION
#endif
    /* PROCESS M */
    while (to_enc >= 4 * BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        msg[6] = _mm_loadu_si128(m_ptr.v + 6);
        msg[7] = _mm_loadu_si128(m_ptr.v + 7);
        m_ptr.v += 8;
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
        ct_ptr.v += 8;
    }
    if (to_enc >= 3 * BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        m_ptr.v += 6;
        to_enc -= 3 * BLCKSIZE;
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
    if (to_enc >= 2 * BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        m_ptr.v += 4;
        to_enc -= 2 * BLCKSIZE;
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

#ifndef __GNUC__
#pragma region HASH
#endif
    while (ad_len >= (8 * HASHBLCK_SIZE)) {
        unpack_and_encode_field_elem(&h_msg[0], ad_ptr.c + 0 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[1], ad_ptr.c + 1 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[2], ad_ptr.c + 2 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[3], ad_ptr.c + 3 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[4], ad_ptr.c + 4 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[5], ad_ptr.c + 5 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[6], ad_ptr.c + 6 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[7], ad_ptr.c + 7 * HASHBLCK_SIZE);
        ad_ptr.c += (8 * HASHBLCK_SIZE);
        ad_len -= (8 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[7]);
        field_mul_no_carry(&dacc[1], &h_msg[1], &hash_key[6]);
        field_mul_no_carry(&dacc[2], &h_msg[2], &hash_key[5]);
        field_mul_no_carry(&dacc[3], &h_msg[3], &hash_key[4]);
        field_mul_no_carry(&dacc[4], &h_msg[4], &hash_key[3]);
        field_mul_no_carry(&dacc[5], &h_msg[5], &hash_key[2]);
        field_mul_no_carry(&dacc[6], &h_msg[6], &hash_key[1]);
        field_mul_no_carry(&dacc[7], &h_msg[7], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_add_dbl(&dacc[2], &dacc[3], &dacc[2]);
        field_add_dbl(&dacc[4], &dacc[5], &dacc[4]);
        field_add_dbl(&dacc[6], &dacc[7], &dacc[6]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[2]);
        field_add_dbl(&dacc[4], &dacc[4], &dacc[6]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[4]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[7]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len >= (4 * HASHBLCK_SIZE)) {
        unpack_and_encode_field_elem(&h_msg[0], ad_ptr.c + 0 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[1], ad_ptr.c + 1 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[2], ad_ptr.c + 2 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[3], ad_ptr.c + 3 * HASHBLCK_SIZE);
        ad_ptr.c += (4 * HASHBLCK_SIZE);
        ad_len -= (4 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[3]);
        field_mul_no_carry(&dacc[1], &h_msg[1], &hash_key[2]);
        field_mul_no_carry(&dacc[2], &h_msg[2], &hash_key[1]);
        field_mul_no_carry(&dacc[3], &h_msg[3], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_add_dbl(&dacc[2], &dacc[3], &dacc[2]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[2]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[3]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len >= (2 * HASHBLCK_SIZE)) {
        unpack_and_encode_field_elem(&h_msg[0], ad_ptr.c + 0 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[1], ad_ptr.c + 1 * HASHBLCK_SIZE);
        ad_ptr.c += (2 * HASHBLCK_SIZE);
        ad_len -= (2 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[1]);
        field_mul_no_carry(&dacc[1], &h_msg[1], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[1]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len >= HASHBLCK_SIZE) {
        unpack_and_encode_field_elem(&h_msg[0], ad_ptr.c);
        ad_ptr.c += HASHBLCK_SIZE;
        ad_len -= HASHBLCK_SIZE;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len > 0) {
        uint8_t buff[BUFFSIZE] = {0};
        memcpy(buff, ad_ptr.c, ad_len);
        unpack_and_encode_field_elem(&h_msg[0], buff);
        ad_ptr.c += ad_len;
        ad_len -= ad_len;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }

    while (ct_r_ptr.c + (8 * HASHBLCK_SIZE) <= ct_ptr.c) {
        unpack_and_encode_field_elem(&h_msg[0], ct_r_ptr.c + 0 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[1], ct_r_ptr.c + 1 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[2], ct_r_ptr.c + 2 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[3], ct_r_ptr.c + 3 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[4], ct_r_ptr.c + 4 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[5], ct_r_ptr.c + 5 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[6], ct_r_ptr.c + 6 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[7], ct_r_ptr.c + 7 * HASHBLCK_SIZE);
        ct_r_ptr.c += (8 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[7]);
        field_mul_no_carry(&dacc[1], &h_msg[1], &hash_key[6]);
        field_mul_no_carry(&dacc[2], &h_msg[2], &hash_key[5]);
        field_mul_no_carry(&dacc[3], &h_msg[3], &hash_key[4]);
        field_mul_no_carry(&dacc[4], &h_msg[4], &hash_key[3]);
        field_mul_no_carry(&dacc[5], &h_msg[5], &hash_key[2]);
        field_mul_no_carry(&dacc[6], &h_msg[6], &hash_key[1]);
        field_mul_no_carry(&dacc[7], &h_msg[7], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_add_dbl(&dacc[2], &dacc[3], &dacc[2]);
        field_add_dbl(&dacc[4], &dacc[5], &dacc[4]);
        field_add_dbl(&dacc[6], &dacc[7], &dacc[6]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[2]);
        field_add_dbl(&dacc[4], &dacc[4], &dacc[6]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[4]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[7]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ct_r_ptr.c + (4 * HASHBLCK_SIZE) <= ct_ptr.c) {
        unpack_and_encode_field_elem(&h_msg[0], ct_r_ptr.c + 0 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[1], ct_r_ptr.c + 1 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[2], ct_r_ptr.c + 2 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[3], ct_r_ptr.c + 3 * HASHBLCK_SIZE);
        ct_r_ptr.c += (4 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[3]);
        field_mul_no_carry(&dacc[1], &h_msg[1], &hash_key[2]);
        field_mul_no_carry(&dacc[2], &h_msg[2], &hash_key[1]);
        field_mul_no_carry(&dacc[3], &h_msg[3], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_add_dbl(&dacc[2], &dacc[3], &dacc[2]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[2]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[3]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ct_r_ptr.c + (2 * HASHBLCK_SIZE) <= ct_ptr.c) {
        unpack_and_encode_field_elem(&h_msg[0], ct_r_ptr.c + 0 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[1], ct_r_ptr.c + 1 * HASHBLCK_SIZE);
        ct_r_ptr.c += (2 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[1]);
        field_mul_no_carry(&dacc[1], &h_msg[1], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[1]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ct_r_ptr.c + HASHBLCK_SIZE <= ct_ptr.c) {
        unpack_and_encode_field_elem(&h_msg[0], ct_r_ptr.c);
        ct_r_ptr.c += HASHBLCK_SIZE;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ct_r_ptr.c < ct_ptr.c) {
        uint8_t buff[BUFFSIZE] = {0};
        memcpy(buff, ct_r_ptr.c, ct_ptr.c - ct_r_ptr.c);
        unpack_and_encode_field_elem(&h_msg[0], buff);
        ct_r_ptr.c = ct_ptr.c;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
#ifndef __GNUC__
#pragma endregion HASH
#endif

    PRINT_FIELD_ELEM(acc);
    field_mul_no_carry(&dacc[0], &len_blck, &hash_key[0]);
    field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
    field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
    carry_round(&acc, &dacc[0]);
    field_add(&acc, &acc, &len);
    field_mul(&acc, &acc, &hash_key[0]);
    PRINT_FIELD_ELEM(acc);
    field_add(&acc, &acc, &wc_key);
    _carry_round(&acc, &acc);
    PRINT_FIELD_ELEM(acc);
    reduce(&acc, &acc);
    PRINT_FIELD_ELEM(acc);
    uint8_t buff[BUFFSIZE] = {0};
    pack_field_elem(buff, &acc);
    memcpy(c + PADDED_LEN(m_len), buff, TAG_SIZE);
    return;
}

void rijndael256x6x_poly256_seq(size_t m_len, const uint8_t m[static m_len],
                                size_t ad_len, const uint8_t ad[static ad_len],
                                uint8_t c[static CTXT_LEN(m_len)],
                                const uint8_t key[static KEY_SIZE],
                                const uint8_t nonce[NONCE_SIZE])
{
    vector key_buff[4] = {0};
    __m128i store_mask;

    vec_ptr m_ptr = {.v = (__m128i *)m};
    vec_ptr ad_ptr = {.v = (__m128i *)ad};
    vec_ptr ct_ptr = {.v = (__m128i *)c};
    vec_ptr ct_r_ptr = {.v = (__m128i *)c};
    __m128i ct[12] = {0};
    __m128i msg[12] = {0};
    __m128i iv[6][2];
    size_t to_enc = m_len;
    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    iv[0][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5],
                                 ((uint32_t *)nonce)[6], 0}})
                   .v;
    iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
    rijndael256block(&key_buff[2].v, roundkeys, iv[0]);
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
    rijndael256block(&key_buff[0].v, roundkeys, input);
    DEBUG_PRINTF("keybuff\n");
    for (size_t i = 0; i < 6 * 16; i++) {
        if (i > 0 && (i % 32 == 0))
            DEBUG_PRINTF("\n");
        DEBUG_PRINTF("%02" PRIx8 "", ((uint8_t *)key_buff[0].i8)[i]);
    }
    DEBUG_PRINTF("\n");

    // Hash Variables
    field_elem_t wc_key;
    field_elem_t hash_key[1] = {0};
    field_elem_t h_msg[1] = {0};
    dfield_elem_t dacc[2] = {0};
    field_elem_t acc = {0};
    field_elem_t len = {0}; // stores the length of the hash input (including
                            // AEAD length encoding)
    size_t int_len =
        PADDED_LEN(ad_len) + PADDED_LEN(m_len) + 2 * AEAD_LEN_BYTES;
    memcpy(&len, &int_len, sizeof(size_t));
    field_elem_t len_blck; // stores the AEAD length encoding
    // auth_keys_t keys = {0};
    uint8_t len_buff[BUFFSIZE] = {0};
    memcpy(len_buff, &ad_len, AEAD_LEN_BYTES);
    memcpy(len_buff + AEAD_LEN_BYTES, &m_len, AEAD_LEN_BYTES);

    unpack_field_elem(&len_blck, len_buff);
    memset(c + m_len, 0, PAD_LEN(m_len));
    // memset(key_buff, 0, sizeof(key_buff));
    // key_buff[0].i8[0] = 0x0f;
    // *(key_buff[0].i8 + 28) = 1;
    // *(key_buff[0].i8 + 56) = 1;
    // key_buff[6].i8[0] = 1;
    unpack_and_encode_key(&hash_key[0], key_buff[0].i8 + 0);
    unpack_and_encode_key(&wc_key, key_buff[2].i8);

    for (int j = 1; j < 1; j++) {
        field_mul(&hash_key[j], &hash_key[j - 1], &hash_key[0]);
    }

    PRINT_FIELD_ELEM(hash_key[0]);
    PRINT_FIELD_ELEM(wc_key);
#ifndef __GNUC__
#pragma region ENCRYPTION
#endif
    /* PROCESS M */
    while (to_enc >= 6 * BLCKSIZE) {
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
    }
    if (to_enc >= 4 * BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        msg[6] = _mm_loadu_si128(m_ptr.v + 6);
        msg[7] = _mm_loadu_si128(m_ptr.v + 7);
        m_ptr.v += 8;
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
        ct_ptr.v += 8;
    }
    if (to_enc >= 3 * BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        m_ptr.v += 6;
        to_enc -= 3 * BLCKSIZE;
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
    if (to_enc >= 2 * BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        m_ptr.v += 4;
        to_enc -= 2 * BLCKSIZE;
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

#ifndef __GNUC__
#pragma region HASH
#endif
    while (ad_len >= HASHBLCK_SIZE) {
        unpack_and_encode_field_elem(&h_msg[0], ad_ptr.c);
        ad_ptr.c += HASHBLCK_SIZE;
        ad_len -= HASHBLCK_SIZE;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len > 0) {
        uint8_t buff[BUFFSIZE] = {0};
        memcpy(buff, ad_ptr.c, ad_len);
        unpack_and_encode_field_elem(&h_msg[0], buff);
        ad_ptr.c += ad_len;
        ad_len -= ad_len;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }

    while (ct_r_ptr.c + HASHBLCK_SIZE <= ct_ptr.c) {
        unpack_and_encode_field_elem(&h_msg[0], ct_r_ptr.c);
        ct_r_ptr.c += HASHBLCK_SIZE;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ct_r_ptr.c < ct_ptr.c) {
        uint8_t buff[BUFFSIZE] = {0};
        memcpy(buff, ct_r_ptr.c, ct_ptr.c - ct_r_ptr.c);
        unpack_and_encode_field_elem(&h_msg[0], buff);
        ct_r_ptr.c = ct_ptr.c;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
#ifndef __GNUC__
#pragma endregion HASH
#endif

    PRINT_FIELD_ELEM(acc);
    field_mul_no_carry(&dacc[0], &len_blck, &hash_key[0]);
    field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
    field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
    carry_round(&acc, &dacc[0]);
    field_add(&acc, &acc, &len);
    field_mul(&acc, &acc, &hash_key[0]);
    PRINT_FIELD_ELEM(acc);
    field_add(&acc, &acc, &wc_key);
    _carry_round(&acc, &acc);
    PRINT_FIELD_ELEM(acc);
    reduce(&acc, &acc);
    PRINT_FIELD_ELEM(acc);
    uint8_t buff[BUFFSIZE] = {0};
    pack_field_elem(buff, &acc);
    memcpy(c + PADDED_LEN(m_len), buff, TAG_SIZE);
    return;
}

void rijndael256x6x_poly256_seqx2(size_t m_len, const uint8_t m[static m_len],
                                  size_t ad_len,
                                  const uint8_t ad[static ad_len],
                                  uint8_t c[static CTXT_LEN(m_len)],
                                  const uint8_t key[static KEY_SIZE],
                                  const uint8_t nonce[NONCE_SIZE])
{
    vector key_buff[4] = {0};
    __m128i store_mask;

    vec_ptr m_ptr = {.v = (__m128i *)m};
    vec_ptr ad_ptr = {.v = (__m128i *)ad};
    vec_ptr ct_ptr = {.v = (__m128i *)c};
    vec_ptr ct_r_ptr = {.v = (__m128i *)c};
    __m128i ct[12] = {0};
    __m128i msg[12] = {0};
    __m128i iv[6][2];
    size_t to_enc = m_len;
    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    iv[0][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5],
                                 ((uint32_t *)nonce)[6], 0}})
                   .v;
    iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
    rijndael256block(&key_buff[2].v, roundkeys, iv[0]);
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
    rijndael256block(&key_buff[0].v, roundkeys, input);
    DEBUG_PRINTF("keybuff\n");
    for (size_t i = 0; i < 6 * 16; i++) {
        if (i > 0 && (i % 32 == 0))
            DEBUG_PRINTF("\n");
        DEBUG_PRINTF("%02" PRIx8 "", ((uint8_t *)key_buff[0].i8)[i]);
    }
    DEBUG_PRINTF("\n");

    // Hash Variables
    field_elem_t wc_key;
    field_elem_t hash_key[2] = {0};
    field_elem_t h_msg[2] = {0};
    dfield_elem_t dacc[2] = {0};
    field_elem_t acc = {0};
    field_elem_t len = {0}; // stores the length of the hash input (including
                            // AEAD length encoding)
    size_t int_len =
        PADDED_LEN(ad_len) + PADDED_LEN(m_len) + 2 * AEAD_LEN_BYTES;
    memcpy(&len, &int_len, sizeof(size_t));
    field_elem_t len_blck; // stores the AEAD length encoding
    // auth_keys_t keys = {0};
    uint8_t len_buff[BUFFSIZE] = {0};
    memcpy(len_buff, &ad_len, AEAD_LEN_BYTES);
    memcpy(len_buff + AEAD_LEN_BYTES, &m_len, AEAD_LEN_BYTES);

    unpack_field_elem(&len_blck, len_buff);
    memset(c + m_len, 0, PAD_LEN(m_len));
    // memset(key_buff, 0, sizeof(key_buff));
    // key_buff[0].i8[0] = 0x0f;
    // *(key_buff[0].i8 + 28) = 1;
    // *(key_buff[0].i8 + 56) = 1;
    // key_buff[6].i8[0] = 1;
    unpack_and_encode_key(&hash_key[0], key_buff[0].i8 + 0);
    unpack_and_encode_key(&wc_key, key_buff[2].i8);

    for (int j = 1; j < 2; j++) {
        field_mul(&hash_key[j], &hash_key[j - 1], &hash_key[0]);
    }

    PRINT_FIELD_ELEM(hash_key[0]);
    PRINT_FIELD_ELEM(wc_key);
#ifndef __GNUC__
#pragma region ENCRYPTION
#endif
    /* PROCESS M */
    while (to_enc >= 6 * BLCKSIZE) {
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
    }
    if (to_enc >= 4 * BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        msg[6] = _mm_loadu_si128(m_ptr.v + 6);
        msg[7] = _mm_loadu_si128(m_ptr.v + 7);
        m_ptr.v += 8;
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
        ct_ptr.v += 8;
    }
    if (to_enc >= 3 * BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        m_ptr.v += 6;
        to_enc -= 3 * BLCKSIZE;
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
    if (to_enc >= 2 * BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        m_ptr.v += 4;
        to_enc -= 2 * BLCKSIZE;
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

#ifndef __GNUC__
#pragma region HASH
#endif
    while (ad_len >= (2 * HASHBLCK_SIZE)) {
        unpack_and_encode_field_elem(&h_msg[0], ad_ptr.c + 0 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[1], ad_ptr.c + 1 * HASHBLCK_SIZE);
        ad_ptr.c += (2 * HASHBLCK_SIZE);
        ad_len -= (2 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[1]);
        field_mul_no_carry(&dacc[1], &h_msg[1], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[1]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len >= HASHBLCK_SIZE) {
        unpack_and_encode_field_elem(&h_msg[0], ad_ptr.c);
        ad_ptr.c += HASHBLCK_SIZE;
        ad_len -= HASHBLCK_SIZE;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len > 0) {
        uint8_t buff[BUFFSIZE] = {0};
        memcpy(buff, ad_ptr.c, ad_len);
        unpack_and_encode_field_elem(&h_msg[0], buff);
        ad_ptr.c += ad_len;
        ad_len -= ad_len;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }

    while (ct_r_ptr.c + (2 * HASHBLCK_SIZE) <= ct_ptr.c) {
        unpack_and_encode_field_elem(&h_msg[0], ct_r_ptr.c + 0 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[1], ct_r_ptr.c + 1 * HASHBLCK_SIZE);
        ct_r_ptr.c += (2 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[1]);
        field_mul_no_carry(&dacc[1], &h_msg[1], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[1]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ct_r_ptr.c + HASHBLCK_SIZE <= ct_ptr.c) {
        unpack_and_encode_field_elem(&h_msg[0], ct_r_ptr.c);
        ct_r_ptr.c += HASHBLCK_SIZE;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ct_r_ptr.c < ct_ptr.c) {
        uint8_t buff[BUFFSIZE] = {0};
        memcpy(buff, ct_r_ptr.c, ct_ptr.c - ct_r_ptr.c);
        unpack_and_encode_field_elem(&h_msg[0], buff);
        ct_r_ptr.c = ct_ptr.c;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
#ifndef __GNUC__
#pragma endregion HASH
#endif

    PRINT_FIELD_ELEM(acc);
    field_mul_no_carry(&dacc[0], &len_blck, &hash_key[0]);
    field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
    field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
    carry_round(&acc, &dacc[0]);
    field_add(&acc, &acc, &len);
    field_mul(&acc, &acc, &hash_key[0]);
    PRINT_FIELD_ELEM(acc);
    field_add(&acc, &acc, &wc_key);
    _carry_round(&acc, &acc);
    PRINT_FIELD_ELEM(acc);
    reduce(&acc, &acc);
    PRINT_FIELD_ELEM(acc);
    uint8_t buff[BUFFSIZE] = {0};
    pack_field_elem(buff, &acc);
    memcpy(c + PADDED_LEN(m_len), buff, TAG_SIZE);
    return;
}

void rijndael256x6x_poly256_seqx4(size_t m_len, const uint8_t m[static m_len],
                                  size_t ad_len,
                                  const uint8_t ad[static ad_len],
                                  uint8_t c[static CTXT_LEN(m_len)],
                                  const uint8_t key[static KEY_SIZE],
                                  const uint8_t nonce[NONCE_SIZE])
{
    vector key_buff[4] = {0};
    __m128i store_mask;

    vec_ptr m_ptr = {.v = (__m128i *)m};
    vec_ptr ad_ptr = {.v = (__m128i *)ad};
    vec_ptr ct_ptr = {.v = (__m128i *)c};
    vec_ptr ct_r_ptr = {.v = (__m128i *)c};
    __m128i ct[12] = {0};
    __m128i msg[12] = {0};
    __m128i iv[6][2];
    size_t to_enc = m_len;
    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    iv[0][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5],
                                 ((uint32_t *)nonce)[6], 0}})
                   .v;
    iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
    rijndael256block(&key_buff[2].v, roundkeys, iv[0]);
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
    rijndael256block(&key_buff[0].v, roundkeys, input);
    DEBUG_PRINTF("keybuff\n");
    for (size_t i = 0; i < 6 * 16; i++) {
        if (i > 0 && (i % 32 == 0))
            DEBUG_PRINTF("\n");
        DEBUG_PRINTF("%02" PRIx8 "", ((uint8_t *)key_buff[0].i8)[i]);
    }
    DEBUG_PRINTF("\n");

    // Hash Variables
    field_elem_t wc_key;
    field_elem_t hash_key[4] = {0};
    field_elem_t h_msg[4] = {0};
    dfield_elem_t dacc[4] = {0};
    field_elem_t acc = {0};
    field_elem_t len = {0}; // stores the length of the hash input (including
                            // AEAD length encoding)
    size_t int_len =
        PADDED_LEN(ad_len) + PADDED_LEN(m_len) + 2 * AEAD_LEN_BYTES;
    memcpy(&len, &int_len, sizeof(size_t));
    field_elem_t len_blck; // stores the AEAD length encoding
    // auth_keys_t keys = {0};
    uint8_t len_buff[BUFFSIZE] = {0};
    memcpy(len_buff, &ad_len, AEAD_LEN_BYTES);
    memcpy(len_buff + AEAD_LEN_BYTES, &m_len, AEAD_LEN_BYTES);

    unpack_field_elem(&len_blck, len_buff);
    memset(c + m_len, 0, PAD_LEN(m_len));
    // memset(key_buff, 0, sizeof(key_buff));
    // key_buff[0].i8[0] = 0x0f;
    // *(key_buff[0].i8 + 28) = 1;
    // *(key_buff[0].i8 + 56) = 1;
    // key_buff[6].i8[0] = 1;
    unpack_and_encode_key(&hash_key[0], key_buff[0].i8 + 0);
    unpack_and_encode_key(&wc_key, key_buff[2].i8);

    for (int j = 1; j < 4; j++) {
        field_mul(&hash_key[j], &hash_key[j - 1], &hash_key[0]);
    }

    PRINT_FIELD_ELEM(hash_key[0]);
    PRINT_FIELD_ELEM(wc_key);
#ifndef __GNUC__
#pragma region ENCRYPTION
#endif
    /* PROCESS M */
    while (to_enc >= 6 * BLCKSIZE) {
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
    }
    if (to_enc >= 4 * BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        msg[6] = _mm_loadu_si128(m_ptr.v + 6);
        msg[7] = _mm_loadu_si128(m_ptr.v + 7);
        m_ptr.v += 8;
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
        ct_ptr.v += 8;
    }
    if (to_enc >= 3 * BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        m_ptr.v += 6;
        to_enc -= 3 * BLCKSIZE;
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
    if (to_enc >= 2 * BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        m_ptr.v += 4;
        to_enc -= 2 * BLCKSIZE;
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

#ifndef __GNUC__
#pragma region HASH
#endif
    while (ad_len >= (4 * HASHBLCK_SIZE)) {
        unpack_and_encode_field_elem(&h_msg[0], ad_ptr.c + 0 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[1], ad_ptr.c + 1 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[2], ad_ptr.c + 2 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[3], ad_ptr.c + 3 * HASHBLCK_SIZE);
        ad_ptr.c += (4 * HASHBLCK_SIZE);
        ad_len -= (4 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[3]);
        field_mul_no_carry(&dacc[1], &h_msg[1], &hash_key[2]);
        field_mul_no_carry(&dacc[2], &h_msg[2], &hash_key[1]);
        field_mul_no_carry(&dacc[3], &h_msg[3], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_add_dbl(&dacc[2], &dacc[3], &dacc[2]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[2]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[3]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len >= (2 * HASHBLCK_SIZE)) {
        unpack_and_encode_field_elem(&h_msg[0], ad_ptr.c + 0 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[1], ad_ptr.c + 1 * HASHBLCK_SIZE);
        ad_ptr.c += (2 * HASHBLCK_SIZE);
        ad_len -= (2 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[1]);
        field_mul_no_carry(&dacc[1], &h_msg[1], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[1]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len >= HASHBLCK_SIZE) {
        unpack_and_encode_field_elem(&h_msg[0], ad_ptr.c);
        ad_ptr.c += HASHBLCK_SIZE;
        ad_len -= HASHBLCK_SIZE;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len > 0) {
        uint8_t buff[BUFFSIZE] = {0};
        memcpy(buff, ad_ptr.c, ad_len);
        unpack_and_encode_field_elem(&h_msg[0], buff);
        ad_ptr.c += ad_len;
        ad_len -= ad_len;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }

    while (ct_r_ptr.c + (4 * HASHBLCK_SIZE) <= ct_ptr.c) {
        unpack_and_encode_field_elem(&h_msg[0], ct_r_ptr.c + 0 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[1], ct_r_ptr.c + 1 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[2], ct_r_ptr.c + 2 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[3], ct_r_ptr.c + 3 * HASHBLCK_SIZE);
        ct_r_ptr.c += (4 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[3]);
        field_mul_no_carry(&dacc[1], &h_msg[1], &hash_key[2]);
        field_mul_no_carry(&dacc[2], &h_msg[2], &hash_key[1]);
        field_mul_no_carry(&dacc[3], &h_msg[3], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_add_dbl(&dacc[2], &dacc[3], &dacc[2]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[2]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[3]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ct_r_ptr.c + (2 * HASHBLCK_SIZE) <= ct_ptr.c) {
        unpack_and_encode_field_elem(&h_msg[0], ct_r_ptr.c + 0 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[1], ct_r_ptr.c + 1 * HASHBLCK_SIZE);
        ct_r_ptr.c += (2 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[1]);
        field_mul_no_carry(&dacc[1], &h_msg[1], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[1]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ct_r_ptr.c + HASHBLCK_SIZE <= ct_ptr.c) {
        unpack_and_encode_field_elem(&h_msg[0], ct_r_ptr.c);
        ct_r_ptr.c += HASHBLCK_SIZE;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ct_r_ptr.c < ct_ptr.c) {
        uint8_t buff[BUFFSIZE] = {0};
        memcpy(buff, ct_r_ptr.c, ct_ptr.c - ct_r_ptr.c);
        unpack_and_encode_field_elem(&h_msg[0], buff);
        ct_r_ptr.c = ct_ptr.c;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
#ifndef __GNUC__
#pragma endregion HASH
#endif

    PRINT_FIELD_ELEM(acc);
    field_mul_no_carry(&dacc[0], &len_blck, &hash_key[0]);
    field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
    field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
    carry_round(&acc, &dacc[0]);
    field_add(&acc, &acc, &len);
    field_mul(&acc, &acc, &hash_key[0]);
    PRINT_FIELD_ELEM(acc);
    field_add(&acc, &acc, &wc_key);
    _carry_round(&acc, &acc);
    PRINT_FIELD_ELEM(acc);
    reduce(&acc, &acc);
    PRINT_FIELD_ELEM(acc);
    uint8_t buff[BUFFSIZE] = {0};
    pack_field_elem(buff, &acc);
    memcpy(c + PADDED_LEN(m_len), buff, TAG_SIZE);
    return;
}

void rijndael256x6x_poly256_seqx8(size_t m_len, const uint8_t m[static m_len],
                                  size_t ad_len,
                                  const uint8_t ad[static ad_len],
                                  uint8_t c[static CTXT_LEN(m_len)],
                                  const uint8_t key[static KEY_SIZE],
                                  const uint8_t nonce[NONCE_SIZE])
{
    vector key_buff[4] = {0};
    __m128i store_mask;

    vec_ptr m_ptr = {.v = (__m128i *)m};
    vec_ptr ad_ptr = {.v = (__m128i *)ad};
    vec_ptr ct_ptr = {.v = (__m128i *)c};
    vec_ptr ct_r_ptr = {.v = (__m128i *)c};
    __m128i ct[12] = {0};
    __m128i msg[12] = {0};
    __m128i iv[6][2];
    size_t to_enc = m_len;
    __m128i roundkeys[ROUNDKEYS] = {0};
    __m128i inKeys[2];
    inKeys[0] = _mm_loadu_si128((__m128i *)key);
    inKeys[1] = _mm_loadu_si128(((__m128i *)key) + 1);
    rijndael256_key_expansion(roundkeys, inKeys);

    iv[0][0] = _mm_loadu_si128((__m128i *)nonce);
    iv[0][1] = ((vector){.i32 = {((uint32_t *)nonce)[4], ((uint32_t *)nonce)[5],
                                 ((uint32_t *)nonce)[6], 0}})
                   .v;
    iv[0][1] = _mm_add_epi32(iv[0][1], one.v);
    rijndael256block(&key_buff[2].v, roundkeys, iv[0]);
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
    rijndael256block(&key_buff[0].v, roundkeys, input);
    DEBUG_PRINTF("keybuff\n");
    for (size_t i = 0; i < 6 * 16; i++) {
        if (i > 0 && (i % 32 == 0))
            DEBUG_PRINTF("\n");
        DEBUG_PRINTF("%02" PRIx8 "", ((uint8_t *)key_buff[0].i8)[i]);
    }
    DEBUG_PRINTF("\n");

    // Hash Variables
    field_elem_t wc_key;
    field_elem_t hash_key[8] = {0};
    field_elem_t h_msg[8] = {0};
    dfield_elem_t dacc[8] = {0};
    field_elem_t acc = {0};
    field_elem_t len = {0}; // stores the length of the hash input (including
                            // AEAD length encoding)
    size_t int_len =
        PADDED_LEN(ad_len) + PADDED_LEN(m_len) + 2 * AEAD_LEN_BYTES;
    memcpy(&len, &int_len, sizeof(size_t));
    field_elem_t len_blck; // stores the AEAD length encoding
    // auth_keys_t keys = {0};
    uint8_t len_buff[BUFFSIZE] = {0};
    memcpy(len_buff, &ad_len, AEAD_LEN_BYTES);
    memcpy(len_buff + AEAD_LEN_BYTES, &m_len, AEAD_LEN_BYTES);

    unpack_field_elem(&len_blck, len_buff);
    memset(c + m_len, 0, PAD_LEN(m_len));
    // memset(key_buff, 0, sizeof(key_buff));
    // key_buff[0].i8[0] = 0x0f;
    // *(key_buff[0].i8 + 28) = 1;
    // *(key_buff[0].i8 + 56) = 1;
    // key_buff[6].i8[0] = 1;
    unpack_and_encode_key(&hash_key[0], key_buff[0].i8 + 0);
    unpack_and_encode_key(&wc_key, key_buff[2].i8);

    for (int j = 1; j < 8; j++) {
        field_mul(&hash_key[j], &hash_key[j - 1], &hash_key[0]);
    }

    PRINT_FIELD_ELEM(hash_key[0]);
    PRINT_FIELD_ELEM(wc_key);
#ifndef __GNUC__
#pragma region ENCRYPTION
#endif
    /* PROCESS M */
    while (to_enc >= 6 * BLCKSIZE) {
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
    }
    if (to_enc >= 4 * BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        msg[6] = _mm_loadu_si128(m_ptr.v + 6);
        msg[7] = _mm_loadu_si128(m_ptr.v + 7);
        m_ptr.v += 8;
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
        ct_ptr.v += 8;
    }
    if (to_enc >= 3 * BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        msg[4] = _mm_loadu_si128(m_ptr.v + 4);
        msg[5] = _mm_loadu_si128(m_ptr.v + 5);
        m_ptr.v += 6;
        to_enc -= 3 * BLCKSIZE;
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
    if (to_enc >= 2 * BLCKSIZE) {
        msg[0] = _mm_loadu_si128(m_ptr.v + 0);
        msg[1] = _mm_loadu_si128(m_ptr.v + 1);
        msg[2] = _mm_loadu_si128(m_ptr.v + 2);
        msg[3] = _mm_loadu_si128(m_ptr.v + 3);
        m_ptr.v += 4;
        to_enc -= 2 * BLCKSIZE;
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

#ifndef __GNUC__
#pragma region HASH
#endif
    while (ad_len >= (8 * HASHBLCK_SIZE)) {
        unpack_and_encode_field_elem(&h_msg[0], ad_ptr.c + 0 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[1], ad_ptr.c + 1 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[2], ad_ptr.c + 2 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[3], ad_ptr.c + 3 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[4], ad_ptr.c + 4 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[5], ad_ptr.c + 5 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[6], ad_ptr.c + 6 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[7], ad_ptr.c + 7 * HASHBLCK_SIZE);
        ad_ptr.c += (8 * HASHBLCK_SIZE);
        ad_len -= (8 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[7]);
        field_mul_no_carry(&dacc[1], &h_msg[1], &hash_key[6]);
        field_mul_no_carry(&dacc[2], &h_msg[2], &hash_key[5]);
        field_mul_no_carry(&dacc[3], &h_msg[3], &hash_key[4]);
        field_mul_no_carry(&dacc[4], &h_msg[4], &hash_key[3]);
        field_mul_no_carry(&dacc[5], &h_msg[5], &hash_key[2]);
        field_mul_no_carry(&dacc[6], &h_msg[6], &hash_key[1]);
        field_mul_no_carry(&dacc[7], &h_msg[7], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_add_dbl(&dacc[2], &dacc[3], &dacc[2]);
        field_add_dbl(&dacc[4], &dacc[5], &dacc[4]);
        field_add_dbl(&dacc[6], &dacc[7], &dacc[6]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[2]);
        field_add_dbl(&dacc[4], &dacc[4], &dacc[6]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[4]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[7]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len >= (4 * HASHBLCK_SIZE)) {
        unpack_and_encode_field_elem(&h_msg[0], ad_ptr.c + 0 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[1], ad_ptr.c + 1 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[2], ad_ptr.c + 2 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[3], ad_ptr.c + 3 * HASHBLCK_SIZE);
        ad_ptr.c += (4 * HASHBLCK_SIZE);
        ad_len -= (4 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[3]);
        field_mul_no_carry(&dacc[1], &h_msg[1], &hash_key[2]);
        field_mul_no_carry(&dacc[2], &h_msg[2], &hash_key[1]);
        field_mul_no_carry(&dacc[3], &h_msg[3], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_add_dbl(&dacc[2], &dacc[3], &dacc[2]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[2]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[3]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len >= (2 * HASHBLCK_SIZE)) {
        unpack_and_encode_field_elem(&h_msg[0], ad_ptr.c + 0 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[1], ad_ptr.c + 1 * HASHBLCK_SIZE);
        ad_ptr.c += (2 * HASHBLCK_SIZE);
        ad_len -= (2 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[1]);
        field_mul_no_carry(&dacc[1], &h_msg[1], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[1]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len >= HASHBLCK_SIZE) {
        unpack_and_encode_field_elem(&h_msg[0], ad_ptr.c);
        ad_ptr.c += HASHBLCK_SIZE;
        ad_len -= HASHBLCK_SIZE;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ad_len > 0) {
        uint8_t buff[BUFFSIZE] = {0};
        memcpy(buff, ad_ptr.c, ad_len);
        unpack_and_encode_field_elem(&h_msg[0], buff);
        ad_ptr.c += ad_len;
        ad_len -= ad_len;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }

    while (ct_r_ptr.c + (8 * HASHBLCK_SIZE) <= ct_ptr.c) {
        unpack_and_encode_field_elem(&h_msg[0], ct_r_ptr.c + 0 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[1], ct_r_ptr.c + 1 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[2], ct_r_ptr.c + 2 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[3], ct_r_ptr.c + 3 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[4], ct_r_ptr.c + 4 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[5], ct_r_ptr.c + 5 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[6], ct_r_ptr.c + 6 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[7], ct_r_ptr.c + 7 * HASHBLCK_SIZE);
        ct_r_ptr.c += (8 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[7]);
        field_mul_no_carry(&dacc[1], &h_msg[1], &hash_key[6]);
        field_mul_no_carry(&dacc[2], &h_msg[2], &hash_key[5]);
        field_mul_no_carry(&dacc[3], &h_msg[3], &hash_key[4]);
        field_mul_no_carry(&dacc[4], &h_msg[4], &hash_key[3]);
        field_mul_no_carry(&dacc[5], &h_msg[5], &hash_key[2]);
        field_mul_no_carry(&dacc[6], &h_msg[6], &hash_key[1]);
        field_mul_no_carry(&dacc[7], &h_msg[7], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_add_dbl(&dacc[2], &dacc[3], &dacc[2]);
        field_add_dbl(&dacc[4], &dacc[5], &dacc[4]);
        field_add_dbl(&dacc[6], &dacc[7], &dacc[6]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[2]);
        field_add_dbl(&dacc[4], &dacc[4], &dacc[6]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[4]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[7]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ct_r_ptr.c + (4 * HASHBLCK_SIZE) <= ct_ptr.c) {
        unpack_and_encode_field_elem(&h_msg[0], ct_r_ptr.c + 0 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[1], ct_r_ptr.c + 1 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[2], ct_r_ptr.c + 2 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[3], ct_r_ptr.c + 3 * HASHBLCK_SIZE);
        ct_r_ptr.c += (4 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[3]);
        field_mul_no_carry(&dacc[1], &h_msg[1], &hash_key[2]);
        field_mul_no_carry(&dacc[2], &h_msg[2], &hash_key[1]);
        field_mul_no_carry(&dacc[3], &h_msg[3], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        field_add_dbl(&dacc[2], &dacc[3], &dacc[2]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[2]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[3]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ct_r_ptr.c + (2 * HASHBLCK_SIZE) <= ct_ptr.c) {
        unpack_and_encode_field_elem(&h_msg[0], ct_r_ptr.c + 0 * HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg[1], ct_r_ptr.c + 1 * HASHBLCK_SIZE);
        ct_r_ptr.c += (2 * HASHBLCK_SIZE);

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[1]);
        field_mul_no_carry(&dacc[1], &h_msg[1], &hash_key[0]);

        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);

        field_mul_no_carry(&dacc[1], &acc, &hash_key[1]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ct_r_ptr.c + HASHBLCK_SIZE <= ct_ptr.c) {
        unpack_and_encode_field_elem(&h_msg[0], ct_r_ptr.c);
        ct_r_ptr.c += HASHBLCK_SIZE;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
    if (ct_r_ptr.c < ct_ptr.c) {
        uint8_t buff[BUFFSIZE] = {0};
        memcpy(buff, ct_r_ptr.c, ct_ptr.c - ct_r_ptr.c);
        unpack_and_encode_field_elem(&h_msg[0], buff);
        ct_r_ptr.c = ct_ptr.c;

        field_mul_no_carry(&dacc[0], &h_msg[0], &hash_key[0]);
        field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
        field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
        carry_round(&acc, &dacc[0]);
    }
#ifndef __GNUC__
#pragma endregion HASH
#endif

    PRINT_FIELD_ELEM(acc);
    field_mul_no_carry(&dacc[0], &len_blck, &hash_key[0]);
    field_mul_no_carry(&dacc[1], &acc, &hash_key[0]);
    field_add_dbl(&dacc[0], &dacc[0], &dacc[1]);
    carry_round(&acc, &dacc[0]);
    field_add(&acc, &acc, &len);
    field_mul(&acc, &acc, &hash_key[0]);
    PRINT_FIELD_ELEM(acc);
    field_add(&acc, &acc, &wc_key);
    _carry_round(&acc, &acc);
    PRINT_FIELD_ELEM(acc);
    reduce(&acc, &acc);
    PRINT_FIELD_ELEM(acc);
    uint8_t buff[BUFFSIZE] = {0};
    pack_field_elem(buff, &acc);
    memcpy(c + PADDED_LEN(m_len), buff, TAG_SIZE);
    return;
}
