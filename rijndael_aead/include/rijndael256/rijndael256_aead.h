#pragma once
#include <stdint.h>
#include <immintrin.h>
#include "rijndael256.h"

//TODO: FIX
#define NONCE_SIZE (28)
#define IV_SIZE (BLCKSIZE)
#define KEY_SIZE (BLCKSIZE)

#define TAG_SIZE (HASHKEYSIZE)
// #ifndef HASHBLCK_SIZE
#define HASHBLCK_SIZE HASHKEYSIZE
// #endif
#define PAD_SIZE HASHBLCK_SIZE
#define PAD_LEN(arg1) ((PAD_SIZE - ((arg1) % PAD_SIZE)) % PAD_SIZE)
#define PADDED_LEN(arg1) (arg1 + PAD_LEN(arg1))

#define CTXT_LEN(m) (PADDED_LEN(m) + TAG_SIZE)

void rijndael_aead(size_t m_len, const uint8_t inp[static m_len], size_t ad_len,
              const uint8_t ad[static ad_len], uint8_t out[static CTXT_LEN(m_len)],
              const uint8_t k[static KEY_SIZE], const uint8_t iv_in[static IV_SIZE]);