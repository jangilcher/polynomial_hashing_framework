#include "rijndael256/rijndael256.h"
#include "rijndael256_impl.h"
#include <immintrin.h>
#include <inttypes.h>
#include <string.h>

const vector BYTE_IDX = {.i8 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}};
void rijndael256_ctr(const uint8_t *m, size_t m_len, uint8_t *c, const __m128i roundkeys[30],
                     const __m128i iv_in[2])
{
    __m128i store_mask;
    __m128i *m_ptr = (__m128i *)m;
    __m128i *ct_ptr = (__m128i *)c;
    vector ct[2] = {0};
    __m128i msg[2] = {0};
    __m128i iv[2];
    iv[0] = iv_in[0];
    iv[1] = iv_in[1];

    while (m_len >= 32) {
        msg[0] = _mm_loadu_si128(m_ptr);
        msg[1] = _mm_loadu_si128(m_ptr + 1);
        m_ptr += 2;
        m_len -= 32;
        rijndael256blockXOR((__m128i *)ct, roundkeys, iv, msg);
        _mm_storeu_si128(ct_ptr, ct[0].v);
        _mm_storeu_si128(ct_ptr + 1, ct[1].v);
        iv[1] = _mm_add_epi32(iv[1], one.v);
        ct_ptr += 2;
    }
    if (m_len >= 16) {
        msg[0] = _mm_loadu_si128(m_ptr);
        msg[1] = _mm_setzero_si128();
        memcpy(&msg[1], m_ptr + 1, m_len - 16);
        rijndael256blockXOR((__m128i *)ct, roundkeys, iv, msg);
        store_mask = _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)m_len - 16), BYTE_IDX.v);
        ct[1].v = _mm_and_si128(ct[1].v, store_mask);
        _mm_storeu_si128(ct_ptr, ct[0].v);
        memcpy(ct_ptr + 1, &ct[1].v, m_len - 16);
    } else if (m_len > 0) {
        msg[0] = _mm_setzero_si128();
        memcpy(&msg[0], m_ptr, m_len);
        msg[1] = _mm_setzero_si128();
        rijndael256blockXOR((__m128i *)ct, roundkeys, iv, msg);
        store_mask = _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)m_len), BYTE_IDX.v);
        ct[0].v = _mm_and_si128(ct[0].v, store_mask);
        ct[1].v = _mm_setzero_si128();
        memcpy(ct_ptr, &ct[0].v, m_len);
    }
    return;
}

