#pragma once
#include "fieldArith/fieldArith.h"
#include "rijndael256.h"
#include <immintrin.h>
#include <stdint.h>

#define NONCE_SIZE (28)
#define IV_SIZE (BLCKSIZE)
#define KEY_SIZE (BLCKSIZE)

#define AEAD_LEN_BYTES 8
#ifndef HASHBLCK_SIZE
#define HASHBLCK_SIZE 16
#endif
#define TAG_SIZE (HASHBLCK_SIZE)
#define PAD_SIZE HASHBLCK_SIZE
#define PAD_LEN(arg1) ((PAD_SIZE - ((arg1) % PAD_SIZE)) % PAD_SIZE)
#define PADDED_LEN(arg1) (arg1 + PAD_LEN(arg1))
#define CTXT_LEN(m) (PADDED_LEN(m) + TAG_SIZE)

void rijndael256x_mhp_nmh128x4(size_t m_len, const uint8_t m[static m_len],
                               size_t ad_len, const uint8_t ad[static ad_len],
                               uint8_t c[static CTXT_LEN(m_len)],
                               const uint8_t key[static KEY_SIZE],
                               const uint8_t nonce[NONCE_SIZE]);

void rijndael256x_mhp_nmh128x6(size_t m_len, const uint8_t m[static m_len],
                               size_t ad_len, const uint8_t ad[static ad_len],
                               uint8_t c[static CTXT_LEN(m_len)],
                               const uint8_t key[static KEY_SIZE],
                               const uint8_t nonce[NONCE_SIZE]);
