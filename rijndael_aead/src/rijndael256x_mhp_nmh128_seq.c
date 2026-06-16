// #define DEBUG 1
#include <assert.h>
#include <immintrin.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "fieldArith/bf_arithmetic_128_64_64_64_schoolbook.h"
#include "fieldArith/fieldArith.h"
#include "rijndael256/rijndael256.h"
#include "rijndael256/rijndael256x_mhp_nmh128_seq.h"
#include "rijndael256_impl.h"

typedef union {
    __m256i *lv;
    __m128i *v;
    __uint8_t *c;
} vec_ptr;

const vector BYTE_IDX = {
    .i8 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}};

#define SPRBLCKSIZE 36

void rijndael256x6x_mhp_nmh128_seq(size_t m_len, const uint8_t m[static m_len],
                                size_t ad_len, const uint8_t ad[static ad_len],
                                uint8_t c[static CTXT_LEN(m_len)],
                                const uint8_t key[static KEY_SIZE],
                                const uint8_t nonce[NONCE_SIZE])
{
    vector key_buff[6] = {0};
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
    iv[0][1] = _mm_add_epi32(iv[0][1], two.v);
    rijndael256block(&key_buff[4].v, roundkeys, iv[0]);
    iv[1][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[2][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[3][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[4][0] = _mm_loadu_si128((__m128i*) nonce);
    iv[5][0] = _mm_loadu_si128((__m128i*) nonce);
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
    input[1] = one.v;
    rijndael256block(&key_buff[2].v, roundkeys, input);
    DEBUG_PRINTF("keybuff\n");
    for (size_t i = 0; i < 6 * 16; i++) {
        if (i > 0 && (i % 32 == 0))
            DEBUG_PRINTF("\n");
        DEBUG_PRINTF("%02" PRIx8 "", ((uint8_t *)key_buff[0].i8)[i]);
    }
    DEBUG_PRINTF("\n");

    // Hash Variables
    int len_added = 0;
    size_t blck_num = 0;
    field_elem_t le_key;
    field_elem_t wc_key;
    field_elem_t inner_keys[SPRBLCKSIZE] = {0};
    field_elem_t h_msg1 = {0}, h_msg2 = {0};
    dfield_elem_t dacc = {0};
    field_elem_t acc = {0};
    dfield_elem_t tmp = {0};
    field_elem_t outer_key;

    field_elem_t odd = {0};
    field_elem_t evn = {0};
    field_elem_t len = {0}; // stores the length of the hash input (including
                            // AEAD length encoding)
    field_elem_t len_blck;  // stores the AEAD length encoding
    // auth_keys_t keys = {0};
    uint8_t len_buff[BUFFSIZE] = {0};
    memcpy(len_buff, &ad_len, AEAD_LEN_BYTES);
    memcpy(len_buff + AEAD_LEN_BYTES, &m_len, AEAD_LEN_BYTES);
    size_t int_len =
        PADDED_LEN(ad_len) + PADDED_LEN(m_len) + 2 * AEAD_LEN_BYTES;
    memcpy(&len, &int_len, sizeof(size_t));

    unpack_field_elem(&len_blck, len_buff);
    memset(c + m_len, 0, PAD_LEN(m_len));

    unpack_and_encode_key(&inner_keys[0], key_buff[0].i8 + 0);
    unpack_and_encode_key(&outer_key, key_buff[0].i8 + 16);
    unpack_and_encode_key(&le_key, key_buff[0].i8 + 32);
    unpack_and_encode_key(&wc_key, key_buff[4].i8);

    for (int j = 2; j < SPRBLCKSIZE; j += 2) {
        field_mul(&inner_keys[j], &inner_keys[j - 2], &inner_keys[0]);
    }
    field_mul(&inner_keys[SPRBLCKSIZE - 1], &inner_keys[SPRBLCKSIZE - 2],
              &inner_keys[0]);
    for (int j = SPRBLCKSIZE - 3; j > 0; j -= 2) {
        field_mul(&inner_keys[j], &inner_keys[j + 2], &inner_keys[0]);
    }

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
        _mm_storeu_si128(ct_ptr.v +  0, ct[ 0]);
        _mm_storeu_si128(ct_ptr.v +  1, ct[ 1]);
        _mm_storeu_si128(ct_ptr.v +  2, ct[ 2]);
        _mm_storeu_si128(ct_ptr.v +  3, ct[ 3]);
        _mm_storeu_si128(ct_ptr.v +  4, ct[ 4]);
        _mm_storeu_si128(ct_ptr.v +  5, ct[ 5]);
        _mm_storeu_si128(ct_ptr.v +  6, ct[ 6]);
        _mm_storeu_si128(ct_ptr.v +  7, ct[ 7]);
        _mm_storeu_si128(ct_ptr.v +  8, ct[ 8]);
        _mm_storeu_si128(ct_ptr.v +  9, ct[ 9]);
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
        rijndael256blockXORx4(&ct[ 0], roundkeys, (__m128i *)iv, &msg[0]);
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
        rijndael256blockXORx3(&ct[ 0], roundkeys, (__m128i *)iv, &msg[0]);
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
        rijndael256blockXORx2(&ct[ 0], roundkeys, (__m128i *)iv, &msg[0]);
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
    while (ad_len >= 2 * HASHBLCK_SIZE) {
        HERE;
        unpack_and_encode_field_elem(&h_msg1, ad_ptr.c);
        unpack_and_encode_field_elem(&h_msg2, ad_ptr.c + HASHBLCK_SIZE);
        ad_ptr.c += 2 * HASHBLCK_SIZE;
        ad_len -= 2 * HASHBLCK_SIZE;
        field_add(&evn, &h_msg1, &(inner_keys[blck_num + 0]));
        field_add(&odd, &h_msg2, &(inner_keys[blck_num + 1]));
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
        if (blck_num == SPRBLCKSIZE) {
            HERE;
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
    }
    if (ad_len > HASHBLCK_SIZE) {
        HERE;
        unpack_and_encode_field_elem(&h_msg1, ad_ptr.c);
        uint8_t buff[BUFFSIZE] = {0};
        memcpy(buff, ad_ptr.c + HASHBLCK_SIZE, ad_len - HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg2, buff);
        ad_ptr.c += ad_len;
        ad_len -= ad_len;
        field_add(&evn, &h_msg1, &(inner_keys[blck_num + 0]));
        field_add(&odd, &h_msg2, &(inner_keys[blck_num + 1]));
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
        if (blck_num == SPRBLCKSIZE) {
            HERE;
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
    } else if (ad_len > 0) {
        HERE;
        if (ct_r_ptr.c < ct_ptr.c) {
            HERE;
            if (to_enc >= 32) {
            } else {
            }
            uint8_t buff[BUFFSIZE] = {0};
            memcpy(buff, ad_ptr.c, ad_len);
            unpack_and_encode_field_elem(&h_msg1, buff);
            field_add(&evn, &h_msg1, &(inner_keys[blck_num + 0]));
            unpack_and_encode_field_elem(&h_msg2, ct_r_ptr.c);
            field_add(&odd, &h_msg2, &(inner_keys[blck_num + 1]));
            ct_r_ptr.c += HASHBLCK_SIZE;
        } else {
            HERE;
            uint8_t buff[BUFFSIZE] = {0};
            memcpy(buff, ad_ptr.c, ad_len);
            unpack_and_encode_field_elem(&h_msg1, buff);
            field_add(&evn, &h_msg1, &(inner_keys[blck_num + 0]));
            field_add(&odd, &len_blck, &(inner_keys[blck_num + 1]));
            len_added = 1;
        }
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
    }

    while (ct_r_ptr.c + (2 * HASHBLCK_SIZE) <= ct_ptr.c) {
        HERE;
        if (blck_num == SPRBLCKSIZE) {
            HERE;
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
        unpack_and_encode_field_elem(&h_msg1, ct_r_ptr.c);
        unpack_and_encode_field_elem(&h_msg2, ct_r_ptr.c + HASHBLCK_SIZE);
        PRINT_FIELD_ELEM(h_msg1);
        PRINT_FIELD_ELEM(h_msg2);
        PRINT_FIELD_ELEM(inner_keys[blck_num + 0]);
        PRINT_FIELD_ELEM(inner_keys[blck_num + 1]);
        ct_r_ptr.c += 2 * HASHBLCK_SIZE;
        field_add(&evn, &h_msg1, &(inner_keys[blck_num + 0]));
        field_add(&odd, &h_msg2, &(inner_keys[blck_num + 1]));
        PRINT_FIELD_ELEM(evn);
        PRINT_FIELD_ELEM(odd);
        field_mul_no_carry(&tmp, &evn, &odd);
        PRINT_DFIELD_ELEM(tmp);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
    }
    if (ct_r_ptr.c + (HASHBLCK_SIZE) < ct_ptr.c) {
        HERE;
        DEBUG_PRINTF("ct_r_ptr + (HASHBLCK_SIZE) < ct_ptr\n");
        DEBUG_PRINTF("blck_num: %zu\n", blck_num);
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
        unpack_and_encode_field_elem(&h_msg1, ct_r_ptr.c);
        uint8_t buff[BUFFSIZE] = {0};
        memcpy(buff, ct_r_ptr.c + HASHBLCK_SIZE,
               ct_ptr.c - (ct_r_ptr.c + (HASHBLCK_SIZE)));
        unpack_and_encode_field_elem(&h_msg2, buff);
        PRINT_FIELD_ELEM(h_msg1);
        PRINT_FIELD_ELEM(h_msg2);
        ct_r_ptr = ct_ptr;
        field_add(&evn, &h_msg1, &(inner_keys[blck_num + 0]));
        field_add(&odd, &h_msg2, &(inner_keys[blck_num + 1]));
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
    } else if (ct_r_ptr.c < ct_ptr.c) {
        HERE;
        DEBUG_PRINTF("ct_r_ptr < ct_ptr\n");
        DEBUG_PRINTF("blck_num: %zu\n", blck_num);
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
        uint8_t buff[BUFFSIZE] = {0};
        memcpy(buff, ct_r_ptr.c, ct_ptr.c - ct_r_ptr.c);
        unpack_and_encode_field_elem(&h_msg1, buff);

        ct_r_ptr = ct_ptr;

        PRINT_FIELD_ELEM(h_msg1);
        PRINT_FIELD_ELEM(len_blck);
        field_add(&evn, &h_msg1, &(inner_keys[blck_num + 0]));
        field_add(&odd, &len_blck, &(inner_keys[blck_num + 1]));

        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
        len_added = 1;
    }
#ifndef __GNUC__
#pragma endregion HASH
#endif
    if (!len_added) {
        HERE;
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
        PRINT_DFIELD_ELEM(dacc);
        field_add_mix(&dacc, &dacc, &len_blck);
    }
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
    _carry_round(&acc, &acc);
    PRINT_FIELD_ELEM(acc);
    reduce(&acc, &acc);
    PRINT_FIELD_ELEM(acc);
    uint8_t buff[BUFFSIZE] = {0};
    pack_field_elem(buff, &acc);
    memcpy(c + PADDED_LEN(m_len), buff, TAG_SIZE);
    return;
}

void rijndael256x4x_mhp_nmh128_seq(size_t m_len, const uint8_t m[static m_len],
                                   size_t ad_len,
                                   const uint8_t ad[static ad_len],
                                   uint8_t c[static CTXT_LEN(m_len)],
                                   const uint8_t key[static KEY_SIZE],
                                   const uint8_t nonce[NONCE_SIZE])
{
    vector key_buff[6] = {0};
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
    iv[0][1] = _mm_add_epi32(iv[0][1], two.v);
    rijndael256block(&key_buff[4].v, roundkeys, iv[0]);
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
    input[1] = one.v;
    rijndael256block(&key_buff[2].v, roundkeys, input);
    DEBUG_PRINTF("keybuff\n");
    for (size_t i = 0; i < 6 * 16; i++) {
        if (i > 0 && (i % 32 == 0))
            DEBUG_PRINTF("\n");
        DEBUG_PRINTF("%02" PRIx8 "", ((uint8_t *)key_buff[0].i8)[i]);
    }
    DEBUG_PRINTF("\n");

    // Hash Variables
    int len_added = 0;
    size_t blck_num = 0;
    field_elem_t le_key;
    field_elem_t wc_key;
    field_elem_t inner_keys[SPRBLCKSIZE] = {0};
    field_elem_t h_msg1 = {0}, h_msg2 = {0};
    dfield_elem_t dacc = {0};
    field_elem_t acc = {0};
    dfield_elem_t tmp = {0};
    field_elem_t outer_key;

    field_elem_t odd = {0};
    field_elem_t evn = {0};
    field_elem_t len = {0}; // stores the length of the hash input (including
                            // AEAD length encoding)
    field_elem_t len_blck;  // stores the AEAD length encoding
    // auth_keys_t keys = {0};
    uint8_t len_buff[BUFFSIZE] = {0};
    memcpy(len_buff, &ad_len, AEAD_LEN_BYTES);
    memcpy(len_buff + AEAD_LEN_BYTES, &m_len, AEAD_LEN_BYTES);
    size_t int_len =
        PADDED_LEN(ad_len) + PADDED_LEN(m_len) + 2 * AEAD_LEN_BYTES;
    memcpy(&len, &int_len, sizeof(size_t));

    unpack_field_elem(&len_blck, len_buff);
    memset(c + m_len, 0, PAD_LEN(m_len));

    unpack_and_encode_key(&inner_keys[0], key_buff[0].i8 + 0);
    unpack_and_encode_key(&outer_key, key_buff[0].i8 + 16);
    unpack_and_encode_key(&le_key, key_buff[0].i8 + 32);
    unpack_and_encode_key(&wc_key, key_buff[4].i8);

    for (int j = 2; j < SPRBLCKSIZE; j += 2) {
        field_mul(&inner_keys[j], &inner_keys[j - 2], &inner_keys[0]);
    }
    field_mul(&inner_keys[SPRBLCKSIZE - 1], &inner_keys[SPRBLCKSIZE - 2],
              &inner_keys[0]);
    for (int j = SPRBLCKSIZE - 3; j > 0; j -= 2) {
        field_mul(&inner_keys[j], &inner_keys[j + 2], &inner_keys[0]);
    }

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
        rijndael256blockXORx4(&ct[ 0], roundkeys, (__m128i *)iv, &msg[0]);
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

        rijndael256blockXORx3(&ct[ 0], roundkeys, (__m128i *)iv, &msg[ 0]);
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

        rijndael256blockXORx2(&ct[ 0], roundkeys, (__m128i *)iv, &msg[ 0]);
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
    while (ad_len >= 2 * HASHBLCK_SIZE) {
        HERE;
        unpack_and_encode_field_elem(&h_msg1, ad_ptr.c);
        unpack_and_encode_field_elem(&h_msg2, ad_ptr.c + HASHBLCK_SIZE);
        ad_ptr.c += 2 * HASHBLCK_SIZE;
        ad_len -= 2 * HASHBLCK_SIZE;
        field_add(&evn, &h_msg1, &(inner_keys[blck_num + 0]));
        field_add(&odd, &h_msg2, &(inner_keys[blck_num + 1]));
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
        if (blck_num == SPRBLCKSIZE) {
            HERE;
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
    }
    if (ad_len > HASHBLCK_SIZE) {
        HERE;
        unpack_and_encode_field_elem(&h_msg1, ad_ptr.c);
        uint8_t buff[BUFFSIZE] = {0};
        memcpy(buff, ad_ptr.c + HASHBLCK_SIZE, ad_len - HASHBLCK_SIZE);
        unpack_and_encode_field_elem(&h_msg2, buff);
        ad_ptr.c += ad_len;
        ad_len -= ad_len;
        field_add(&evn, &h_msg1, &(inner_keys[blck_num + 0]));
        field_add(&odd, &h_msg2, &(inner_keys[blck_num + 1]));
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
        if (blck_num == SPRBLCKSIZE) {
            HERE;
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
    } else if (ad_len > 0) {
        HERE;
        if (ct_r_ptr.c < ct_ptr.c) {
            HERE;
            if (to_enc >= 32) {
            } else {
            }
            uint8_t buff[BUFFSIZE] = {0};
            memcpy(buff, ad_ptr.c, ad_len);
            unpack_and_encode_field_elem(&h_msg1, buff);
            field_add(&evn, &h_msg1, &(inner_keys[blck_num + 0]));
            unpack_and_encode_field_elem(&h_msg2, ct_r_ptr.c);
            field_add(&odd, &h_msg2, &(inner_keys[blck_num + 1]));
            ct_r_ptr.c += HASHBLCK_SIZE;
        } else {
            HERE;
            uint8_t buff[BUFFSIZE] = {0};
            memcpy(buff, ad_ptr.c, ad_len);
            unpack_and_encode_field_elem(&h_msg1, buff);
            field_add(&evn, &h_msg1, &(inner_keys[blck_num + 0]));
            field_add(&odd, &len_blck, &(inner_keys[blck_num + 1]));
            len_added = 1;
        }
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
    }

    while (ct_r_ptr.c + (2 * HASHBLCK_SIZE) <= ct_ptr.c) {
        HERE;
        if (blck_num == SPRBLCKSIZE) {
            HERE;
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
        unpack_and_encode_field_elem(&h_msg1, ct_r_ptr.c);
        unpack_and_encode_field_elem(&h_msg2, ct_r_ptr.c + HASHBLCK_SIZE);
        PRINT_FIELD_ELEM(h_msg1);
        PRINT_FIELD_ELEM(h_msg2);
        PRINT_FIELD_ELEM(inner_keys[blck_num + 0]);
        PRINT_FIELD_ELEM(inner_keys[blck_num + 1]);
        ct_r_ptr.c += 2 * HASHBLCK_SIZE;
        field_add(&evn, &h_msg1, &(inner_keys[blck_num + 0]));
        field_add(&odd, &h_msg2, &(inner_keys[blck_num + 1]));
        PRINT_FIELD_ELEM(evn);
        PRINT_FIELD_ELEM(odd);
        field_mul_no_carry(&tmp, &evn, &odd);
        PRINT_DFIELD_ELEM(tmp);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
    }
    if (ct_r_ptr.c + (HASHBLCK_SIZE) < ct_ptr.c) {
        HERE;
        DEBUG_PRINTF("ct_r_ptr + (HASHBLCK_SIZE) < ct_ptr\n");
        DEBUG_PRINTF("blck_num: %zu\n", blck_num);
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
        unpack_and_encode_field_elem(&h_msg1, ct_r_ptr.c);
        uint8_t buff[BUFFSIZE] = {0};
        memcpy(buff, ct_r_ptr.c + HASHBLCK_SIZE,
               ct_ptr.c - (ct_r_ptr.c + (HASHBLCK_SIZE)));
        unpack_and_encode_field_elem(&h_msg2, buff);
        PRINT_FIELD_ELEM(h_msg1);
        PRINT_FIELD_ELEM(h_msg2);
        ct_r_ptr = ct_ptr;
        field_add(&evn, &h_msg1, &(inner_keys[blck_num + 0]));
        field_add(&odd, &h_msg2, &(inner_keys[blck_num + 1]));
        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
    } else if (ct_r_ptr.c < ct_ptr.c) {
        HERE;
        DEBUG_PRINTF("ct_r_ptr < ct_ptr\n");
        DEBUG_PRINTF("blck_num: %zu\n", blck_num);
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
        uint8_t buff[BUFFSIZE] = {0};
        memcpy(buff, ct_r_ptr.c, ct_ptr.c - ct_r_ptr.c);
        unpack_and_encode_field_elem(&h_msg1, buff);

        ct_r_ptr = ct_ptr;

        PRINT_FIELD_ELEM(h_msg1);
        PRINT_FIELD_ELEM(len_blck);
        field_add(&evn, &h_msg1, &(inner_keys[blck_num + 0]));
        field_add(&odd, &len_blck, &(inner_keys[blck_num + 1]));

        field_mul_no_carry(&tmp, &evn, &odd);
        field_add_dbl(&dacc, &dacc, &tmp);
        blck_num += 2;
        len_added = 1;
    }
#ifndef __GNUC__
#pragma endregion HASH
#endif
    if (!len_added) {
        HERE;
        if (blck_num == SPRBLCKSIZE) {
            carry_round(&acc, &dacc);
            field_mul_no_carry(&dacc, &acc, &outer_key);
            blck_num = 0;
        }
        PRINT_DFIELD_ELEM(dacc);
        field_add_mix(&dacc, &dacc, &len_blck);
    }
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
    _carry_round(&acc, &acc);
    PRINT_FIELD_ELEM(acc);
    reduce(&acc, &acc);
    PRINT_FIELD_ELEM(acc);
    uint8_t buff[BUFFSIZE] = {0};
    pack_field_elem(buff, &acc);
    memcpy(c + PADDED_LEN(m_len), buff, TAG_SIZE);
    return;
}