void rijndael256_ctrx2(const uint8_t *m, size_t m_len, uint8_t *c, const __m128i roundkeys[30],
                       const __m128i iv_in[2])
{
    __m128i store_mask;

    __m128i *m_ptr = (__m128i *)m;
    __m128i *ct_ptr = (__m128i *)c;
    vector ct1[2] = {0};
    __m128i msg1[2] = {0};
    __m128i iv1[2];

    vector ct2[2] = {0};
    __m128i msg2[2] = {0};
    __m128i iv2[2];

    iv1[0] = iv_in[0];
    iv1[1] = iv_in[1];
    iv2[0] = iv_in[0];
    iv2[1] = _mm_add_epi32(iv_in[1], one.v);

    while (m_len >= 64) {
        msg1[0] = _mm_loadu_si128(m_ptr + 0 + 0);
        msg1[1] = _mm_loadu_si128(m_ptr + 0 + 1);
        msg2[0] = _mm_loadu_si128(m_ptr + 2 + 0);
        msg2[1] = _mm_loadu_si128(m_ptr + 2 + 1);
        m_ptr += 4;
        m_len -= 64;
        rijndael256blockXOR((__m128i *)ct1, roundkeys, iv1, msg1);
        rijndael256blockXOR((__m128i *)ct2, roundkeys, iv2, msg2);
        _mm_storeu_si128(ct_ptr + 0 + 0, ct1[0].v);
        _mm_storeu_si128(ct_ptr + 0 + 1, ct1[1].v);
        _mm_storeu_si128(ct_ptr + 2 + 0, ct2[0].v);
        _mm_storeu_si128(ct_ptr + 2 + 1, ct2[1].v);

        iv1[1] = _mm_add_epi32(iv1[1], two.v);
        iv2[1] = _mm_add_epi32(iv2[1], two.v);
        ct_ptr += 4;
    }
    if (m_len >= 32) {
        msg1[0] = _mm_loadu_si128(m_ptr);
        msg1[1] = _mm_loadu_si128(m_ptr + 1);
        m_ptr += 2;
        m_len -= 32;
        rijndael256blockXOR((__m128i *)ct1, roundkeys, iv1, msg1);
        _mm_storeu_si128(ct_ptr, ct1[0].v);
        _mm_storeu_si128(ct_ptr + 1, ct1[1].v);
        iv1[1] = _mm_add_epi32(iv1[1], one.v);
        ct_ptr += 2;
    }
    if (m_len >= 16) {
        msg1[0] = _mm_loadu_si128(m_ptr);
        msg1[1] = _mm_setzero_si128();
        memcpy(&msg1[1], m_ptr + 1, m_len - 16);
        rijndael256blockXOR((__m128i *)ct1, roundkeys, iv1, msg1);
        store_mask = _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)m_len - 16), BYTE_IDX.v);
        ct1[1].v = _mm_and_si128(ct1[1].v, store_mask);
        _mm_storeu_si128(ct_ptr, ct1[0].v);
        memcpy(ct_ptr + 1, &ct1[1].v, m_len - 16);
    } else if (m_len > 0) {
        msg1[0] = _mm_setzero_si128();
        memcpy(&msg1[0], m_ptr, m_len);
        msg1[1] = _mm_setzero_si128();
        rijndael256blockXOR((__m128i *)ct1, roundkeys, iv1, msg1);
        store_mask = _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)m_len), BYTE_IDX.v);
        ct1[0].v = _mm_and_si128(ct1[0].v, store_mask);
        ct1[1].v = _mm_setzero_si128();
        memcpy(ct_ptr, &ct1[0].v, m_len);
    }
    return;
}

void rijndael256_ctrx3(const uint8_t *m, size_t m_len, uint8_t *c, const __m128i roundkeys[30],
                       const __m128i iv_in[2])
{
    __m128i store_mask;

    __m128i *m_ptr = (__m128i *)m;
    __m128i *ct_ptr = (__m128i *)c;
    vector ct1[2] = {0};
    __m128i msg1[2] = {0};
    __m128i iv1[2];

    vector ct2[2] = {0};
    __m128i msg2[2] = {0};
    __m128i iv2[2];

    vector ct3[2] = {0};
    __m128i msg3[2] = {0};
    __m128i iv3[2];

    iv1[0] = iv_in[0];
    iv1[1] = iv_in[1];
    iv2[0] = iv_in[0];
    iv2[1] = _mm_add_epi32(iv_in[1], one.v);
    iv3[0] = iv_in[0];
    iv3[1] = _mm_add_epi32(iv_in[1], two.v);

    while (m_len >= 96) {
        msg1[0] = _mm_loadu_si128(m_ptr + 0 + 0);
        msg1[1] = _mm_loadu_si128(m_ptr + 0 + 1);
        msg2[0] = _mm_loadu_si128(m_ptr + 2 + 0);
        msg2[1] = _mm_loadu_si128(m_ptr + 2 + 1);
        msg3[0] = _mm_loadu_si128(m_ptr + 4 + 0);
        msg3[1] = _mm_loadu_si128(m_ptr + 4 + 1);
        m_ptr += 6;
        m_len -= 96;
        rijndael256blockXOR((__m128i *)ct1, roundkeys, iv1, msg1);
        rijndael256blockXOR((__m128i *)ct2, roundkeys, iv2, msg2);
        rijndael256blockXOR((__m128i *)ct3, roundkeys, iv3, msg3);
        _mm_storeu_si128(ct_ptr + 0 + 0, ct1[0].v);
        _mm_storeu_si128(ct_ptr + 0 + 1, ct1[1].v);
        _mm_storeu_si128(ct_ptr + 2 + 0, ct2[0].v);
        _mm_storeu_si128(ct_ptr + 2 + 1, ct2[1].v);
        _mm_storeu_si128(ct_ptr + 4 + 0, ct3[0].v);
        _mm_storeu_si128(ct_ptr + 4 + 1, ct3[1].v);

        iv1[1] = _mm_add_epi32(iv1[1], three.v);
        iv2[1] = _mm_add_epi32(iv2[1], three.v);
        iv3[1] = _mm_add_epi32(iv3[1], three.v);
        ct_ptr += 6;
    }
    if (m_len >= 64) {
        msg1[0] = _mm_loadu_si128(m_ptr + 0 + 0);
        msg1[1] = _mm_loadu_si128(m_ptr + 0 + 1);
        msg2[0] = _mm_loadu_si128(m_ptr + 2 + 0);
        msg2[1] = _mm_loadu_si128(m_ptr + 2 + 1);
        m_ptr += 4;
        m_len -= 64;
        rijndael256blockXOR((__m128i *)ct1, roundkeys, iv1, msg1);
        rijndael256blockXOR((__m128i *)ct2, roundkeys, iv2, msg2);
        _mm_storeu_si128(ct_ptr + 0 + 0, ct1[0].v);
        _mm_storeu_si128(ct_ptr + 0 + 1, ct1[1].v);
        _mm_storeu_si128(ct_ptr + 2 + 0, ct2[0].v);
        _mm_storeu_si128(ct_ptr + 2 + 1, ct2[1].v);

        iv1[1] = _mm_add_epi32(iv1[1], two.v);
        iv2[1] = _mm_add_epi32(iv2[1], two.v);
        ct_ptr += 4;
    }
    if (m_len >= 32) {
        msg1[0] = _mm_loadu_si128(m_ptr);
        msg1[1] = _mm_loadu_si128(m_ptr + 1);
        m_ptr += 2;
        m_len -= 32;
        rijndael256blockXOR((__m128i *)ct1, roundkeys, iv1, msg1);
        _mm_storeu_si128(ct_ptr, ct1[0].v);
        _mm_storeu_si128(ct_ptr + 1, ct1[1].v);
        iv1[1] = _mm_add_epi32(iv1[1], one.v);
        ct_ptr += 2;
    }
    if (m_len >= 16) {
        msg1[0] = _mm_loadu_si128(m_ptr);
        msg1[1] = _mm_setzero_si128();
        memcpy(&msg1[1], m_ptr + 1, m_len - 16);
        rijndael256blockXOR((__m128i *)ct1, roundkeys, iv1, msg1);
        store_mask = _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)m_len - 16), BYTE_IDX.v);
        ct1[1].v = _mm_and_si128(ct1[1].v, store_mask);
        _mm_storeu_si128(ct_ptr, ct1[0].v);
        memcpy(ct_ptr + 1, &ct1[1].v, m_len - 16);
    } else if (m_len > 0) {
        msg1[0] = _mm_setzero_si128();
        memcpy(&msg1[0], m_ptr, m_len);
        msg1[1] = _mm_setzero_si128();
        rijndael256blockXOR((__m128i *)ct1, roundkeys, iv1, msg1);
        store_mask = _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)m_len), BYTE_IDX.v);
        ct1[0].v = _mm_and_si128(ct1[0].v, store_mask);
        ct1[1].v = _mm_setzero_si128();
        memcpy(ct_ptr, &ct1[0].v, m_len);
    }
    return;
}

void rijndael256_ctrx4(const uint8_t *m, size_t m_len, uint8_t *c, const __m128i roundkeys[30],
                       const __m128i iv_in[2])
{
    __m128i store_mask;

    __m128i *m_ptr = (__m128i *)m;
    __m128i *ct_ptr = (__m128i *)c;
    vector ct1[2] = {0};
    __m128i msg1[2] = {0};
    __m128i iv1[2];

    vector ct2[2] = {0};
    __m128i msg2[2] = {0};
    __m128i iv2[2];

    vector ct3[2] = {0};
    __m128i msg3[2] = {0};
    __m128i iv3[2];

    vector ct4[2] = {0};
    __m128i msg4[2] = {0};
    __m128i iv4[2];

    iv1[0] = iv_in[0];
    iv1[1] = iv_in[1];
    iv2[0] = iv_in[0];
    iv2[1] = _mm_add_epi32(iv_in[1], one.v);
    iv3[0] = iv_in[0];
    iv3[1] = _mm_add_epi32(iv_in[1], two.v);
    iv4[0] = iv_in[0];
    iv4[1] = _mm_add_epi32(iv_in[1], three.v);

    while (m_len >= 128) {
        msg1[0] = _mm_loadu_si128(m_ptr + 0 + 0);
        msg1[1] = _mm_loadu_si128(m_ptr + 0 + 1);
        msg2[0] = _mm_loadu_si128(m_ptr + 2 + 0);
        msg2[1] = _mm_loadu_si128(m_ptr + 2 + 1);
        msg3[0] = _mm_loadu_si128(m_ptr + 4 + 0);
        msg3[1] = _mm_loadu_si128(m_ptr + 4 + 1);
        msg4[0] = _mm_loadu_si128(m_ptr + 6 + 0);
        msg4[1] = _mm_loadu_si128(m_ptr + 6 + 1);
        m_ptr += 8;
        m_len -= 128;
        rijndael256blockXOR((__m128i *)ct1, roundkeys, iv1, msg1);
        rijndael256blockXOR((__m128i *)ct2, roundkeys, iv2, msg2);
        rijndael256blockXOR((__m128i *)ct3, roundkeys, iv3, msg3);
        rijndael256blockXOR((__m128i *)ct4, roundkeys, iv4, msg4);
        _mm_storeu_si128(ct_ptr + 0 + 0, ct1[0].v);
        _mm_storeu_si128(ct_ptr + 0 + 1, ct1[1].v);
        _mm_storeu_si128(ct_ptr + 2 + 0, ct2[0].v);
        _mm_storeu_si128(ct_ptr + 2 + 1, ct2[1].v);
        _mm_storeu_si128(ct_ptr + 4 + 0, ct3[0].v);
        _mm_storeu_si128(ct_ptr + 4 + 1, ct3[1].v);
        _mm_storeu_si128(ct_ptr + 6 + 0, ct4[0].v);
        _mm_storeu_si128(ct_ptr + 6 + 1, ct4[1].v);

        iv1[1] = _mm_add_epi32(iv1[1], four.v);
        iv2[1] = _mm_add_epi32(iv2[1], four.v);
        iv3[1] = _mm_add_epi32(iv3[1], four.v);
        iv4[1] = _mm_add_epi32(iv4[1], four.v);
        ct_ptr += 8;
    }
    if (m_len >= 96) {
        msg1[0] = _mm_loadu_si128(m_ptr + 0 + 0);
        msg1[1] = _mm_loadu_si128(m_ptr + 0 + 1);
        msg2[0] = _mm_loadu_si128(m_ptr + 2 + 0);
        msg2[1] = _mm_loadu_si128(m_ptr + 2 + 1);
        msg3[0] = _mm_loadu_si128(m_ptr + 4 + 0);
        msg3[1] = _mm_loadu_si128(m_ptr + 4 + 1);
        m_ptr += 6;
        m_len -= 96;
        rijndael256blockXOR((__m128i *)ct1, roundkeys, iv1, msg1);
        rijndael256blockXOR((__m128i *)ct2, roundkeys, iv2, msg2);
        rijndael256blockXOR((__m128i *)ct3, roundkeys, iv3, msg3);
        _mm_storeu_si128(ct_ptr + 0 + 0, ct1[0].v);
        _mm_storeu_si128(ct_ptr + 0 + 1, ct1[1].v);
        _mm_storeu_si128(ct_ptr + 2 + 0, ct2[0].v);
        _mm_storeu_si128(ct_ptr + 2 + 1, ct2[1].v);
        _mm_storeu_si128(ct_ptr + 4 + 0, ct3[0].v);
        _mm_storeu_si128(ct_ptr + 4 + 1, ct3[1].v);

        iv1[1] = _mm_add_epi32(iv1[1], three.v);
        iv2[1] = _mm_add_epi32(iv2[1], three.v);
        iv3[1] = _mm_add_epi32(iv3[1], three.v);
        ct_ptr += 6;
    }
    if (m_len >= 64) {
        msg1[0] = _mm_loadu_si128(m_ptr + 0 + 0);
        msg1[1] = _mm_loadu_si128(m_ptr + 0 + 1);
        msg2[0] = _mm_loadu_si128(m_ptr + 2 + 0);
        msg2[1] = _mm_loadu_si128(m_ptr + 2 + 1);
        m_ptr += 4;
        m_len -= 64;
        rijndael256blockXOR((__m128i *)ct1, roundkeys, iv1, msg1);
        rijndael256blockXOR((__m128i *)ct2, roundkeys, iv2, msg2);
        _mm_storeu_si128(ct_ptr + 0 + 0, ct1[0].v);
        _mm_storeu_si128(ct_ptr + 0 + 1, ct1[1].v);
        _mm_storeu_si128(ct_ptr + 2 + 0, ct2[0].v);
        _mm_storeu_si128(ct_ptr + 2 + 1, ct2[1].v);

        iv1[1] = _mm_add_epi32(iv1[1], two.v);
        iv2[1] = _mm_add_epi32(iv2[1], two.v);
        ct_ptr += 4;
    }
    if (m_len >= 32) {
        msg1[0] = _mm_loadu_si128(m_ptr);
        msg1[1] = _mm_loadu_si128(m_ptr + 1);
        m_ptr += 2;
        m_len -= 32;
        rijndael256blockXOR((__m128i *)ct1, roundkeys, iv1, msg1);
        _mm_storeu_si128(ct_ptr, ct1[0].v);
        _mm_storeu_si128(ct_ptr + 1, ct1[1].v);
        iv1[1] = _mm_add_epi32(iv1[1], one.v);
        ct_ptr += 2;
    }
    if (m_len >= 16) {
        msg1[0] = _mm_loadu_si128(m_ptr);
        msg1[1] = _mm_setzero_si128();
        memcpy(&msg1[1], m_ptr + 1, m_len - 16);
        rijndael256blockXOR((__m128i *)ct1, roundkeys, iv1, msg1);
        store_mask = _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)m_len - 16), BYTE_IDX.v);
        ct1[1].v = _mm_and_si128(ct1[1].v, store_mask);
        _mm_storeu_si128(ct_ptr, ct1[0].v);
        memcpy(ct_ptr + 1, &ct1[1].v, m_len - 16);
    } else if (m_len > 0) {
        msg1[0] = _mm_setzero_si128();
        memcpy(&msg1[0], m_ptr, m_len);
        msg1[1] = _mm_setzero_si128();
        rijndael256blockXOR((__m128i *)ct1, roundkeys, iv1, msg1);
        store_mask = _mm_cmpgt_epi8(_mm_set1_epi8((unsigned char)m_len), BYTE_IDX.v);
        ct1[0].v = _mm_and_si128(ct1[0].v, store_mask);
        ct1[1].v = _mm_setzero_si128();
        memcpy(ct_ptr, &ct1[0].v, m_len);
    }
    return;
}